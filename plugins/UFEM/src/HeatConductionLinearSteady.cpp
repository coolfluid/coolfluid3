// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"

#include "Actions/Proto/CProtoElementsAction.hpp"
#include "Actions/Proto/CProtoNodesAction.hpp"

#include "Solver/CEigenLSS.hpp"

#include "HeatConductionLinearSteady.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Common::String;
using namespace Mesh;
using namespace Solver;
using namespace Actions;
using namespace Actions::Proto;

CF::Common::ComponentBuilder < UFEM::HeatConductionLinearSteady, CMethod, LibUFEM > aHeatConductionLinearSteady_Builder;

HeatConductionLinearSteady::HeatConductionLinearSteady(const std::string& name) : CMethod (name), m_blocks("LSS")
{
  this->regist_signal("add_dirichlet_bc" , "Add a Dirichlet boundary condition", "Add Dirichlet BC")->connect( boost::bind ( &HeatConductionLinearSteady::add_dirichlet_bc, this, _1 ) );
  signal("add_dirichlet_bc").signature.insert<std::string>("BCName", "Name of the boundary condition" );

  this->regist_signal("run" , "Run the method", "Run")->connect( boost::bind ( &HeatConductionLinearSteady::run, this, _1 ) );

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;
  signal("run_operation").is_hidden   = true;
}

void HeatConductionLinearSteady::initialize()
{
  // Variable holding the geometric support
  MeshTerm<0, ConstNodes> nodes("ConductivityRegion");

  // Variable holding the output field. Note that it can be const from the perspective
  MeshTerm<1, ConstField<Real> > temperature("Temperature", "T");

  // Create a CAction to build the system matrix
  build_elements_action("HeatEquation", *this, m_blocks += integral<1>( laplacian(nodes, temperature) * jacobian_determinant(nodes) ));
}


void HeatConductionLinearSteady::add_dirichlet_bc( XmlNode& node )
{
  XmlParams p(node);

  const std::string bc_name = p.get_option<std::string>("BCName");

  MeshTerm<3, ConfigurableConstant<Real> > boundary_temperature(bc_name, "Temperature boundary condition");
  CAction::Ptr xneg_action = build_nodes_action(bc_name, *this, dirichlet(m_blocks) = boundary_temperature );
  xneg_action->add_tag("dirichlet_bc");
  xneg_action->configure_property("LSS", URI(get_child("HeatEquation")->property("LSS").value_str()));
}


void HeatConductionLinearSteady::run(XmlNode& node)
{
  // Region we loop over
  CRegion::Ptr region = look_component<CRegion>( get_child("HeatEquation")->property("ConductivityRegion").value_str() );
  if(!region)
    throw ValueNotFound(FromHere(), "Region at path " + get_child("HeatEquation")->property("ConductivityRegion").value_str() + " not found when looking for ConductivityRegion.");

  // Mesh
  CMesh::Ptr mesh = boost::dynamic_pointer_cast<CMesh>(region->get_parent());
  if(!mesh)
    throw InvalidStructure(FromHere(), "Parent of ConductivityRegion is not a mesh");

  // LSS
  CEigenLSS::Ptr lss = look_component<CEigenLSS>( get_child("HeatEquation")->property("LSS").value_str() );
  if(!lss)
    throw InvalidStructure(FromHere(), "LSS is not set");

  // field name and var name
  const std::string field_name = get_child("HeatEquation")->property("TemperatureFieldName").value_str();
  const std::string var_name = get_child("HeatEquation")->property("TemperatureVariableName").value_str();

  // Create output field
  const std::vector<std::string> vars(1, var_name + "[1]");
  CField& field = mesh->create_field(field_name, vars, CField::NODE_BASED);
  lss->configure_property("SolutionField", URI(field.full_path()) );

  // Build the system
  CAction::Ptr heat_equation = get_child<CAction>("HeatEquation");
  heat_equation->execute();

  // Set the boundary conditions
  boost_foreach(CAction& bc_action, find_components_with_tag<CAction>(*this, "dirichlet_bc"))
  {
    bc_action.execute();
  }

  // Solve the linear system
  lss->solve();
}

} // UFEM
} // CF
