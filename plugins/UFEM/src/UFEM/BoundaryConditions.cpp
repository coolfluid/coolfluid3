// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CNodes.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/Actions/CSolveSystem.hpp"
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
  Implementation(CProtoActionDirector& comp) :
   m_component(comp),
   m_proxy(*m_component.options().add_option< OptionComponent<CEigenLSS> >("lss", URI()), m_component.option("physical_model")),
   dirichlet(m_proxy)
  {
    m_component.option("lss")
        .set_description("The referenced linear system solver")
        ->set_pretty_name("LSS");
  }

  CAction& add_scalar_bc(const std::string& region_name, const std::string& variable_name, const Real default_value)
  {
    MeshTerm<0, ScalarField> var(variable_name, variable_name);
    ConfigurableConstant<Real> value("value", "Value for constant boundary condition", default_value);

    return m_component.add_action("BC"+region_name+variable_name, nodes_expression(dirichlet(var) = value));
  }

  CAction& add_vector_bc(const std::string& region_name, const std::string& variable_name, const RealVector default_value)
  {
    MeshTerm<0, VectorField> var(variable_name, variable_name);
    ConfigurableConstant<RealVector> value("value", "Value for constant boundary condition", default_value);

    return m_component.add_action("BC"+region_name+variable_name, nodes_expression(dirichlet(var) = value));
  }

  void signal_add_constant_bc(SignalArgs& node)
  {
    SignalOptions options( node );

    const std::string region_name = options.value<std::string>("region_name");
    const std::string variable_name = options.value<std::string>("variable_name");

    BoundaryConditions& my_bc = dynamic_cast<BoundaryConditions&>(m_component);
    if(my_bc.physical_model().variable_type(variable_name) == CF::Solver::CPhysicalModel::SCALAR)
      my_bc << my_bc.add_constant_bc(region_name, variable_name, 0.);
    else
      my_bc << my_bc.add_constant_bc(region_name, variable_name, RealVector());
  }

  void add_constant_bc_signature(SignalArgs& node)
  {
    SignalOptions options( node );

    options.add_option< OptionT<std::string> >("region_name", std::string())
        ->set_description("Default region name for this BC");

    options.add_option< OptionT<std::string> >("variable_name", std::string())
        ->set_description("Variable name for this BC");
  }

  CProtoActionDirector& m_component;
  LSSProxy m_proxy;
  DirichletBC dirichlet;
};

BoundaryConditions::BoundaryConditions(const std::string& name) :
  CProtoActionDirector(name),
  m_implementation( new Implementation(*this) )
{
  m_options["propagate_region"].change_value(false);

  regist_signal("add_constant_bc" , "Create a constant Dirichlet BC", "Add Constant BC")
    ->signal->connect( boost::bind(&Implementation::signal_add_constant_bc, m_implementation.get(), _1) );
  signal("add_constant_bc")->signature->connect( boost::bind(&Implementation::add_constant_bc_signature, m_implementation.get(), _1) );
}

BoundaryConditions::~BoundaryConditions()
{
}

CAction& BoundaryConditions::add_constant_bc(const std::string& region_name, const std::string& variable_name, const boost::any default_value)
{
  CAction& result = physical_model().variable_type(variable_name) == CF::Solver::CPhysicalModel::SCALAR ?
    m_implementation->add_scalar_bc(region_name, variable_name, boost::any_cast<Real>(default_value)) :
    m_implementation->add_vector_bc(region_name, variable_name, boost::any_cast<RealVector>(default_value));

  OptionComponent<CRegion>& region_option = dynamic_cast< OptionComponent<CRegion>& >(option("region"));
  if(region_option.check())
  {
    CRegion::Ptr region = find_component_ptr_recursively_with_name<CRegion>(region_option.component(), region_name);
    if(region)
    {
      CFinfo << "Resolved BC default region as " << region->uri().string() << CFendl;
      result.configure_option("region", region);
    }
    else
    {
      CFwarn << "Default region with name " << region_name << " was not found" << CFendl;
    }
  }

  return result;
}

} // UFEM
} // CF
