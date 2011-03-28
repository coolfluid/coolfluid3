// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/Signal.hpp"
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

/// (Internal) Wraps a constant that can be configured through an option
class ConfigurableConstant : public Component
{
public:
  typedef boost::shared_ptr<ConfigurableConstant> Ptr;
  typedef boost::shared_ptr<ConfigurableConstant const> ConstPtr;
  
  ConfigurableConstant(const std::string& name) : Component(name)
  {
  }
  
  static std::string type_name () { return "ConfigurableConstant"; }

  /// Setup a default value and property
  void setup(const std::string& description, const Real default_value)
  {
    m_description = description;
    m_value = default_value;
  }
  
  /// Add the property to modify the value to the given component
  void add_property(Component::Ptr parent)
  {
    parent->properties().add_option< OptionT<Real> >(name(), m_description, m_value)->mark_basic();
    parent->properties()[name()].as_option().attach_trigger(boost::bind(&ConfigurableConstant::trigger_value, this));
    m_parent = parent;
    parent->add_component(shared_from_this());
  }

  /// Reference to the stored value
  Real& value()
  {
    return m_value;
  }

  void trigger_value()
  {
    m_value = m_parent.lock()->property(name()).value<Real>();
  }

private:  
  Real m_value;
  std::string m_description;
  boost::weak_ptr<Component> m_parent;
};

LinearSystem::LinearSystem(const std::string& name) : Component (name)
{
  m_lss_path = boost::dynamic_pointer_cast<Common::OptionURI>( properties().add_option<Common::OptionURI>("LSS", "Linear system solver", std::string()) );
  m_lss_path.lock()->supported_protocol(CF::Common::URI::Scheme::CPATH);
  m_lss_path.lock()->mark_basic();
  m_lss_path.lock()->attach_trigger( boost::bind(&LinearSystem::on_lss_set, this) );

  this->regist_signal("add_dirichlet_bc" , "Add a Dirichlet boundary condition", "Add Dirichlet BC")->signal->connect( boost::bind ( &LinearSystem::add_dirichlet_bc, this, _1 ) );
  signal("add_dirichlet_bc")->signature->connect( boost::bind( &LinearSystem::dirichlet_bc_signature, this, _1) );

  this->regist_signal("add_initial_condition" , "Add an initial condition for a field", "Add Initial Condition")->signal->connect( boost::bind ( &LinearSystem::add_initial_condition, this, _1 ) );
  signal("add_initial_condition")->signature->connect( boost::bind( &LinearSystem::add_initial_condition_signature, this, _1) );

  this->regist_signal("run" , "Run the method", "Run")->signal->connect( boost::bind ( &LinearSystem::run, this, _1 ) );
  this->regist_signal("initialize" , "Initialize the solution", "Initialize")->signal->connect( boost::bind ( &LinearSystem::signal_initialize_fields, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;
}

CEigenLSS& LinearSystem::lss()
{
  CEigenLSS::Ptr lss_ptr = m_lss.lock();
  if(lss_ptr == nullptr)
    throw ValueNotFound(FromHere(), "LSS was not set for LinearSystem " + path().string());

  return *m_lss.lock();
}

void LinearSystem::dirichlet_bc_signature(SignalArgs& args)
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("BCName", std::string(), "Name for the boundary condition" );
  p.set_option<std::string>("FieldName", std::string(), "Name for the field for which to set the BC" );
  p.set_option<std::string>("VariableName", std::string(), "Name for the variable for which to set the BC" );
}


void LinearSystem::add_dirichlet_bc( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  const std::string bc_name = p.get_option<std::string>("BCName");
  const std::string field_name = p.get_option<std::string>("FieldName");
  const std::string var_name = p.get_option<std::string>("VariableName");

  ConfigurableConstant::Ptr bc_value = allocate_component<ConfigurableConstant>(bc_name);
  bc_value->setup("Dirichlet boundary condition value", 0.);
  
  MeshTerm<0, ScalarField> bc_var(field_name, var_name);

  CAction::Ptr bc = build_nodes_action(bc_name, *this, m_physical_model, dirichlet( lss(), bc_var ) = store(bc_value->value()) );
  bc->add_tag("dirichlet_bc");
  
  // Make sure the BC value can be configured from the BC action itself
  bc_value->add_property(bc);
}

void LinearSystem::add_initial_condition_signature(SignalArgs& args)
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("Name", std::string(), "Name for the initial condition" );
  p.set_option<std::string>("FieldName", std::string(), "Name for the field for which to set the initial values" );
  p.set_option<std::string>("VariableName", std::string(), "Name for the variable for which to set the initial values" );
}


