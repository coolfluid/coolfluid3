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
#include "common/ActionDirector.hpp"

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

common::ComponentBuilder < LUSGS, common::Action, LibLUSGS > LUSGS_Builder;

////////////////////////////////////////////////////////////////////////////////

LUSGS::LUSGS ( const std::string& name ) :
  common::Action(name),
  m_sweep_direction(FORWARD)
{
  options().add("print_iteration_summary",false).description("Print iteration summary").mark_basic();
  options().add("max_sweeps",math::Consts::uint_max()).description("Maximum number of sweeps").mark_basic();
  options().add("convergence_level",1e-12).description("Convergence level").mark_basic();
  options().add("recompute_lhs_frequency",1u).description("Recompute left-hand-side every x iterations").mark_basic();
  options().add("system",m_system).link_to(&m_system)
      .description("system to solve");

  options().add("dict",m_dict).link_to(&m_dict);
  options().add("pre_update",m_pre_update).link_to(&m_pre_update);
  options().add("post_update",m_post_update).link_to(&m_post_update);
}

///////////////////////////////////////////////////////////////////////////////////////

void LUSGS::compute_system_lhs()
{
  cf3_assert(m_system);

//  const Uint iteration = parent()->properties().value<Uint>("iteration");
//  const Uint recompute_lhs_frequency = options().value<Uint>("recompute_lhs_frequency");

//  if ( iteration % recompute_lhs_frequency == 0 || m_lu.size() == 0)
  {
    if (options().value<bool>("print_iteration_summary"))
      CFinfo << "    LUSGS:  Computing system left-hand-side" << CFendl;

    // Storage for the LU-decomposition
    m_lu.resize(m_dict->entities_range().size());

    // Compute Left Hand Side and do LU-decomposition
    for (Uint c=0; c<m_dict->entities_range().size(); ++c)
    {
      if ( m_system->loop_cells(m_dict->entities_range()[c]) )
      {
        RealMatrix lhs(m_system->nb_rows(),m_system->nb_cols());

        const Uint nb_elems = m_dict->entities_range()[c]->size();

        m_lu[c].resize(nb_elems);
        for (Uint e=0; e<nb_elems; ++e)
        {
          if (m_dict->entities_range()[c]->is_ghost(e) == false)
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
  CFdebug << "    LUSGS: start iteration" << CFendl;

  configure_option_recursively( "iterator", handle<Component>() );

  if ( is_null(m_system) ) throw SetupError(FromHere(), "system not configured");
  if ( is_null(m_dict) )   throw SetupError(FromHere(), "dict not configured");

//  link_fields();
  m_system->prepare();

  Real convergence = math::Consts::real_max();
  const Real convergence_level = options().value<Real>("convergence_level");
  const Uint max_sweeps = options().value<Uint>("max_sweeps");

  // Compute the left-hand-side and store it in m_lu to be solved easily during the sweeps
  compute_system_lhs();

  if (options().value<bool>("print_iteration_summary"))
    CFinfo << "    LUSGS:  Starting sweeps (maximum " << max_sweeps << ")" << CFendl;

  // Do the Symmetric Gauss-Seidel sweeps, alternately doing a forward and a backward sweeps
  Uint sweep;
  for (sweep=1; sweep <= max_sweeps && convergence > convergence_level; ++sweep)
  {
    if (m_pre_update) m_pre_update->execute();

    switch (m_sweep_direction)
    {
      case FORWARD:
        convergence = forward_sweep();  break;
      case BACKWARD:
        convergence = backward_sweep(); break;
    }

    // Parallel communication
    m_system->synchronize();

    PE::Comm::instance().all_reduce(PE::max(),&convergence,1,&convergence);

    // Apply post-update actions

    if (m_post_update) m_post_update->execute();

    // Print Information about every sweep
    if (options().value<bool>("print_iteration_summary"))
      CFinfo  << "    LUSGS:  sweep [" << std::setw(4) << sweep << "]  convergence ["<< std::scientific <<std::setw(12) << convergence <<"]" << CFendl;

    // Raise iteration finished
//    raise_iteration_done();

    // reverse sweep direction for next sweep
    if      (m_sweep_direction == FORWARD)    m_sweep_direction = BACKWARD;
    else if (m_sweep_direction == BACKWARD)   m_sweep_direction = FORWARD;

  } // End sweeps

  // Check if convergence was reached according to configured levels
  if (sweep >= max_sweeps && convergence > convergence_level)
  {
    CFinfo << "    LUSGS warning: convergence not reached after " << max_sweeps << " sweeps.    convergence ["<< std::scientific <<std::setw(12) << convergence <<"]" << CFendl;
  }
  else if ( is_zero(convergence) || is_nan(convergence) || is_inf(convergence) ) // The only time this happens, is when the residuals are nan or inf.
  {
    throw common::FailedToConverge(FromHere(), "LUSGS solver was not able to converge");
  }
}

////////////////////////////////////////////////////////////////////////////////

Real LUSGS::forward_sweep()
{
  CFdebug << "    LUSGS: forward sweep" << CFendl;
  Real convergence = 0.;
  Real cell_convergence;
  for (Uint c=0; c<m_dict->entities_range().size(); ++c)
  {
    if ( m_system->loop_cells(m_dict->entities_range()[c]) )
    {
      RealVector unknowns(m_system->nb_unknowns());
      RealVector rhs(m_system->nb_rows());

      // Element loop
      const Uint nb_elems = m_dict->entities_range()[c]->size();
      for (Uint e=0; e<nb_elems; ++e)
      {
        if (m_dict->entities_range()[c]->is_ghost(e) == false)
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

Real LUSGS::backward_sweep()
{
  CFdebug << "    LUSGS: backward sweep" << CFendl;
  Real convergence = 0.;
  Real cell_convergence;
  for (Uint c=m_dict->entities_range().size(); c-- > 0; )
  {
    if ( m_system->loop_cells(m_dict->entities_range()[c]) )
    {
      RealVector unknowns(m_system->nb_unknowns());
      RealVector rhs(m_system->nb_rows());

      const Uint nb_elems = m_dict->entities_range()[c]->size();
      for (Uint e=nb_elems ; e-- > 0 ; )
      {
        if (m_dict->entities_range()[c]->is_ghost(e) == false)
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
