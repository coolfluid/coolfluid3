// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "Math/VariableManager.hpp"
#include "Math/VariablesDescriptor.hpp"

#include "Math/LSS/System.hpp"

#include "Mesh/Geometry.hpp"

#include "Solver/Actions/CSolveSystem.hpp"

#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/Tags.hpp"

#include "BoundaryConditions.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace common::XML;
using namespace Math;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BoundaryConditions, CActionDirector, LibUFEM > BoundaryConditions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct BoundaryConditions::Implementation
{
  Implementation(CActionDirector& comp) :
    m_component(comp),
    m_physical_model(),
    dirichlet(*m_component.options().add_option< OptionComponent<LSS::System> >("lss", URI())
              ->pretty_name("LSS")
              ->description("The referenced linear system solver"))
  {
    m_component.options().add_option< OptionArrayT < URI > > (Solver::Tags::regions())
      ->pretty_name("Regions")
      ->description("Regions the boundary condition applies to")
      ->link_to(&m_region_uris);
    m_component.options().add_option< OptionComponent<Physics::PhysModel> >(Solver::Tags::physical_model())
      ->pretty_name("Physical Model")
      ->description("Physical Model")
      ->link_to(&m_physical_model);
  }

  CAction::Ptr create_scalar_bc(const std::string& region_name, const std::string& variable_name, const Real default_value)
  {
    MeshTerm<0, ScalarField> var(variable_name, UFEM::Tags::solution());
    ConfigurableConstant<Real> value("value", "Value for constant boundary condition", default_value);

    return create_proto_action("BC"+region_name+variable_name, nodes_expression(dirichlet(var) = value));
  }

  CAction::Ptr create_vector_bc(const std::string& region_name, const std::string& variable_name, const RealVector default_value)
  {
    MeshTerm<0, VectorField> var(variable_name, UFEM::Tags::solution());
    ConfigurableConstant<RealVector> value("value", "Value for constant boundary condition", default_value);

    return create_proto_action("BC"+region_name+variable_name, nodes_expression(dirichlet(var) = value));
  }

  void add_constant_bc_signature(SignalArgs& node)
  {
    SignalOptions options( node );

    options.add_option< OptionT<std::string> >("region_name", std::string())
        ->description("Default region name for this BC");

    options.add_option< OptionT<std::string> >("variable_name", std::string())
        ->description("Variable name for this BC");
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
  DirichletBC dirichlet;
  std::vector<URI> m_region_uris;
};

BoundaryConditions::BoundaryConditions(const std::string& name) :
  CActionDirector(name),
  m_implementation( new Implementation(*this) )
{
  regist_signal( "add_constant_bc" )
    ->connect( boost::bind( &BoundaryConditions::signal_create_constant_bc, this, _1 ) )
    ->description("Create a constant Dirichlet BC")
    ->pretty_name("Add Constant BC")
    ->signature( boost::bind(&Implementation::add_constant_bc_signature, m_implementation.get(), _1) );
}

BoundaryConditions::~BoundaryConditions()
{
}

void BoundaryConditions::add_constant_bc(const std::string& region_name, const std::string& variable_name, const boost::any default_value)
{
  const VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(m_implementation->physical_model().variable_manager(), UFEM::Tags::solution());

  CAction::Ptr result = descriptor.dimensionality(variable_name) == VariablesDescriptor::Dimensionalities::SCALAR ?
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

  result->configure_option(Solver::Tags::regions(), bc_regions);
  result->configure_option(Solver::Tags::physical_model(), m_implementation->m_physical_model);
}

void BoundaryConditions::signal_create_constant_bc(SignalArgs& node)
{
  SignalOptions options( node );

  const std::string region_name = options.value<std::string>("region_name");
  const std::string variable_name = options.value<std::string>("variable_name");

  const VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(m_implementation->physical_model().variable_manager(), UFEM::Tags::solution());

  if(descriptor.dimensionality(variable_name) == VariablesDescriptor::Dimensionalities::SCALAR)
    add_constant_bc(region_name, variable_name, 0.);
  else
    add_constant_bc(region_name, variable_name, RealVector());
}

} // UFEM
} // cf3
