// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"

#include "math/Consts.hpp"
#include "math/VariablesDescriptor.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/System.hpp"
#include "sdm/DomainDiscretization.hpp"
#include "sdm/ComputeCellJacobianPerturb.hpp"
#include "sdm/implicit/BackwardEuler.hpp"

#include "solver/Solver.hpp"


using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace sdm {
namespace implicit {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BackwardEuler, System, LibImplicit > BackwardEuler_Builder;

////////////////////////////////////////////////////////////////////////////////

BackwardEuler::BackwardEuler ( const std::string& name ) :
  System(name)
{
  CFdebug << "Creating BackwardEuler" << CFendl;

  options().add("domain_discretization",m_domain_discretization).link_to(&m_domain_discretization);
  options().add("solution",m_solution).link_to(&m_solution).attach_trigger( boost::bind( &BackwardEuler::create_solution_backup , this) );
  options().add("residual",m_residual).link_to(&m_residual);
  options().add("update_coefficient", m_update_coeff)
    .description("Update coefficient to multiply with residual")
    .pretty_name("Update Coefficient")
    .link_to(&m_update_coeff);

  m_compute_jacobian = create_static_component<ComputeCellJacobianPerturb>("ComputeCellJacobian");

  CFdebug << "Created BackwardEuler" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void BackwardEuler::create_solution_backup()
{
  if ( is_null(m_solution) )              throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());

  if ( is_null(m_solution->dict().get_child("solution_backup")) )
  {
    m_solution_backup = m_solution->dict().create_field("solution_backup",m_solution->descriptor().description()).handle<Field>();
  }
  else
  {
    m_solution_backup = m_solution->dict().get_child("solution_backup")->handle<Field>();
  }
}


////////////////////////////////////////////////////////////////////////////////

void BackwardEuler::prepare()
{
  // check configuration
  if ( is_null(m_solution) )              throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());

  // backup
  cf3_assert(m_solution_backup);
  *m_solution_backup = *m_solution;

  // compute residual of backup, plus wave-speeds! --> used to compute dt
  m_domain_discretization->execute();
}

////////////////////////////////////////////////////////////////////////////////

bool BackwardEuler::loop_cells(const Handle<mesh::Cells const>& cells)
{
  if ( is_null(m_domain_discretization) ) throw SetupError( FromHere(), "Option 'domain_discretization' not configured for "+uri().string());
  if ( is_null(m_solution) )              throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());
  if ( is_null(m_residual) )              throw SetupError( FromHere(), "Option 'residual' not configured for "+uri().string());

  // configure m_compute_jacobian
  m_compute_jacobian->options().set("domain_discretization",m_domain_discretization);
  m_compute_jacobian->options().set("solution",m_solution);
  m_compute_jacobian->options().set("residual",m_residual);

  if (m_compute_jacobian->loop_cells(cells) == false)
    return false;

  m_space = m_solution->space(cells);
  cf3_assert(m_space);

  m_nb_vars = m_solution->row_size();
  m_nb_sol_pts = m_space->shape_function().nb_nodes();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void BackwardEuler::compute_lhs(const Uint elem, RealMatrix& lhs)
{
  /// lhs = -dR/dQ + I/dt

  cf3_assert(lhs.rows() == m_nb_sol_pts*m_nb_vars);
  cf3_assert(lhs.cols() == m_nb_sol_pts*m_nb_vars);

  // store dR/dQ in lhs
  m_compute_jacobian->compute_jacobian(elem,lhs);

  // flip sign of lhs
  lhs = -lhs;

  // add I/dt
  const Field& H = *m_update_coeff;
  for (Uint s=0; s<m_nb_sol_pts; ++s)
  {
    const Uint p = m_space->connectivity()[elem][s];
    const Real inv_dt = 1./H[p][0];
    for (Uint v=0; v<m_nb_vars; ++v)
    {
      lhs(s*m_nb_vars+v,s*m_nb_vars+v) += inv_dt;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void BackwardEuler::compute_rhs(const Uint elem, RealVector &rhs)
{
  const Field& Q  = *m_solution;
  const Field& Q0 = *m_solution_backup;
  const Field& H  = *m_update_coeff;
  Field&       R  = *m_residual;

  for (Uint s=0; s<m_nb_sol_pts; ++s)
  {
    const Uint p = m_space->connectivity()[elem][s];
    for (Uint v=0; v<m_nb_vars; ++v)
    {
      R[p][v] = 0.;
    }
  }
  m_domain_discretization->compute_element(elem);

  for (Uint s=0; s<m_nb_sol_pts; ++s)
  {
    const Uint p = m_space->connectivity()[elem][s];
    const Real& dt = H[p][0];
    for (Uint v=0; v<m_nb_vars; ++v)
    {
      rhs[s*m_nb_vars+v] = R[p][v] - (Q[p][v] - Q0[p][v])/dt;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // implicit
} // sdm
} // cf3
