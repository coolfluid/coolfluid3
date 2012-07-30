// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"

#include "math/Consts.hpp"
#include "math/Checks.hpp"
#include "math/VariablesDescriptor.hpp"

#include "solver/Solver.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/System.hpp"
#include "sdm/lusgs/LUSGS.hpp"
#include "sdm/DomainDiscretization.hpp"
#include "sdm/SDSolver.hpp"


using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::solver;
using namespace cf3::mesh;
using namespace cf3::math::Consts;
using namespace cf3::math::Checks;

namespace cf3 {
namespace sdm {
namespace lusgs {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LUSGS, IterativeSolver, LibLUSGS > LUSGS_Builder;

////////////////////////////////////////////////////////////////////////////////

LUSGS::LUSGS ( const std::string& name ) :
  IterativeSolver(name),
  m_sweep_direction(FORWARD)
{
  options().add("print_iteration_summary",false).description("Print iteration summary").mark_basic();
  options().add("max_sweeps",math::Consts::uint_max()).description("Maximum number of sweeps").mark_basic();
  options().add("convergence_level",1e-12).description("Convergence level").mark_basic();
  options().add("recompute_lhs_frequency",1u).description("Recompute left-hand-side every x iterations").mark_basic();
  options().add("system",std::string("cf3.sdm.implicit.BackwardEuler"))
      .description("system to solve")
      .attach_trigger( boost::bind( &LUSGS::configure_system, this) );
  configure_system();
}

///////////////////////////////////////////////////////////////////////////////////////

void LUSGS::configure_system()
{
  if (is_not_null(m_system))
  {
    if (m_system->derived_type_name() != options().value<std::string>("system"))
    {
      remove_component(*m_system);
    }
    else
    {
      return;
    }
  }
  m_system = create_component<sdm::System>("System",options().value<std::string>("system"));
}

///////////////////////////////////////////////////////////////////////////////////////

void LUSGS::compute_system_lhs()
{
  cf3_assert(m_system);

  const Uint iteration = parent()->properties().value<Uint>("iteration");
  const Uint recompute_lhs_frequency = options().value<Uint>("recompute_lhs_frequency");

  if ( iteration % recompute_lhs_frequency == 0 || m_lu.size() == 0)
  {
    if (options().value<bool>("print_iteration_summary"))
      CFinfo << "  LUSGS:  Computing system left-hand-side" << CFendl;

    // Cells to perform LUSGS to
    std::vector< Handle<Cells> > cell_elements = range_to_vector(find_components_recursively<Cells>(mesh()));

    // Storage for the LU-decomposition
    m_lu.resize(cell_elements.size());

    // Compute Left Hand Side and do LU-decomposition
    for (Uint c=0; c<cell_elements.size(); ++c)
    {
      if ( m_system->loop_cells(cell_elements[c]) )
      {
        RealMatrix lhs(m_system->nb_rows(),m_system->nb_cols());

        const Uint nb_elems = cell_elements[c]->size();

        m_lu[c].resize(nb_elems);
        for (Uint e=0; e<nb_elems; ++e)
        {
          if (cell_elements[c]->is_ghost(e) == false)
          {
            m_system->compute_lhs(e,lhs);
            m_lu[c][e].compute(lhs);
          }
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////

void LUSGS::execute()
{
  CFdebug << "LUSGS: start iteration" << CFendl;

  configure_option_recursively( "iterator", handle<Component>() );

  link_fields();
  m_system->prepare();

  Real convergence = math::Consts::real_max();
  const Real convergence_level = options().value<Real>("convergence_level");
  const Uint max_sweeps = options().value<Uint>("max_sweeps");

  // Cells to perform LUSGS to
  std::vector< Handle<Cells> > cell_elements = range_to_vector(find_components_recursively<Cells>(mesh()));

  // Compute the left-hand-side and store it in m_lu to be solved easily during the sweeps
  compute_system_lhs();

  if (options().value<bool>("print_iteration_summary"))
    CFinfo << "  LUSGS:  Starting sweeps (maximum " << max_sweeps << ")" << CFendl;

  // Do the Symmetric Gauss-Seidel sweeps, alternately doing a forward and a backward sweeps
  Uint sweep;
  for (sweep=1; sweep <= max_sweeps && convergence > convergence_level; ++sweep)
  {
    pre_update().execute();

    switch (m_sweep_direction)
    {
      case FORWARD:
        convergence = forward_sweep(cell_elements);  break;
      case BACKWARD:
        convergence = backward_sweep(cell_elements); break;
    }

    // Parallel communication
    m_solution->synchronize();
    PE::Comm::instance().all_reduce(PE::max(),&convergence,1,&convergence);

    // Apply post-update actions
    post_update().execute();

    // Print Information about every sweep
    if (options().value<bool>("print_iteration_summary"))
      CFinfo  << "  LUSGS:  sweep [" << std::setw(4) << sweep << "]  convergence ["<< std::scientific <<std::setw(12) << convergence <<"]" << CFendl;

    // Raise iteration finished
    raise_iteration_done();

    // reverse sweep direction for next sweep
    if      (m_sweep_direction == FORWARD)    m_sweep_direction = BACKWARD;
    else if (m_sweep_direction == BACKWARD)   m_sweep_direction = FORWARD;

  } // End sweeps

  // Check if convergence was reached according to configured levels
  if (sweep >= max_sweeps && convergence > convergence_level)
  {
    CFinfo << "  LUSGS warning: convergence not reached after " << max_sweeps << " sweeps.    convergence ["<< std::scientific <<std::setw(12) << convergence <<"]" << CFendl;
  }
  else if ( is_zero(convergence) || is_nan(convergence) || is_inf(convergence) ) // The only time this happens, is when the residuals are nan or inf.
  {
    throw common::FailedToConverge(FromHere(), "LUSGS solver was not able to converge");
  }
}

////////////////////////////////////////////////////////////////////////////////

Real LUSGS::forward_sweep(std::vector< Handle<Cells> >& cell_elements)
{
  CFdebug << "  LUSGS: forward sweep" << CFendl;
  Real convergence = 0.;
  Real cell_convergence;
  for (Uint c=0; c<cell_elements.size(); ++c)
  {
    if ( m_system->loop_cells(cell_elements[c]) )
    {
      RealVector unknowns(m_system->nb_unknowns());
      RealVector rhs(m_system->nb_rows());

      // Element loop
      const Uint nb_elems = cell_elements[c]->size();
      for (Uint e=0; e<nb_elems; ++e)
      {
        if (cell_elements[c]->is_ghost(e) == false)
        {
          // Compute RHS for this cell
          m_system->compute_rhs(e,rhs);

          // Solve LU
          unknowns = m_lu[c][e].solve(rhs);

          // Update unknowns
          cell_convergence = m_system->update(e,unknowns);

          // Update the convergence
          convergence = std::max(convergence,cell_convergence);
        }
      }
    }
  }
  return convergence;
}

////////////////////////////////////////////////////////////////////////////////

Real LUSGS::backward_sweep(std::vector< Handle<Cells> >& cell_elements)
{
  CFdebug << "  LUSGS: backward sweep" << CFendl;
  Real convergence = 0.;
  Real cell_convergence;
  for (Uint c=cell_elements.size(); c-- > 0; )
  {
    if ( m_system->loop_cells(cell_elements[c]) )
    {
      RealVector unknowns(m_system->nb_unknowns());
      RealVector rhs(m_system->nb_rows());

      const Uint nb_elems = cell_elements[c]->size();
      for (Uint e=nb_elems ; e-- > 0 ; )
      {
        if (cell_elements[c]->is_ghost(e) == false)
        {
          // Compute RHS for this cell
          m_system->compute_rhs(e,rhs);

          // Solve LU
          unknowns = m_lu[c][e].solve(rhs);

          // Update unknowns
          cell_convergence = m_system->update(e,unknowns);

          // Update the convergence
          convergence = std::max(convergence,cell_convergence);
        }
      }
    }
  }
  return convergence;
}

////////////////////////////////////////////////////////////////////////////////

} // lusgs
} // sdm
} // cf3
