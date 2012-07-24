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

#include "solver/Solver.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/lusgs/LUSGS.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::solver;
using namespace cf3::mesh;
using namespace cf3::math::Consts;

namespace cf3 {
namespace sdm {
namespace lusgs {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LUSGS, IterativeSolver, LibLUSGS > LUSGS_Builder;

////////////////////////////////////////////////////////////////////////////////

LUSGS::LUSGS ( const std::string& name ) :
  IterativeSolver(name)
{
  options().add("max_sweeps",math::Consts::uint_max()).description("Maximum number of sweeps").mark_basic();
  options().add("convergence_level",1.0e-6).description("Convergence level").mark_basic();
  options().add("recompute_lhs_frequency",1u).description("Recompute left-hand-side every x iterations").mark_basic();
  options().add("system",std::string("cf3.sdm.BackwardEuler")).description("system to solve")
      .attach_trigger( boost::bind( &LUSGS::configure_system, this) );
  configure_system();
}

///////////////////////////////////////////////////////////////////////////////////////

void LUSGS::configure_system()
{
  if (is_not_null(m_system))
    remove_component(*m_system);

  m_system = create_component<System>("system",options().value<std::string>("system"));
}

///////////////////////////////////////////////////////////////////////////////////////

void LUSGS::compute_system_lhs()
{
  const Uint iteration = parent()->properties().value<Uint>("iteration");
  const Uint recompute_lhs_frequency = options().value<Uint>("recompute_lhs_frequency");

  if ( iteration % recompute_lhs_frequency == 0 || m_lu.size() == 0)
  {
    // Cells to perform LUSGS to
    std::vector< Handle<Cells> > cell_elements = range_to_vector(find_components_recursively<Cells>(mesh()));

    // Storage for the LU-decomposition
    m_lu.resize(cell_elements.size());

    // Compute Left Hand Side and do LU-decomposition
    for (Uint c=0; c<cell_elements.size(); ++c)
    {
      if (m_system->loop_cells(*cell_elements[c]))
      {
        const Handle<Space const>& space = m_solution->space(cell_elements[c]);

        // matrix to compute left hand side in (nb_sol_pts x nb_vars) x (nb_sol_pts x nb_vars)
        const Uint nb_sol_pts = space->shape_function().nb_nodes();
        const Uint nb_vars = m_solution->row_size();
        RealMatrix lhs(nb_sol_pts*nb_vars,nb_sol_pts*nb_vars);

        const Uint nb_elems = cell_elements[c]->size();

        m_lu[c].resize(nb_elems);
        for (Uint e=0; e<nb_elems; ++e)
        {
          m_system->compute_lhs(e,lhs);
          m_lu[c][e].compute(lhs);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void LUSGS::execute()
{
  configure_option_recursively( "iterator", handle<Component>() );
  
  link_fields();

  Field& Q = *m_solution;

  Real convergence = math::Consts::real_max();
  Real convergence_level = options().value<Real>("convergence_level");
  Real max_sweeps = options().value<Real>("max_sweeps");

  // Cells to perform LUSGS to
  std::vector< Handle<Cells> > cell_elements = range_to_vector(find_components_recursively<Cells>(mesh()));

  // Compute the left-hand-side and store it in m_lu to be solved easily during the sweeps
  compute_system_lhs();

  // Do the Symmetric Gauss-Seidel sweeps, consisting of a forward and a backward sweeps
  Uint sweep;
  for (sweep=0; sweep <= max_sweeps || convergence > convergence_level; ++sweep)
  {
    // Reset convergence to zero. It will be computed during the backward sweep
    convergence=0.;

    // Do a forward sweep
    for (Uint c=0; c<cell_elements.size(); ++c)
    {
      if (m_system->loop_cells(*cell_elements[c]))
      {
        const Handle<Space const>& space = Q.space(cell_elements[c]);
        const Uint nb_vars = Q.row_size();
        const Uint nb_sol_pts = space->shape_function().nb_nodes();
        RealMatrix dQ(nb_sol_pts,nb_vars);  // nb_sol_pts x nb_vars
        RealMatrix rhs(nb_sol_pts,nb_vars); // nb_sol_pts x nb_vars

        const Uint nb_elems = cell_elements[c]->size();
        for (Uint e=0; e<nb_elems; ++e)
        {
          // Compute RHS for this cell
          m_system->compute_rhs(e,rhs);

          // Solve LU
          dQ = m_lu[c][e].solve(rhs);

          // Update solution register, and compute convergence
          for (Uint s=0; s<nb_sol_pts; ++s)
          {
            const Uint p = space->connectivity()[e][s];
            for (Uint v=0; v<nb_vars; ++v)
            {
              Q[p][v] += dQ(s,v);
            }
          }
        }
      }
    }
    Q.synchronize();

    // Do a backward sweep (same as forward sweep, except traverse cells backwards)
    for (Uint c=cell_elements.size(); c-- > 0; )
    {
      if (m_system->loop_cells(*cell_elements[c]))
      {
        const Handle<Space const>& space = Q.space(cell_elements[c]);
        const Uint nb_vars = Q.row_size();
        const Uint nb_sol_pts = space->shape_function().nb_nodes();
        RealMatrix dQ(nb_sol_pts,nb_vars);  // nb_sol_pts x nb_vars
        RealMatrix rhs(nb_sol_pts,nb_vars); // nb_sol_pts x nb_vars

        const Uint nb_elems = cell_elements[c]->size();
        for (Uint e=nb_elems ; e-- > 0 ; )
        {
          // Compute RHS for this cell
          m_system->compute_rhs(e,rhs);

          // Solve LU
          dQ = m_lu[c][e].solve(rhs);

          // Update solution register, and compute convergence
          for (Uint s=0; s<nb_sol_pts; ++s)
          {
            const Uint p = space->connectivity()[e][s];
            for (Uint v=0; v<nb_vars; ++v)
            {
              Q[p][v] += dQ(s,v);

              convergence = std::max( convergence, std::abs(dQ(s,v)/(eps()+Q[p][v])) );
            }
          }
        }
      }
    }
    Q.synchronize();

    raise_iteration_done();
  }
}

////////////////////////////////////////////////////////////////////////////////

} // lusgs
} // sdm
} // cf3
