// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>
#include <iostream>

#include <boost/assign/list_of.hpp>

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroupActions.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CTime.hpp"
#include "Solver/FlowSolver.hpp"
#include "Solver/Tags.hpp"

namespace CF {
namespace Solver {

using namespace boost::assign;
using namespace Common;
using namespace Mesh;
using namespace Physics;
using namespace Solver;

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < FlowSolver, CSolver, LibSolver > FlowSolver_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

FlowSolver::FlowSolver ( const std::string& name  ) : CSolver ( name )
{
  // properties

  m_properties["brief"] = std::string("Basic Flow Solver component");
  m_properties["description"] = std::string("");

  // options

  m_options.add_option(OptionComponent<CMesh>::create(Tags::mesh(), &m_mesh))
      ->description("Mesh")
      ->pretty_name("Mesh")
      ->attach_trigger( boost::bind ( &FlowSolver::setup, this ) )
      ->mark_basic();

  m_options.add_option(OptionComponent<PhysModel>::create(Tags::physical_model(), &m_physical_model))
      ->description("Mesh")
      ->pretty_name("Mesh")
      ->attach_trigger( boost::bind ( &FlowSolver::setup, this ) )
      ->mark_basic();

  m_options.add_option(OptionComponent<CTime>::create(Tags::time(), &m_time))
      ->description("Time tracking component")
      ->pretty_name("Time")
      ->attach_trigger( boost::bind ( &FlowSolver::setup, this ) )
      ->mark_basic();

  m_options.add_option(OptionComponent<CAction>::create(Tags::setup(), &m_setup))
      ->description("Setup action for the solver")
      ->pretty_name("Setup")
      ->attach_trigger( boost::bind ( &FlowSolver::setup, this ) )
      ->mark_basic();

  m_options.add_option(OptionComponent<CAction>::create(Tags::solve(), &m_solve))
      ->description("Action that executes the \"solve\"")
      ->pretty_name("Solve")
      ->attach_trigger( boost::bind ( &FlowSolver::setup, this) )
      ->mark_basic();

  m_options.add_option(OptionComponent<CAction>::create(Tags::inner(), &m_inner))
      ->description("Action to execute inner domain computations. Tag component with \"inner\" for automatic detection")
      ->pretty_name("Inner Domain");

  m_options.add_option(OptionComponent<CAction>::create(Tags::bc(), &m_bc))
      ->description("Action to execute boundary conditions. Tag component with \"bc\" for automatic detection")
      ->pretty_name("Boundary Conditions");

  // signals

  regist_signal( "create_bc_action" )
      ->connect( boost::bind( &FlowSolver::signal_create_bc_action, this, _1 ) )
      ->signature( boost::bind( &FlowSolver::signature_create_bc_action, this, _1 ) )
      ->description("Create Boundary Condition")
      ->pretty_name("Create Boundary Condition");

  regist_signal( "create_inner_action" )
      ->connect( boost::bind( &FlowSolver::signal_create_inner_action, this, _1 ) )
      ->signature( boost::bind( &FlowSolver::signature_create_inner_action, this, _1 ) )
      ->description("Create Inner Domain action")
      ->pretty_name("Create Inner Domain action");
}


FlowSolver::~FlowSolver()
{
}


void FlowSolver::setup()
{
  if ( m_solve.expired() == false )
  {
    // Attempt to automatically find m_bc component
    if (m_bc.expired() == true)
    {
      Component::Ptr bc_action = find_component_ptr_recursively_with_tag(*m_solve.lock(),Tags::bc());
      if (is_not_null(bc_action)) configure_option(Tags::bc(),bc_action->uri());
    }

    // Attempt to automatically find m_inner component
    if (m_inner.expired() == true)
    {
      Component::Ptr inner_action = find_component_ptr_recursively_with_tag(*m_solve.lock(),Tags::inner());
      if (is_not_null(inner_action)) configure_option(Tags::inner(),inner_action->uri());
    }

    //m_inner = find_component_ptr_recursively_with_tag<CAction>(*m_solve.lock(),Tags::inner());
    if ( m_setup.expired() == false )
    {
      if ( m_physical_model.expired() == false )
      {
        m_solve.lock()->configure_option_recursively(Tags::physical_model(),m_physical_model.lock()->uri());
        m_setup.lock()->configure_option_recursively(Tags::physical_model(),m_physical_model.lock()->uri());

        if ( m_mesh.expired() == false )
        {
          m_solve.lock()->configure_option_recursively(Tags::mesh(),m_mesh.lock()->uri());
          m_setup.lock()->configure_option_recursively(Tags::mesh(),m_mesh.lock()->uri());

          if ( m_time.expired() == false )
          {
            m_solve.lock()->configure_option_recursively(Tags::time(),m_time.lock()->uri());
            m_setup.lock()->configure_option_recursively(Tags::time(),m_time.lock()->uri());

            m_setup.lock()->execute();
            auto_config(*m_solve.lock());
          }
        }
      }
    }
  }
}


void FlowSolver::auto_config(Component& component)
{
  component.configure_option_recursively(Tags::physical_model(),m_physical_model.lock()->uri());
  component.configure_option_recursively(Tags::mesh(),m_mesh.lock()->uri());
  component.configure_option_recursively(Tags::time(),m_time.lock()->uri());

  boost_foreach(Field& field, find_components<Field>(*m_mesh.lock()) )
    component.configure_option_recursively(field.name(), field.uri());
}


void FlowSolver::solve()
{
  if ( m_solve.expired() ) throw SetupError (FromHere(),"solve not set");
  if ( m_setup.expired() ) throw SetupError (FromHere(),"setup not set");
  if ( m_physical_model.expired() ) throw SetupError (FromHere(),"physical_model not set");
  if ( m_mesh.expired() )  throw SetupError (FromHere(),"mesh not set");
  if ( m_time.expired() )  throw SetupError (FromHere(),"time not set");

  m_solve.lock()->execute();
}

void FlowSolver::execute()
{
  solve();
}


CAction& FlowSolver::create_solve(const std::string& name, const std::string& solve_builder_name)
{
  configure_option("solve",create_component(name,solve_builder_name).uri());
  m_solve.lock()->mark_basic();
  return *m_solve.lock();
}


CAction& FlowSolver::create_setup(const std::string& name, const std::string& setup_builder_name)
{
  configure_option("setup",create_component(name,setup_builder_name).uri());
  return *m_setup.lock();
}


CAction& FlowSolver::create_bc_action(const std::string& name, const std::string& builder_name, const std::vector<CRegion::ConstPtr>& regions)
{
  if (m_bc.expired())
    throw SetupError(FromHere(),"component to execute bc's was not set");

  std::vector<URI> regions_uri; regions_uri.reserve(regions.size());
  boost_foreach(CRegion::ConstPtr region, regions)
    regions_uri.push_back(region->uri());

  CAction& bc_action = m_bc.lock()->create_component(name,builder_name).as_type<CAction>();
  bc_action.configure_option(Solver::Tags::regions(),regions_uri);
  auto_config(bc_action);
  return bc_action;
}


CAction& FlowSolver::create_bc_action(const std::string& name, const std::string& builder_name, const CRegion& region)
{
  return create_bc_action(name,
                          builder_name,
                          std::vector<CRegion::ConstPtr>(1,region.as_ptr<CRegion>()));
}


CAction& FlowSolver::create_inner_action(const std::string& name, const std::string& builder_name, const std::vector<CRegion::ConstPtr>& regions)
{
  if (m_bc.expired())
    throw SetupError(FromHere(),"component to execute inner actions was not set");

  std::vector<URI> regions_uri; regions_uri.reserve(regions.size());
  boost_foreach(CRegion::ConstPtr region, regions)
    regions_uri.push_back(region->uri());

  CAction& inner_action = m_inner.lock()->create_component(name,builder_name).as_type<CAction>();
  inner_action.configure_option(Solver::Tags::regions(),regions_uri);
  auto_config(inner_action);
  return inner_action;
}


CAction& FlowSolver::create_inner_action(const std::string& name, const std::string& builder_name, const CRegion& region)
{
  return create_inner_action(name,
                             builder_name,
                             std::vector<CRegion::ConstPtr>(1,region.as_ptr<CRegion>()));
}


void FlowSolver::signal_create_bc_action( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("name");
  std::string builder = options.value<std::string>("builder");
  std::vector<URI> regions_uri = options.array<URI>(Solver::Tags::regions());
  std::vector<CRegion::ConstPtr> regions(regions_uri.size());
  for (Uint i=0; i<regions_uri.size(); ++i)
  {
    regions[i] = access_component(regions_uri[i]).as_ptr_checked<CRegion>();
  }
  create_bc_action(name,builder,regions);
}


void FlowSolver::signature_create_bc_action( SignalArgs& node )
{
  SignalOptions options( node );

  // builder
  //  CFactory::Ptr bc_factory = Common::Core::instance().factories().get_factory<BC>();
  //  std::vector<std::string> bcs;

  //  // build the restricted list
  //  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *bc_factory ) )
  //      bcs.push_back(bdr.name());

  // create de value and add the restricted list
  //  options.add_option< OptionT<std::string> >( "builder", std::string() , "Choose BC", bcs, " ; ");

  // name
  options.add_option< OptionT<std::string> >("name", std::string())
      ->description("Name for created boundary condition" );

  // builder
  options.add_option< OptionT<std::string> >( "builder", std::string() )
      ->description("Builder name of boundary condition computation");

  // regions
  options.add_option< OptionArrayT<URI> >(Solver::Tags::regions(), std::vector<URI>() )
      ->description("Regions where to apply the boundary condition");

}


void FlowSolver::signal_create_inner_action( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("name");
  std::string builder = options.value<std::string>("builder");
  std::vector<URI> regions_uri = options.array<URI>(Solver::Tags::regions());
  std::vector<CRegion::ConstPtr> regions(regions_uri.size());
  for (Uint i=0; i<regions_uri.size(); ++i)
  {
    regions[i] = access_component(regions_uri[i]).as_ptr_checked<CRegion>();
  }
  create_bc_action(name,builder,regions);
}


void FlowSolver::signature_create_inner_action( SignalArgs& node )
{
  SignalOptions options( node );

  // name
  options.add_option< OptionT<std::string> >("name", std::string())
      ->description("Name for created inner action" );

  // builder
  options.add_option< OptionT<std::string> >( "builder", std::string())
      ->description("Builder name of inner action");

  // regions
  options.add_option< OptionArrayT<URI> >(Solver::Tags::regions(), std::vector<URI>())
      ->description("Regions where to apply the boundary condition");

}

////////////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
