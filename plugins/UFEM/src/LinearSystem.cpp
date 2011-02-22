// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"

#include "Solver/Actions/Proto/CProtoElementsAction.hpp"
#include "Solver/Actions/Proto/CProtoNodesAction.hpp"

#include "Solver/CEigenLSS.hpp"

#include "LinearSystem.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Common::XML;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

LinearSystem::LinearSystem(const std::string& name) : CMethod (name)
{
  m_lss_path = boost::dynamic_pointer_cast<Common::OptionURI>( properties().add_option<Common::OptionURI>("LSS", "Linear system solver", std::string()) );
  m_lss_path.lock()->supported_protocol(CF::Common::URI::Scheme::CPATH);
  m_lss_path.lock()->mark_basic();
  m_lss_path.lock()->attach_trigger( boost::bind(&LinearSystem::on_lss_set, this) );

  this->regist_signal("add_dirichlet_bc" , "Add a Dirichlet boundary condition", "Add Dirichlet BC")->connect( boost::bind ( &LinearSystem::add_dirichlet_bc, this, _1 ) );
  signal("add_dirichlet_bc").signature->connect( boost::bind( &LinearSystem::dirichlet_bc_signature, this, _1) );

  this->regist_signal("add_initial_condition" , "Add an initial condition for a field", "Add Initial Condition")->connect( boost::bind ( &LinearSystem::add_initial_condition, this, _1 ) );
  signal("add_initial_condition").signature->connect( boost::bind( &LinearSystem::add_initial_condition_signature, this, _1) );

  this->regist_signal("run" , "Run the method", "Run")->connect( boost::bind ( &LinearSystem::run, this, _1 ) );
  this->regist_signal("initialize" , "Initialize the solution", "Initialize")->connect( boost::bind ( &LinearSystem::signal_initialize_fields, this, _1 ) );

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;
  signal("run_operation").is_hidden   = true;
}

CEigenLSS& LinearSystem::lss()
{
  CEigenLSS::Ptr lss_ptr = m_lss.lock();
  if(lss_ptr == nullptr)
    throw ValueNotFound(FromHere(), "LSS was not set for LinearSystem " + path().string());

  return *m_lss.lock();
}

void LinearSystem::dirichlet_bc_signature(Signal::arg_t& args)
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("BCName", std::string(), "Name for the boundary condition" );
  p.set_option<std::string>("FieldName", std::string(), "Name for the field for which to set the BC" );
  p.set_option<std::string>("VariableName", std::string(), "Name for the variable for which to set the BC" );
}


void LinearSystem::add_dirichlet_bc( Signal::arg_t& node )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  const std::string bc_name = p.get_option<std::string>("BCName");
  const std::string field_name = p.get_option<std::string>("FieldName");
  const std::string var_name = p.get_option<std::string>("VariableName");

  MeshTerm<0, ConfigurableConstant<Real> > bc_value(bc_name, "Dirichlet boundary condition");
  MeshTerm<1, Field<Real> > bc_var(field_name, var_name);

  CAction::Ptr bc = build_nodes_action(bc_name, *this, dirichlet( lss(), bc_var ) = bc_value );
  bc->add_tag("dirichlet_bc");
}

void LinearSystem::add_initial_condition_signature(Signal::arg_t& node)
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("Name", std::string(), "Name for the initial condition" );
  p.set_option<std::string>("FieldName", std::string(), "Name for the field for which to set the initial values" );
  p.set_option<std::string>("VariableName", std::string(), "Name for the variable for which to set the initial values" );
}


void LinearSystem::add_initial_condition( Signal::arg_t& node )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  const std::string name = p.get_option<std::string>("Name");
  const std::string field_name = p.get_option<std::string>("FieldName");
  const std::string var_name = p.get_option<std::string>("VariableName");

  MeshTerm<0, ConfigurableConstant<Real> > value(name, "Value for the initial value");
  MeshTerm<1, Field<Real> > var(field_name, var_name);

  CAction::Ptr bc = build_nodes_action(name, *this, var = value );
  bc->add_tag("initial_condition");
}


void LinearSystem::run(Signal::arg_t& node)
{
  on_run();
}

void LinearSystem::on_run()
{
  // Action component that will build the linear system
  CFieldAction::Ptr builder = m_system_builder.lock();
  if(!builder)
    throw ValueNotFound(FromHere(), "System builder action not found. Did you set a LSS?");

  // Region we loop over
  CRegion::Ptr region = look_component<CRegion>( builder->property("Region").value_str() );
  if(!region)
    throw ValueNotFound(FromHere(), "Region at path " + get_child("HeatEquation")->property("ConductivityRegion").value_str() + " not found when looking for calculation Region.");

  CMesh& mesh = Common::find_parent_component<Mesh::CMesh>(*region);

  // Build the system
  builder->execute();

  // Set the boundary conditions
  boost_foreach(CAction& bc_action, find_components_with_tag<CAction>(*this, "dirichlet_bc"))
  {
    bc_action.execute();
  }

  // Solve the linear system
  lss().solve();
  increment_solution(lss().solution(), builder->field_names(), builder->variable_names(), builder->variable_sizes(), mesh);
}


void LinearSystem::on_lss_set()
{
  // Remove the old system builder, if needed
  if(m_system_builder.lock())
    remove_component( m_system_builder.lock()->name() );

  // Any boundary conditions are invalidated by setting the LSS
  boost_foreach(CAction& bc_action, find_components_with_tag<CAction>(*this, "dirichlet_bc"))
  {
    remove_component( bc_action.name() );
  }

  m_lss = look_component<CEigenLSS>( m_lss_path.lock()->value_str() );

  // Build the action that will create the system matrix. This requires access to the LSS! This also automatically adds the CAction as a child.
  m_system_builder = build_equation();
}

void LinearSystem::signal_initialize_fields(Signal::arg_t& node)
{
  CFieldAction::Ptr builder = m_system_builder.lock();
  if(!builder)
    throw ValueNotFound(FromHere(), "System builder action not found. Did you set a LSS?");

  // Create the fields and adjust the LSS size
  builder->create_fields();
  lss().resize( builder->nb_dofs() );

  // Set the initial values
  boost_foreach(CAction& action, find_components_with_tag<CAction>(*this, "initial_condition"))
  {
    action.execute();
  }
}

} // UFEM
} // CF
