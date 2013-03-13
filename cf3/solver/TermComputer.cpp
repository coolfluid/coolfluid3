// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "solver/TermComputer.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
  
/////////////////////////////////////////////////////////////////////////////////////

TermComputer::TermComputer ( const std::string& name ) 
  : common::Action(name) 
{
  options().add("field",m_term_field).link_to(&m_term_field)
    .description("Term that will be computed")
    .mark_basic();
  options().add("term_wave_speed_field",m_term_ws).link_to(&m_term_ws)
    .description("Term wave speed that will be computed")
    .mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

void TermComputer::execute()
{
  if ( is_null(m_term_field) )  throw common::SetupError( FromHere(), "term not configured" );
  if ( is_null(m_term_ws) )     throw common::SetupError( FromHere(), "term_wave_speed not configured" );
  
  compute_term(*m_term_field,*m_term_ws);
}

/////////////////////////////////////////////////////////////////////////////////////

void TermComputer::compute_term(mesh::Field& term, mesh::Field& wave_speed)
{
  term = 0.;
  boost_foreach( const Handle<mesh::Entities const>& cells, term.entities_range() )
  {
    if (loop_cells(cells))
    {
      const mesh::Space& space = term.space(*cells);
      const Uint nb_elems = space.size();
      const Uint nb_nodes_per_elem = space.shape_function().nb_nodes();
      for (Uint e=0; e<nb_elems; ++e)
      {
        compute_term(e,m_tmp_term,m_tmp_ws);
        for (Uint s=0; s<nb_nodes_per_elem; ++s)
        {
          const Uint p=space.connectivity()[e][s];
          for (Uint eq=0; eq<m_tmp_term[s].size(); ++eq)
          {
            term[p][eq] += m_tmp_term[s][eq];
          }
          wave_speed[p][0] = m_tmp_ws[s]; 
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
