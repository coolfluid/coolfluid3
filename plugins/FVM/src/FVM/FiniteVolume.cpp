// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CField2.hpp"
#include "Mesh/CRegion.hpp"

#include "Solver/CIterativeSolver.hpp"
#include "Solver/Actions/CForAllFaces.hpp"

#include "FVM/FiniteVolume.hpp"
#include "FVM/ComputeFlux.hpp"

using namespace boost::assign;

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

Common::ComponentBuilder < FiniteVolume, CDiscretization, LibFVM > FiniteVolume_Builder;

////////////////////////////////////////////////////////////////////////////////

FiniteVolume::FiniteVolume ( const std::string& name  ) :
  CDiscretization ( name )
{
  // properties

  properties()["brief"] = std::string("Finite Volume Method");
  std::string description =
    "Discretize the PDE's using the Cell Centered Finite Volume Method\n"
    "This method is expected to fill the residual field, and the advection field.\n"
    " - The residual for cell[i] being F[i+1/2] - F[i-1/2],\n"
    "   F[i+1/2] is calculated using an approximate Riemann solver on the face\n"
    "   between cell[i] and cell[i+1]\n"
    " - The advection being the wavespeed \"a\" in the Courant number defined as:\n"
    "        CFL = a * dt / V ,\n"
    "   with V the volume of a cell (notice not length) and dt the timestep to take.";
  properties()["description"] = description; 

  // create apply boundary conditions action
  m_apply_bcs = create_static_component<CAction>("apply_boundary_conditions");
  m_apply_bcs->mark_basic();
  
  // create compute rhs action
  m_compute_rhs = create_static_component<CAction>("compute_rhs");
  m_compute_rhs->mark_basic();
  
  // set the compute rhs action
  CAction::Ptr for_all_faces = m_compute_rhs->create_component<CForAllFaces>("for_all_inner_faces");
  for_all_faces->create_component<ComputeFlux>("add_flux_to_rhs");
}

////////////////////////////////////////////////////////////////////////////////

FiniteVolume::~FiniteVolume()
{
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolume::compute_rhs()
{
  m_apply_bcs->execute();
  m_compute_rhs->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
