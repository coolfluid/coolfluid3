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
  void add_property(Component& parent)
  {
    parent.properties().add_option< OptionT<Real> >(name(), m_description, m_value)->mark_basic();
    parent.properties()[name()].as_option().attach_trigger(boost::bind(&ConfigurableConstant::trigger_value, this));
    m_parent = parent.follow();
    parent.add_component(shared_from_this());
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

LinearSystem::LinearSystem(const std::string& name) : CSolver(name)
{
  m_lss_path = boost::dynamic_pointer_cast<Common::OptionURI>( properties().add_option<Common::OptionURI>("LSS", "Linear system solver", std::string()) );
  m_lss_path.lock()->supported_protocol(CF::Common::URI::Scheme::CPATH);
  m_lss_path.lock()->mark_basic();
  m_lss_path.lock()->attach_trigger( boost::bind(&LinearSystem::trigger_lss_set, this) );
  
  m_region_path = boost::dynamic_pointer_cast<Common::OptionURI>( properties().add_option<Common::OptionURI>("Region", "Region over which the problem is solved", std::string()) );
  m_region_path.lock()->supported_protocol(CF::Common::URI::Scheme::CPATH);
  m_region_path.lock()->mark_basic();

  this->regist_signal("add_dirichlet_bc" , "Add a Dirichlet boundary condition", "Add Dirichlet BC")->signal->connect( boost::bind ( &LinearSystem::signal_add_dirichlet_bc, this, _1 ) );
  signal("add_dirichlet_bc")->signature->connect( boost::bind( &LinearSystem::signature_add_dirichlet_bc, this, _1) );

  this->regist_signal("add_initial_condition" , "Add an initial condition for a field", "Add Initial Condition")->signal->connect( boost::bind ( &LinearSystem::signal_add_initial_condition, this, _1 ) );
  signal("add_initial_condition")->signature->connect( boost::bind( &LinearSystem::signature_add_initial_condition, this, _1) );

  this->regist_signal("initialize" , "Initialize the solution", "Initialize")->signal->connect( boost::bind ( &LinearSystem::signal_initialize_fields, this, _1 ) );

  // Add the directors for the different phases
  m_phases[POST_INIT] = this->create_component_ptr<CActionDirector>("PhasePostInit");
  m_phases[ASSEMBLY] = this->create_component_ptr<CActionDirector>("PhaseAssembly");
  m_phases[POST_SOLVE] = this->create_component_ptr<CActionDirector>("PhasePostSolve");
  m_phases[POST_INCREMENT] = this->create_component_ptr<CActionDirector>("PhasePostIncrement");
  
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

void LinearSystem::signature_add_dirichlet_bc(SignalArgs& args)
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("BCName", std::string(), "Name for the boundary condition" );
  p.set_option<std::string>("FieldName", std::string(), "Name for the field for which to set the BC" );
  p.set_option<std::string>("VariableName", std::string(), "Name for the variable for which to set the BC" );
}


void LinearSystem::signal_add_dirichlet_bc( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  const std::string bc_name = p.get_option<std::string>("BCName");
  const std::string field_name = p.get_option<std::string>("FieldName");
  const std::string var_name = p.get_option<std::string>("VariableName");

  ConfigurableConstant::Ptr bc_value = allocate_component<ConfigurableConstant>(bc_name);
  bc_value->setup("Dirichlet boundary condition value", 0.);
  
  MeshTerm<0, ScalarField> bc_var(field_name, var_name);

  CAction& bc = build_nodes_action(bc_name, *this, *this, m_physical_model, dirichlet( lss(), bc_var ) = store(bc_value->value()) );
  bc.add_tag("dirichlet_bc");
  
  // Make sure the BC value can be configured from the BC action itself
  bc_value->add_property(bc);
}

void LinearSystem::signature_add_initial_condition(SignalArgs& args)
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("Name", std::string(), "Name for the initial condition" );
  p.set_option<std::string>("FieldName", std::string(), "Name for the field for which to set the initial values" );
  p.set_option<std::string>("VariableName", std::string(), "Name for the variable for which to set the initial values" );
}


