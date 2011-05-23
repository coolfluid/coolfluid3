// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

#include "Solver/Actions/CForAllCells.hpp"

#include "Solver/FlowSolver.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CRegion.hpp"

#include "SFDM/CreateSFDFields.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {

  using namespace Common;
  using namespace Mesh;
  using namespace Solver::Actions;
  using namespace Solver;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CreateSFDFields, Solver::Action, LibSFDM> CreateSFDFields_builder;

//////////////////////////////////////////////////////////////////////////////

CreateSFDFields::CreateSFDFields( const std::string& name )
  : Solver::Action(name)
{

  properties()["brief"] = std::string("Create space for SFDM shape function");
  properties()["description"] = std::string("The polynomial order \"P\" of the solution is configurable, default: P = 0");

}

/////////////////////////////////////////////////////////////////////////////

void CreateSFDFields::execute()
{
  /// @bug property("mesh").value<URI>() does not work for OptionComponent
  CMesh& mesh = access_component(property("mesh").value_str()).as_type<CMesh>();

  std::string solution_space = FlowSolver::Tags::solution();

  if ( is_null(mesh.get_child_ptr(FlowSolver::Tags::solution())) )
  {
    /// @todo get variable names etc, from Physics
    CFinfo <<  "  Creating field \"solution\", cellbased, with vars Q[1]" << CFendl;
    mesh.create_field(FlowSolver::Tags::solution(),CField::Basis::CELL_BASED,solution_space,"Q[1]");
  }

  if ( is_null(mesh.get_child_ptr(FlowSolver::Tags::residual())) )
  {
    CFinfo << "  Creating field \"residual\", cellbased" << CFendl;
    mesh.create_field(FlowSolver::Tags::residual(),CField::Basis::CELL_BASED,solution_space,"Res[1]");
  }

  if ( is_null(mesh.get_child_ptr(FlowSolver::Tags::wave_speed())) )
  {
    CFinfo << "  Creating field \"wave_speed\", cellbased" << CFendl;
    mesh.create_field(FlowSolver::Tags::wave_speed(),CField::Basis::CELL_BASED,"P0","wave_speed[1]");
  }

  if ( is_null(mesh.get_child_ptr(FlowSolver::Tags::update_coeff())) )
  {
    CFinfo << "  Creating field \"update_coeff\", cellbased" << CFendl;
    mesh.create_field(FlowSolver::Tags::update_coeff(),CField::Basis::CELL_BASED,"P0","update_coeff[1]");
  }

  if ( is_null(mesh.get_child_ptr("jacobian_determinant")) )
  {
    CFinfo << "  Creating field \"jacobian_determinant\", cell_based" << CFendl;
    CField& jacobian_determinant = mesh.create_field("jacobian_determinant",CField::Basis::CELL_BASED,solution_space,"jacobian_determinant[1]");

    CLoop& compute_jacobian_determinant = create_component< CForAllCells >("compute_jacobian_determinant");
    compute_jacobian_determinant.configure_property("regions", std::vector<URI>(1,mesh.topology().full_path()));
    compute_jacobian_determinant.create_loop_operation("CF.SFDM.ComputeJacobianDeterminant")
                                                .configure_property("jacobian_determinant",jacobian_determinant.full_path());
    compute_jacobian_determinant.execute();
    remove_component(compute_jacobian_determinant.name());
  }
}

//////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
