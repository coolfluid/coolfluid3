// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/lineuler/BCSubsonicOutlet2D.hpp"
#include "solver/Solver.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<BCSubsonicOutlet2D,BC,LibLinEuler> BCSubsonicOutlet2d_builder;

/////////////////////////////////////////////////////////////////////////////

void BCSubsonicOutlet2D::execute()
{

  find_inner_cell(m_face_entities,m_face_elem_idx,cell_entities,cell_idx,cell_face_nb);
  set_inner_cell();
  cf3_assert(correct_non_reflective_term);
  correct_non_reflective_term->handle<NonReflectiveConvection2D>()->set_boundary_face(cell_face_nb);
  correct_non_reflective_term->execute();
  unset_inner_cell();
}

} // lineuler
} // sdm
} // cf3