void LinearSystem::add_initial_condition( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  const std::string name = p.get_option<std::string>("Name");
  const std::string field_name = p.get_option<std::string>("FieldName");
  const std::string var_name = p.get_option<std::string>("VariableName");

  ConfigurableConstant::Ptr value = allocate_component<ConfigurableConstant>(name);
  value->setup("Initial condition value", 0.);
  
  MeshTerm<0, ScalarField> var(field_name, var_name);

  CAction::Ptr bc = build_nodes_action(name, *this, var = store(value->value()) );
  bc->add_tag("initial_condition");
  
  value->add_property(bc);
}


void LinearSystem::run(SignalArgs& node)
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
  CRegion::Ptr region = access_component_ptr( builder->property("Region").value_str() )->as_ptr<CRegion>();
  if(!region)
    throw ValueNotFound(FromHere(), "Cannot Region at path " +  builder->property("Region").value_str() + " not found when looking for calculation Region.");


  CMesh& mesh = Common::find_parent_component<Mesh::CMesh>(*region);

  // Build the system
  std::cout << "starting system build" << std::endl;
  builder->execute();
  std::cout << "finished system build" << std::endl;
  
  // Calculate the offsets of the variables (so the bcs know how the system matrix needs to be updated)
  const CFieldAction::StringsT var_names = builder->variable_names();
  const CFieldAction::BoolsT eq_vars = builder->equation_variables();
  const CFieldAction::SizesT var_sizes = builder->variable_sizes();
  m_physical_model.nb_dofs = 0;
  for(Uint var_idx = 0; var_idx != var_names.size(); ++var_idx)
  {
    if(eq_vars[var_idx])
    {
      m_physical_model.variable_offsets[var_names[var_idx]] = m_physical_model.nb_dofs;
      m_physical_model.nb_dofs += var_sizes[var_idx];
    }
  }
  
  // Set the boundary conditions
  std::cout << "starting bc setting" << std::endl;
  boost_foreach(CAction& bc_action, find_components_with_tag<CAction>(*this, "dirichlet_bc"))
  {
    bc_action.execute();
  }
  std::cout << "finished bc setting" << std::endl;

  // Solve the linear system
  std::cout << "starting solve" << std::endl;
  lss().solve();
  std::cout << "finished solve" << std::endl;
  builder->update_fields(lss().solution(), mesh);
  std::cout << "finished solution update" << std::endl;
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

  m_lss = access_component_ptr( m_lss_path.lock()->value_str() )->as_ptr<CEigenLSS>();

  // Build the action that will create the system matrix. This requires access to the LSS! This also automatically adds the CAction as a child.
  m_system_builder = build_equation();
  
  // Move any configurable constants to the system builder component
  for(ConstantsT::iterator it = m_configurable_constants.begin(); it != m_configurable_constants.end(); ++it)
    boost::dynamic_pointer_cast<ConfigurableConstant>(it->second)->add_property(m_system_builder.lock());
  
  m_configurable_constants.clear();
}

void LinearSystem::signal_initialize_fields(SignalArgs& node)
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

StoredReference<Real> LinearSystem::add_configurable_constant(const std::string& name, const std::string& description, const Real default_value)
{
  boost::shared_ptr<ConfigurableConstant> c = allocate_component<ConfigurableConstant>(name);
  c->setup(description, default_value);
  m_configurable_constants[name] = c;
  return store(c->value());
}



} // UFEM
} // CF
