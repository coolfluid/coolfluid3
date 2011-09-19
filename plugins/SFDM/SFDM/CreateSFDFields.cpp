// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"

#include "Math/VariablesDescriptor.hpp"

#include "Solver/Actions/CForAllCells.hpp"

#include "Mesh/Field.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CCells.hpp"

#include "Physics/Variables.hpp"
#include "Physics/PhysModel.hpp"

#include "SFDM/CreateSFDFields.hpp"
#include "SFDM/SFDSolver.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {

  using namespace Common;
  using namespace Mesh;
  using namespace Solver::Actions;
  using namespace Solver;
  using namespace Physics;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CreateSFDFields, Solver::Action, LibSFDM> CreateSFDFields_builder;

//////////////////////////////////////////////////////////////////////////////

CreateSFDFields::CreateSFDFields( const std::string& name )
  : Solver::Action(name)
{
  properties()["brief"] = std::string("Create Fields for use with SFD");
  properties()["description"] = std::string("Fields to be created: ...");
}

/////////////////////////////////////////////////////////////////////////////

void CreateSFDFields::execute()
{
  const Uint solution_order = solver().option(SFDM::Tags::solution_order()).value<Uint>();

  std::string sfdm_fields_space = "sfdm_fields_P"+to_str(solution_order);

  if ( is_not_null (find_component_ptr_recursively_with_tag<FieldGroup>(mesh(),sfdm_fields_space)))
  {
    CFinfo << "field group ["<<sfdm_fields_space<<"] already exists, check now to create the fields" << CFendl;
  }
  else
  {
    FieldGroup& sfdm_fields = mesh().create_space_and_field_group(sfdm_fields_space,FieldGroup::Basis::CELL_BASED,"CF.Mesh.LagrangeP1");
    sfdm_fields.add_tag(sfdm_fields_space);

    Component& solution_vars = find_component_with_tag(physical_model(),SFDM::Tags::solution_vars());
    Field& solution   = sfdm_fields.create_field(SFDM::Tags::solution(), solution_vars.as_type<Variables>().description().description() );

    Field& residual   = sfdm_fields.create_field(SFDM::Tags::residual(), solution.descriptor().description());
    residual.descriptor().prefix_variable_names("rhs_");

    Field& wave_speed = sfdm_fields.create_field(SFDM::Tags::wave_speed(), "ws[1]");

    Field& update_coeff = sfdm_fields.create_field(SFDM::Tags::update_coeff(), "uc[1]");

    Field& jacob_det = sfdm_fields.create_field(SFDM::Tags::jacob_det(), "jacob_det[1]");

    boost_foreach(CCells& elements, find_components_recursively<CCells>(sfdm_fields.topology()))
    {
      CSpace& space = jacob_det.space(elements);

      const RealMatrix& local_coords = space.shape_function().local_coordinates();

      RealMatrix geometry_coords;
      elements.allocate_coordinates(geometry_coords);

      for (Uint elem=0; elem<elements.size(); ++elem)
      {
        elements.put_coordinates(geometry_coords,elem);

        CConnectivity::ConstRow field_idx = space.indexes_for_element(elem);

        for (Uint node=0; node<local_coords.rows();++node)
        {
          jacob_det[field_idx[node]][0]=elements.element_type().jacobian_determinant(local_coords.row(node),geometry_coords);
        }

      }
    }
  }

//  if ( is_null(mesh.get_child_ptr(FlowSolver::Tags::solution())) )
//  {
//    const Solver::State& solution_state = physical_model().solution_state();
//    std::vector<std::string> types(solution_state.var_names().size(),"scalar");

//    CFinfo <<  "  Creating field \"solution\", cellbased, with vars ";
//    boost_foreach(const std::string& var , solution_state.var_names())
//      CFinfo << var << "["<<1<<"]  ";
//    CFinfo << CFendl;

//    CField& field = mesh.create_component<CField>(FlowSolver::Tags::solution());
//    field.set_topology(mesh.topology());
//    field.configure_option("Space",solution_space);
//    field.configure_option("VarNames",solution_state.var_names());
//    field.configure_option("VarTypes",types);
//    field.configure_option("FieldType",CField::Basis::Convert::instance().to_str(CField::Basis::CELL_BASED));
//    field.create_data_storage();
//  }
//  CField& solution = mesh.get_child(FlowSolver::Tags::solution()).as_type<CField>();

//  if ( is_null(mesh.get_child_ptr(FlowSolver::Tags::residual())) )
//  {
//    CFinfo << "  Creating field \"residual\", cellbased" << CFendl;
//    mesh.create_field(FlowSolver::Tags::residual(),solution);
//  }

//  if ( is_null(mesh.get_child_ptr(FlowSolver::Tags::wave_speed())) )
//  {
//    CFinfo << "  Creating field \"wave_speed\", cellbased" << CFendl;
//    mesh.create_field(FlowSolver::Tags::wave_speed(),CField::Basis::CELL_BASED,"P0","wave_speed[1]");
//  }

//  if ( is_null(mesh.get_child_ptr(FlowSolver::Tags::update_coeff())) )
//  {
//    CFinfo << "  Creating field \"update_coeff\", cellbased" << CFendl;
//    mesh.create_field(FlowSolver::Tags::update_coeff(),CField::Basis::CELL_BASED,"P0","update_coeff[1]");
//  }

//  if ( is_null(mesh.get_child_ptr("jacobian_determinant")) )
//  {
//    CFinfo << "  Creating field \"jacobian_determinant\", cell_based" << CFendl;
//    CField& jacobian_determinant = mesh.create_scalar_field("jacobian_determinant",solution);

//    CLoop& compute_jacobian_determinant = create_component< CForAllCells >("compute_jacobian_determinant");
//    compute_jacobian_determinant.configure_option("regions", std::vector<URI>(1,mesh.topology().uri()));
//    compute_jacobian_determinant.create_loop_operation("CF.SFDM.ComputeJacobianDeterminant")
//                                                .configure_option("jacobian_determinant",jacobian_determinant.uri());
//    compute_jacobian_determinant.execute();
//    remove_component(compute_jacobian_determinant.name());
//  }
}

//////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
