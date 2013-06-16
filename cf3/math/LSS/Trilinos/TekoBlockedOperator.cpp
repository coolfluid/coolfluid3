// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include "common/PE/Comm.hpp"
#include "common/PE/CommPattern.hpp"
#include "common/Log.hpp"

#include "math/VariablesDescriptor.hpp"


#include "math/LSS/Trilinos/TekoBlockedOperator.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

Teuchos::RCP<Teko::Epetra::BlockedEpetraOperator> create_teko_blocked_operator(TrilinosCrsMatrix& matrix, const math::VariablesDescriptor& vars)
{
  std::vector< std::vector<int> > var_gids;
  matrix.blocked_var_gids(vars, var_gids);
  return Teuchos::rcp(new Teko::Epetra::BlockedEpetraOperator(var_gids, matrix.epetra_matrix()));
}
  
} // namespace LSS
} // namespace math
} // namespace cf3
