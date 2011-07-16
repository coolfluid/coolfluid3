// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CNodes.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/Actions/CSolveSystem.hpp"

#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/Expression.hpp"

#include "BoundaryConditions.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Common::XML;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

struct BoundaryConditions::Implementation
{
  Implementation(CActionDirector& comp) :
    m_component(comp),
    m_physical_model(),
    m_proxy(*(m_component.options().add_option< OptionComponent<CEigenLSS> >("lss", URI())
              ->set_pretty_name("LSS")
              ->set_description("The referenced linear system solver")),
            *(m_component.options().add_option( OptionComponent<Physics::PhysModel>::create("physical_model", &m_physical_model) )
              ->set_pretty_name("Physical Model")
              ->set_description("Physical Model"))
           ),
    dirichlet(m_proxy)
  {
    m_component.options().add_option< OptionArrayT < URI > > ("regions")
      ->set_pretty_name("Regions")
      ->set_description("Regions the boundary condition applies to")
      ->link_to(&m_region_uris);
  }

  CAction::Ptr create_scalar_bc(const std::string& region_name, const std::string& variable_name, const Real default_value)
  {
    MeshTerm<0, ScalarField> var(variable_name, variable_name);
    ConfigurableConstant<Real> value("value", "Value for constant boundary condition", default_value);

    return create_proto_action("BC"+region_name+variable_name, nodes_expression(dirichlet(var) = value));
  }

  CAction::Ptr create_vector_bc(const std::string& region_name, const std::string& variable_name, const RealVector default_value)
  {
    MeshTerm<0, VectorField> var(variable_name, variable_name);
    ConfigurableConstant<RealVector> value("value", "Value for constant boundary condition", default_value);

    return create_proto_action("BC"+region_name+variable_name, nodes_expression(dirichlet(var) = value));
  }

  void add_constant_bc_signature(SignalArgs& node)
  {
    SignalOptions options( node );

    options.add_option< OptionT<std::string> >("region_name", std::string())
        ->set_description("Default region name for this BC");

    options.add_option< OptionT<std::string> >("variable_name", std::string())
        ->set_description("Variable name for this BC");
  }

  // Checked access to the physical model
  Physics::PhysModel& physical_model()
  {
    if(m_physical_model.expired())
      throw SetupError(FromHere(), "Error accessing physical_model from " + m_component.uri().string());
    
    return *m_physical_model.lock();
  }
  
  CActionDirector& m_component;
  boost::weak_ptr<Physics::PhysModel> m_physical_model;
  LSSProxy m_proxy;
  DirichletBC dirichlet;
  std::vector<URI> m_region_uris;
};

BoundaryConditions::BoundaryConditions(const std::string& name) :
  CActionDirector(name),
  m_implementation( new Implementation(*this) )
{
  regist_signal("add_constant_bc" , "Create a constant Dirichlet BC", "Add Constant BC")
    ->signal->connect( boost::bind(&BoundaryConditions::signal_add_constant_bc, this, _1) );
  signal("add_constant_bc")->signature->connect( boost::bind(&Implementation::add_constant_bc_signature, m_implementation.get(), _1) );
}

BoundaryConditions::~BoundaryConditions()
{
}

void BoundaryConditions::add_constant_bc(const std::string& region_name, const std::string& variable_name, const boost::any default_value)
{
  CAction::Ptr result = m_implementation->physical_model().variable_manager().variable_type(variable_name) == Physics::VariableManager::SCALAR ?
    m_implementation->create_scalar_bc(region_name, variable_name, boost::any_cast<Real>(default_value)) :
    m_implementation->create_vector_bc(region_name, variable_name, boost::any_cast<RealVector>(default_value));
    
  *this << result; // Append action

  std::vector<URI> bc_regions;
  
  // find the URIs of all the regions that match the region_name
  boost_foreach(const URI& region_uri, m_implementation->m_region_uris)
  {
    Component::Ptr region_comp = access_component_ptr(region_uri);
    if(!region_comp)
      throw SetupError(FromHere(), "No component found at " + region_uri.string() + " when reading regions from " + uri().string());
    CRegion::Ptr root_region = boost::dynamic_pointer_cast<CRegion>(region_comp);
    if(!root_region)
      throw SetupError(FromHere(), "Component at " + region_uri.string() + " is not a region when reading regions from " + uri().string());
    
    CRegion::Ptr region = find_component_ptr_recursively_with_name<CRegion>(*root_region, region_name);
    if(region)
      bc_regions.push_back(region->uri());
  }
  
  // debug output
  if(bc_regions.empty())
  {
    CFdebug << "No default regions found for region name " << region_name << CFendl;
  }
  else
  {
    CFdebug << "Region name " << region_name << " resolved to:\n";
    boost_foreach(const URI& uri, bc_regions)
    {
      CFdebug << "  " << uri.string() << CFendl;
    }
  }

  result->configure_option("regions", bc_regions);
  result->configure_option("physical_model", m_implementation->m_physical_model);
}

void BoundaryConditions::signal_add_constant_bc(SignalArgs& node)
{
  SignalOptions options( node );
  
  const std::string region_name = options.value<std::string>("region_name");
  const std::string variable_name = options.value<std::string>("variable_name");
  
  if(m_implementation->physical_model().variable_manager().variable_type(variable_name) == Physics::VariableManager::SCALAR)
    add_constant_bc(region_name, variable_name, 0.);
  else
    add_constant_bc(region_name, variable_name, RealVector());
}

} // UFEM
} // CF
