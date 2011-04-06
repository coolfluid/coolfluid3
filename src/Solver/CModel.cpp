// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
#include <boost/algorithm/string/find.hpp>

#include "Common/BoostFilesystem.hpp"

#include "Common/Signal.hpp"
#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CreateComponent.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CDomain.hpp"

#include "Solver/CPhysicalModel.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/CModel.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::XML;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

CModel::CModel( const std::string& name  ) :
  Component ( name )
{
   mark_basic();

   // options

   std::string cwd = boost::filesystem::current_path().string();

   m_properties.add_option< OptionURI >("WorkingDir", "Your working directory", URI( cwd ) )
       ->mark_basic();
   m_properties.add_option< OptionURI >("ResultsDir", "Directory to store the output files", URI( cwd ) )
       ->mark_basic();
   m_properties.add_option< OptionT<Uint> >("CPUs", "Number of cpus to use in simulation", 1u )
       ->mark_basic();

   // properties

   properties()["steady"] = bool(true);

   // signals
   SignalPtr sig_create_physics = regist_signal ( "create_physics" , "Create the physical model", "Create Physics" );
        sig_create_physics->signal    -> connect ( boost::bind ( &CModel::signal_create_physics, this, _1 ) );
        sig_create_physics->signature -> connect ( boost::bind ( &CModel::signature_create_physics, this, _1) );

  SignalPtr sig_create_domain = regist_signal ( "create_domain" , "Create the domain and load a mesh", "Create Domain" );
       sig_create_domain->signal    -> connect ( boost::bind ( &CModel::signal_create_domain, this, _1 ) );
       sig_create_domain->signature -> connect ( boost::bind ( &CModel::signature_create_domain, this, _1) );

   SignalPtr sig_create_solver = regist_signal ( "create_solver" , "Create the solver", "Create Solver" );
       sig_create_solver->signal    -> connect ( boost::bind ( &CModel::signal_create_solver, this, _1 ) );
       sig_create_solver->signature -> connect ( boost::bind ( &CModel::signature_create_solver, this, _1) );

   regist_signal ( "simulate" , "Simulates this model", "Simulate" )
       ->signal->connect ( boost::bind ( &CModel::signal_simulate, this, _1 ) );
}

CModel::~CModel() {}

////////////////////////////////////////////////////////////////////////////////

CPhysicalModel& CModel::physics()
{
  return find_component<CPhysicalModel>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CDomain& CModel::domain()
{
  return find_component<CDomain>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CSolver& CModel::solver()
{
  return find_component<CSolver>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CPhysicalModel& CModel::create_physics( const std::string& name )
{
  return *this->create_component<CPhysicalModel>( name );
}

////////////////////////////////////////////////////////////////////////////////

CDomain& CModel::create_domain( const std::string& name )
{
  return *this->create_component<CDomain>( name );
}

////////////////////////////////////////////////////////////////////////////////

CSolver& CModel::create_solver( const std::string& builder_name)
{
  std::string solver_name = builder_name;
  solver_name.erase(solver_name.begin(),boost::algorithm::find_last(solver_name,".").begin()+1);
  
  CSolver::Ptr solver = create_component_abstract_type<CSolver>(builder_name,solver_name);
  add_component(solver);
  return *solver;
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_physics ( Common::SignalArgs& node )
{
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_physics ( Common::SignalArgs& node )
{
  create_physics("Physics");
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_domain ( Common::SignalArgs& node )
{
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_domain ( Common::SignalArgs& node )
{
  SignalFrame& options = node.map( Protocol::Tags::key_options() );

  CDomain& domain = create_domain("Domain"); // dispatch to virtual function
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signature_create_solver ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  CFactory::Ptr solver_factory = Core::instance().factories()->get_factory<CSolver>();
  std::vector<std::string> solvers;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *solver_factory ) )
  {
    solvers.push_back(bdr.name());
  }

  // create de value and add the restricted list
  options.add( "builder", std::string() , "Choose solver", solvers, " ; ");
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_create_solver ( Common::SignalArgs& node )
{
  SignalOptions options( node );
  std::string builder_name = options.option<std::string>( "builder" );
  create_solver(builder_name);
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_simulate ( Common::SignalArgs& node )
{
  // XmlParams p ( node );
  this->simulate(); // dispatch tos virtual function
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
