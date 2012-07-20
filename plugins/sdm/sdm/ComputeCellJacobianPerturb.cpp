// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"
#include "common/Action.hpp"
#include "common/DynTable.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/Time.hpp"
#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "sdm/Tags.hpp"
#include "sdm/ComputeCellJacobianPerturb.hpp"
#include "math/Consts.hpp"

#include "sdm/DomainDiscretization.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeCellJacobianPerturb, common::Action, LibSDM > ComputeCellJacobianPerturb_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeCellJacobianPerturb::ComputeCellJacobianPerturb ( const std::string& name ) :
  common::Action(name)
{
  options().add("residual", m_residual )
    .description("Residual field")
    .pretty_name("Residual")
    .link_to(&m_residual);

  options().add("solution", m_solution)
    .description("Solution field")
    .pretty_name("Solution")
    .link_to(&m_solution)
    .attach_trigger( boost::bind( &ComputeCellJacobianPerturb::create_backup_fields , this) );

  options().add("reference_solution", std::vector<Real>())
    .description("Reference solution, required for computation of denominator")
    .link_to(&m_ref_sol);

  options().add("cell_jacobian", m_cell_jacobian )
    .description("Cell Jacobian")
    .pretty_name("Cell Jacobian")
    .link_to(&m_cell_jacobian);

  options().add("domain_discretization", m_domain_discretization )
    .link_to(&m_domain_discretization);

  m_eps = sqrt(math::Consts::eps());
}

////////////////////////////////////////////////////////////////////////////////

void ComputeCellJacobianPerturb::create_backup_fields()
{
  if ( is_null(m_solution) )
  {
    throw SetupError(FromHere() , "'solution' needs to be configured or doesn't exist");
  }

  if (is_null(m_residual_backup))
  {
    m_residual_backup = m_solution->dict().create_field("residual_backup", m_solution->descriptor().description()).handle<Field>();
  }

  if (is_null(m_solution_backup))
  {
    m_solution_backup = m_solution->dict().create_field("solution_backup", m_solution->descriptor().description()).handle<Field>();
  }

}

////////////////////////////////////////////////////////////////////////////////

void ComputeCellJacobianPerturb::execute()
{
  if (is_null(m_solution))      throw SetupError(FromHere(),"'solution' needs to be configured");
  if (is_null(m_residual))      throw SetupError(FromHere(),"'residual' needs to be configured");
  if (is_null(m_cell_jacobian)) throw SetupError(FromHere(),"'cell_jacobian' needs to be configured");

  Field& Q  = *m_solution;
  Field& R  = *m_residual;
  Field& Q0 = *m_solution_backup;
  Field& R0 = *m_residual_backup;
  DynTable<Real>& J  = *m_cell_jacobian;

  const Uint nb_vars = Q.row_size();
  const Uint nb_pts = Q.size();

  // Perturb solution and store in m_solution, put backup in m_solution_backup
  for (Uint p=0; p<nb_pts; ++p)
  {
    for (Uint v=0; v<nb_vars; ++v)
    {
      Q0[p][v] = Q[p][v];

      Q[p][v] *= (1. + m_eps);

      R0[p][v] = R[p][v];
    }
  }

  m_domain_discretization->update();

  boost_foreach( const Handle<Space>& space, m_solution->spaces() )
  {
    if ( m_domain_discretization->loop_cells(space->support().handle<Cells>()) )
    {
      const Uint nb_elems = space->size();
      const Uint nb_pts_per_elem = space->shape_function().nb_nodes();
      const Uint nb_jacobian_matrix_elements = nb_pts_per_elem*nb_vars;

      // for every element in this space
      for (Uint e=0; e<nb_elems; ++e)
      {

        // perturb this element
        for (Uint s=0; s<nb_pts_per_elem; ++s)
        {
          Uint p = space->connectivity()[e][s];

          for (Uint v=0; v<nb_vars; ++v)
          {
            Q0[p][v] = Q[p][v];

            Q[p][v] *= (1. + m_eps);

            R0[p][v] = R[p][v];
          }
        }

        // Compute the perturbed residual only for this element
        m_domain_discretization->compute_element(e);

        // Compute the cell jacobian for this element
        for (Uint s=0; s<nb_pts_per_elem; ++s)
        {
          Uint p = space->connectivity()[e][s];
          J.set_row_size(p,nb_jacobian_matrix_elements);

          // contribution of other points of this element to this point
          for (Uint ds=0; ds<nb_pts_per_elem; ++ds)
          {
            Uint dp = space->connectivity()[e][ds];

            for (Uint v=0; v<nb_vars; ++v)
            {
              J[p][nb_pts*ds+v] = ( R[p][v] - R0[p][v] ) / ( m_eps*Q0[dp][v] - m_ref_sol[v] );
            }
          }
        }

        // Restore element from backup
        for (Uint s=0; s<nb_pts_per_elem; ++s)
        {
          Uint p = space->connectivity()[e][s];

          for (Uint p=0; p<nb_pts; ++p)
          {
            for (Uint v=0; v<nb_vars; ++v)
            {
              Q[p][v] = Q0[p][v];
              R[p][v] = R0[p][v];
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////////