void LinearSystem::signal_add_initial_condition( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  const std::string name = p.get_option<std::string>("Name");
  const std::string field_name = p.get_option<std::string>("FieldName");
  const std::string var_name = p.get_option<std::string>("VariableName");

  ConfigurableConstant::Ptr value = allocate_component<ConfigurableConstant>(name);
  value->setup("Initial condition value", 0.);
  
  MeshTerm<0, ScalarField> var(field_name, var_name);

  CAction& ic = build_nodes_action(name, *this, *this, m_physical_model, var = store(value->value()), m_region_path.lock() );
  ic.add_tag("initial_condition");
  
  value->add_property(ic);
}

void LinearSystem::solve()
{
  on_solve();
}

void LinearSystem::signal_initialize_fields(SignalArgs& node)
{
  if(!m_lss.lock())
    throw ValueNotFound(FromHere(), "Linear system solver not set, can't initialize!");

  CRegion::Ptr region = access_component_ptr( m_region_path.lock()->value_str() )->as_ptr<CRegion>();
  if(!region)
    throw ValueNotFound(FromHere(), "Region at path " +  m_region_path.lock()->value_str() + " not found when looking for calculation Region.");

  CMesh& mesh = Common::find_parent_component<Mesh::CMesh>(*region);
  
  // Create the fields and adjust the LSS size
  m_physical_model.create_fields(mesh, properties());
  lss().resize( m_physical_model.nb_dofs() * mesh.nodes().size() );

  // Set the initial values
  boost_foreach(CAction& action, find_components_with_tag<CAction>(*this, "initial_condition"))
  {
    action.execute();
  }
  
  // Post-init phase
  m_phases[POST_INIT].lock()->execute();
}

void LinearSystem::on_solve()
{
  if(!m_lss.lock())
    throw ValueNotFound(FromHere(), "Linear system solver not set, can't solve!");

  // Region we loop over
  CRegion::Ptr region = access_component_ptr( m_region_path.lock()->value_str() )->as_ptr<CRegion>();
  if(!region)
    throw ValueNotFound(FromHere(), "Region at path " +  m_region_path.lock()->value_str() + " not found when looking for calculation Region.");

  CMesh& mesh = Common::find_parent_component<Mesh::CMesh>(*region);

  // Build the system
  m_phases[ASSEMBLY].lock()->execute();
  
  // Set the boundary conditions
  boost_foreach(CAction& bc_action, find_components_with_tag<CAction>(*this, "dirichlet_bc"))
  {
    bc_action.execute();
  }

  // Solve the linear system
  lss().solve();
  
  // Post-solve phase
  m_phases[POST_SOLVE].lock()->execute();
  
  // Increment solution
  m_physical_model.update_fields(mesh, lss().solution());
  
  // Post increment phase
  m_phases[POST_INCREMENT].lock()->execute();
}


void LinearSystem::trigger_lss_set()
{
  // Remove actions from the directors
  m_phases[POST_INIT].lock()->property("ActionList").change_value(std::vector<URI>());
  m_phases[ASSEMBLY].lock()->property("ActionList").change_value(std::vector<URI>());
  m_phases[POST_SOLVE].lock()->property("ActionList").change_value(std::vector<URI>());
  m_phases[POST_INCREMENT].lock()->property("ActionList").change_value(std::vector<URI>());
  
  // Any boundary conditions are invalidated by setting the LSS
  boost_foreach(CAction& bc_action, find_components_with_tag<CAction>(*this, "dirichlet_bc"))
  {
    remove_component( bc_action.name() );
  }

  m_lss = access_component_ptr( m_lss_path.lock()->value_str() )->as_ptr<CEigenLSS>();

  add_actions();
  
  // Move any configurable constants to the system builder component
  for(ConstantsT::iterator it = m_configurable_constants.begin(); it != m_configurable_constants.end(); ++it)
    boost::dynamic_pointer_cast<ConfigurableConstant>(it->second)->add_property(*this);
  
  m_configurable_constants.clear();
}

StoredReference<Real> LinearSystem::add_configurable_constant(const std::string& name, const std::string& description, const Real default_value)
{
  boost::shared_ptr<ConfigurableConstant> c = allocate_component<ConfigurableConstant>(name);
  c->setup(description, default_value);
  m_configurable_constants[name] = c;
  return store(c->value());
}

void LinearSystem::append_action(CAction& action, const LinearSystem::PhasesT phase)
{
  Property& actions_prop = m_phases[phase].lock()->property("ActionList");
  std::vector<URI> actions; actions_prop.put_value(actions);
  
  actions.push_back(action.full_path());
  actions_prop.change_value(actions);
}


} // UFEM
} // CF
