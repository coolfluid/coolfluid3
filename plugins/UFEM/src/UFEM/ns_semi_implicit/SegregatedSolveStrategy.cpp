// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "SegregatedSolveStrategy.hpp"

namespace cf3 {
namespace UFEM {

SegregatedSolveStrategy::SegregatedSolveStrategy ( const std::string& name ) : SolutionStrategy(name)
{
}

SegregatedSolveStrategy::~SegregatedSolveStrategy()
{
}


void SegregatedSolveStrategy::solve()
{

}

void SegregatedSolveStrategy::set_matrix ( const Handle< math::LSS::Matrix >& matrix )
{
  Handle<math::LSS::TrilinosCrsMatrix> full_matrix(matrix);
  if(is_null(full_matrix))
    throw common::SetupError(FromHere(), "Error applying SegregatedSolveStrategy: matrix " + matrix->uri().string() + " is not a math::LSS::TrilinosCrsMatrix");

  const Teuchos::RCP<Epetra_CrsMatrix const> crs_matrix = full_matrix->epetra_matrix();

  if(is_null(m_variables_descriptor))
    throw common::SetupError(FromHere(), "Option variables_descriptor is not set for " + uri().string());
  
  const math::VariablesDescriptor& descriptor = *m_variables_descriptor;

  // Find out what the locations of the pressure and velocity variables are
  const Uint p_idx = descriptor.var_number("Pressure");
  const Uint u_idx = descriptor.var_number("Velocity");
  cf3_assert(p_idx == 0 || p_idx == 1);
  cf3_assert(u_idx == 0 || u_idx == 1);
  cf3_assert(descriptor.nb_vars() == 2);
  cf3_assert(descriptor.offset(u_idx) == u_idx);

  // Get the different blocks from the full matrix
  std::vector< std::vector<int> > vars;
  full_matrix->blocked_var_gids(descriptor, vars);
  m_blocked_mapping = Teuchos::rcp(new Teko::Epetra::BlockedMappingStrategy(vars, Teuchos::rcpFromRef(crs_matrix->OperatorDomainMap()), crs_matrix->Comm()));
  Teko::Epetra::BlockedMappingStrategy& blocked_mapping = *m_blocked_mapping;

  m_blocked_thyra_op = blocked_mapping.buildBlockedThyraOp(crs_matrix);
  m_Auu = Teko::getBlock(u_idx, u_idx, m_blocked_thyra_op);
  m_Aup = Teko::getBlock(u_idx, p_idx, m_blocked_thyra_op);
  m_Apu = Teko::getBlock(p_idx, u_idx, m_blocked_thyra_op);
  m_App = Teko::getBlock(p_idx, p_idx, m_blocked_thyra_op);

}

void SegregatedSolveStrategy::set_rhs ( const Handle< math::LSS::Vector >& rhs )
{

}

void SegregatedSolveStrategy::set_solution ( const Handle< math::LSS::Vector >& solution )
{

}

Real SegregatedSolveStrategy::compute_residual()
{
  return 0.;
}


  
} // UFEM
} // cf3
