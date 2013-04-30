// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"

#include "mesh/Cells.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ShapeFunction.hpp"
#include "mesh/Dictionary.hpp"

#include "solver/ComputeRHS.hpp"
#include "solver/Term.hpp"
#include "solver/TermComputer.hpp"
#include "solver/PDE.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;

namespace cf3 {
namespace solver {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeRHS, common::Action, solver::LibSolver > ComputeRHS_Builder;

////////////////////////////////////////////////////////////////////////////////

ComputeRHS::ComputeRHS ( const std::string& name ) : common::Action(name)
{
  options().add("rhs",m_rhs).link_to(&m_rhs)
      .description("Right-Hand-Side of equations")
      .mark_basic();
  options().add("wave_speed",m_ws).link_to(&m_ws)
      .description("Wave speed")
      .mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRHS::execute()
{
  if ( is_null(m_rhs) ) SetupError(FromHere(), "rhs not configured");
  if ( is_null(m_ws) ) SetupError(FromHere(), "wave_speed not configured");
  compute_rhs(*m_rhs,*m_ws);
}

////////////////////////////////////////////////////////////////////////////////

bool ComputeRHS::loop_cells(const Handle<mesh::Entities const>& cells)
{
  m_term_computers.clear();
  boost_foreach( TermComputer& term_computer, find_components<TermComputer>(*this))
  {
    m_term_computers.push_back(term_computer.handle<TermComputer>());
  }
  m_loop_cells.resize(m_term_computers.size());

  bool loop_any = true;
  for (Uint t=0; t<m_term_computers.size(); ++t)
  {
    m_loop_cells[t] = m_term_computers[t]->loop_cells(cells);
    loop_any &= m_loop_cells[t];
  }
  return loop_any;
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRHS::compute_rhs(const Uint elem_idx, std::vector<RealVector>& rhs, std::vector<Real>& wave_speed)
{
  boost_foreach( RealVector& rhs_in_pt, rhs)
  {
    rhs_in_pt.setZero();
  }
  boost_foreach( Real& wave_speed_in_pt, wave_speed)
  {
    wave_speed_in_pt = 0.;
  }

  for (Uint t=0; t<m_term_computers.size(); ++t)
  {
    if (m_loop_cells[t])
    {
      m_term_computers[t]->compute_term(elem_idx,m_tmp_term,m_tmp_ws);
      for (Uint p=0; p<rhs.size(); ++p)
      {
        for (Uint eq=0; eq<rhs[p].size(); ++eq)
        {
          rhs[p][eq] += m_tmp_term[p][eq];
        }
        wave_speed[p] = std::max(wave_speed[p],m_tmp_ws[p]);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRHS::compute_rhs(mesh::Field& rhs, mesh::Field& wave_speed)
{
  const Uint nb_eqs = rhs.row_size();
  mesh::Dictionary& dict = rhs.dict();
  boost_foreach(const Handle<mesh::Entities>& cells, dict.entities_range() )
  {
    if ( loop_cells(cells) )
    {
      const Space& space = dict.space(*cells);

      // Element-loop
      const Uint nb_elems = cells->size();
      const Uint nb_sol_pts = space.shape_function().nb_nodes();

      std::vector<RealVector> elem_rhs(nb_sol_pts,RealVector(nb_eqs));
      std::vector<Real> elem_wave_speed(nb_sol_pts);

      for (Uint elem_idx=0; elem_idx<nb_elems; ++elem_idx)
      {
        if (cells->is_ghost(elem_idx)==false)
        {
          compute_rhs(elem_idx,elem_rhs,elem_wave_speed);

          mesh::Connectivity::ConstRow nodes = space.connectivity()[elem_idx];
          for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
          {
            for (Uint eq=0; eq<nb_eqs; ++eq)
            {
              rhs[nodes[sol_pt]][eq] = elem_rhs[sol_pt][eq];
            }
            wave_speed[nodes[sol_pt]][0] = elem_wave_speed[sol_pt];
          }
        }
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
