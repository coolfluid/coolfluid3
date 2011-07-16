// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionArray.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"

#include "Common/StringConversion.hpp"
#include "Common/CGroupActions.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CTable.hpp"

#include "Mesh/Actions/CInitFieldFunction.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "Solver/Actions/CComputeLNorm.hpp"

#include "RDM/Core/Solver.hpp"
#include "RDM/Core/DomainTerm.hpp"
#include "RDM/Core/BoundaryTerm.hpp"
#include "RDM/Core/Cleanup.hpp"
#include "RDM/Core/UpdateSolution.hpp"
#include "RDM/Core/ForwardEuler.hpp"

namespace CF {
namespace RDM {

using namespace Common;
using namespace Mesh;
using namespace Mesh::Actions;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Solver, CSolver, LibCore > Solver_Builder;

////////////////////////////////////////////////////////////////////////////////

Solver::Solver ( const std::string& name  ) :
  CSolver ( name )
{
  // options

  m_options["domain"].attach_trigger ( boost::bind ( &Solver::config_domain,   this ) );

  m_options.add_option< OptionT<std::string> >( Tags::update_vars(), "")
      ->attach_trigger ( boost::bind ( &Solver::config_physics,   this ) );

  m_options.add_option(OptionComponent<CMesh>::create("mesh", &m_mesh))
      ->set_description("Mesh the Discretization Method will be applied to")
      ->set_pretty_name("Mesh")
      ->attach_trigger ( boost::bind ( &Solver::config_mesh,   this ) );


  m_options.add_option( OptionComponent<Physics::PhysModel>::create("physics", &m_physical_model))
      ->set_description("Physical model to discretize")
      ->set_pretty_name("Physics")
      ->mark_basic()
      ->attach_trigger ( boost::bind ( &Solver::config_physics,   this ) );

  // properties

  properties()["brief"] = std::string("Residual Distribution Solver");
  properties()["description"] = std::string("No long description available");

  // signals

  regist_signal ("initialize_solution" ,
                 "initializes the solution using analytical math formulas",
                 "Initialize the solution" )->
      signal->connect ( boost::bind ( &Solver::signal_initialize_solution, this, _1 ) );

  signal("initialize_solution")->
      signature->connect( boost::bind( &Solver::signature_signal_initialize_solution, this, _1));

  regist_signal ("create_boundary_term" ,
                 "creates a boundary condition term",
                 "Create Boundary Condition" )->
      signal->connect ( boost::bind ( &Solver::signal_create_boundary_term, this, _1 ) );

  signal("create_boundary_term")->
      signature->connect( boost::bind( &Solver::signature_signal_create_boundary_term, this, _1));

  regist_signal ("create_domain_term",
                 "creates a domain volume term",
                 "Create Domain Term" )->
      signal->connect ( boost::bind ( &Solver::signal_create_domain_term, this, _1 ) );

  signal("create_domain_term")->
      signature->connect( boost::bind( &Solver::signature_signal_create_domain_term, this, _1));

  // setup of the static components

  // create apply boundary conditions action
  m_compute_boundary_terms =
      create_static_component_ptr<CGroupActions>("compute_boundary_terms");
  m_compute_boundary_terms->mark_basic();

  // create compute rhs action
  m_compute_domain_terms =
      create_static_component_ptr<CGroupActions>("compute_domain_terms");
  m_compute_domain_terms->mark_basic();

  // additional actions

  m_cleanup      = create_static_component_ptr<Cleanup>("cleanup");
  m_compute_norm = create_static_component_ptr<CF::Solver::Actions::CComputeLNorm>("compute_norm");

  m_time_stepping = create_static_component_ptr<ForwardEuler>("time_stepping");

}

////////////////////////////////////////////////////////////////////////////////

Solver::~Solver() {}

////////////////////////////////////////////////////////////////////////////////

void Solver::config_physics()
{
  using namespace CF::Physics;

  if( is_null(m_physical_model.lock()) )
    return;

  std::string user_vars = option(  Tags::update_vars() ).value<std::string>();
  if( user_vars.empty() )
    return;

  Physics::PhysModel& pm = * m_physical_model.lock();

  Component::Ptr upv =
      find_component_ptr_with_tag(pm, Tags::update_vars());

  if( is_not_null(upv) ) // if exits insure is the good one
  {
    Variables& vars = upv->as_type<Variables>();
    if( vars.type() != user_vars )
    {
      pm.remove_component(vars.name() );
      upv.reset();
    }
  }

  if( is_null(upv) )
    pm.create_variables( user_vars, Tags::update_vars() );

}

////////////////////////////////////////////////////////////////////////////////

void Solver::config_domain()
{
  CDomain::Ptr domain = access_component_ptr( option("domain").value<URI>() )->as_ptr<CDomain>();
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not point to Domain");

  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>( *domain );

//  CFinfo << "mesh " << mesh->uri().string() << CFendl;
  if (is_not_null(mesh))
  {
    m_mesh = mesh;
    config_mesh();
  }
}

////////////////////////////////////////////////////////////////////////////////

void Solver::config_mesh()
{
  if( is_null(m_mesh.lock()) )
      return;

  CMesh& mesh = *(m_mesh.lock());

  Physics::PhysModel::Ptr physmodel = m_physical_model.lock();
  if( is_null( physmodel ) )
    throw SetupError(FromHere(), "Physical model not yet set for Solver component " + uri().string() );

  const Uint nbdofs = physmodel->neqs();

  // configure solution

  std::string solution_tag("solution");
  m_solution = find_component_ptr_with_tag<CField>( mesh, solution_tag );
  if ( is_null( m_solution.lock() ) )
  {
    std::string vars;
    for(Uint i = 0; i < nbdofs; ++i)
    {
     vars += "u" + to_str(i) + "[1]";
     if( i != nbdofs-1 ) vars += ",";
    }

    m_solution =
        mesh.create_field("solution",CField::Basis::POINT_BASED,"space[0]",vars).as_ptr<CField>();

    m_solution.lock()->add_tag(solution_tag);
  }

  // parallelize the solution if not yet done
  m_solution.lock()->parallelize();

  /// @todo here we should check if space() order is correct,
  ///       if not the change space() by enriching or other appropriate action

  // configure residual

  std::string residual_tag("residual");
  m_residual = find_component_ptr_with_tag<CField>( mesh, residual_tag);
  if ( is_null( m_residual.lock() ) )
  {
    m_residual = mesh.create_field("residual",*m_solution.lock()).as_ptr<CField>();
    m_residual.lock()->add_tag(residual_tag);
  }

  // configure wave_speed

  std::string wave_speed_tag("wave_speed");
  m_wave_speed = find_component_ptr_with_tag<CField>( mesh, wave_speed_tag);
  if ( is_null(m_wave_speed.lock()) )
  {
    m_wave_speed = mesh.create_scalar_field("wave_speed",*m_solution.lock()).as_ptr<CField>();
    m_wave_speed.lock()->add_tag(wave_speed_tag);
  }

  // configure field action

  configure_option_recursively("solution", m_solution.lock()->uri() );
  configure_option_recursively("residual", m_residual.lock()->uri() );
  configure_option_recursively("wave_speed", m_wave_speed.lock()->uri() );

  std::vector<URI> cleanup_fields;
  cleanup_fields.push_back( m_residual.lock()->uri() );
  cleanup_fields.push_back( m_wave_speed.lock()->uri() );
  m_cleanup->configure_option("Fields", cleanup_fields);

  m_compute_norm->configure_option("Scale", true);
  m_compute_norm->configure_option("Order", 2u);
  m_compute_norm->configure_option("Field", m_residual.lock()->uri());
}

//////////////////////////////////////////////////////////////////////////////

void Solver::execute()
{
  // ensure domain is sane

  CDomain::Ptr domain = access_component_ptr( option("domain").value<URI>() )->as_ptr<CDomain>();
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not poitn to Domain");

  CSolver::execute();

}

//////////////////////////////////////////////////////////////////////////////

void Solver::signal_create_boundary_term( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("Name");
  std::string type = options.value<std::string>("Type");

  std::vector<URI> regions = options.array<URI>("Regions");

  RDM::BoundaryTerm::Ptr bterm = build_component_abstract_type<RDM::BoundaryTerm>(type,name);
  m_compute_boundary_terms->add_component(bterm);

  bterm->configure_option("regions" , regions);
  if( m_mesh.lock() )
    bterm->configure_option("mesh", m_mesh.lock()->uri());
  if( m_physical_model.lock() )
    bterm->configure_option("physical_model" , m_physical_model.lock()->uri());
}

////////////////////////////////////////////////////////////////////////////////

void Solver::signature_signal_create_boundary_term( SignalArgs& node )
{
  SignalOptions options( node );

  // name
  options.add_option< OptionT<std::string> >("Name", std::string() )
      ->set_description("Name for created boundary term" );

  /// @todo shoould loop on availabe BCs in factory of BCs

  // type
  std::vector< boost::any > restricted;
//  restricted.push_back( std::string("CF.RDM.Core.BcDirichlet") );
  options.add_option< OptionT<std::string> >("Type", std::string("CF.RDM.Core.BcDirichlet") )
      ->set_description("Type for created boundary")
      ->restricted_list() = restricted;

  // regions
  std::vector<URI> dummy;
  // create here the list of restricted surface regions
  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->set_description("Regions where to apply the boundary condition");
}

//////////////////////////////////////////////////////////////////////////////

void Solver::signal_create_domain_term( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("Name");
  std::string type = options.value<std::string>("Type");

  RDM::DomainTerm::Ptr dterm = build_component_abstract_type<RDM::DomainTerm>(type,name);
  m_compute_domain_terms->add_component(dterm);

  std::vector<URI> regions = options.array<URI>("Regions");

  dterm->configure_option("regions" , regions);
  if( m_mesh.lock() )
    dterm->configure_option("mesh", m_mesh.lock()->uri());
  if( m_physical_model.lock() )
    dterm->configure_option("physical_model" , m_physical_model.lock()->uri());
}

////////////////////////////////////////////////////////////////////////////////

void Solver::signature_signal_create_domain_term( SignalArgs& node )
{
  SignalOptions options( node );

  // name
  options.add_option< OptionT<std::string> >("Name", std::string() )
      ->set_description("Name for created volume term");

  // type
//  std::vector< std::string > restricted;
//  restricted.push_back( std::string("CF.RDM.Core.BcDirichlet") );
//  XmlNode type_node = options.add_option< OptionT<std::string> >("Type", std::string("CF.RDM.Core.BcDirichlet"), "Type for created boundary");
//  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions
  std::vector<URI> dummy;
  // create here the list of restricted surface regions
  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->set_description("Regions where to apply the domain term");
}

//////////////////////////////////////////////////////////////////////////////

void Solver::signal_initialize_solution( SignalArgs& node )
{
  if( is_null(m_mesh.lock()) )
      throw SetupError( FromHere(), "Mesh has not been configured on solver " + uri().string() );

  cf_assert( is_not_null(m_solution.lock()) );

  SignalOptions options( node );

  std::vector< std::string > functions = options.array<std::string>("functions");

  CInitFieldFunction::Ptr init_solution;
  if( is_null(get_child_ptr("init_solution")) )
    init_solution = create_component_ptr<CInitFieldFunction>("init_solution");
  else
    init_solution = get_child("init_solution").as_ptr_checked<CInitFieldFunction>();

  init_solution->configure_option( "functions", functions );
  init_solution->configure_option( "field", m_solution.lock()->uri() );

  init_solution->transform( m_mesh.lock() );
}

////////////////////////////////////////////////////////////////////////////////

void Solver::signature_signal_initialize_solution( SignalArgs& node )
{
  SignalOptions options( node );

  std::vector<std::string> dummy;
  options.add_option< OptionArrayT<std::string> >("functions", dummy )
      ->set_description("Analytical function definitions for initial condition (one per DOF, variables are x,y,z)");
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
