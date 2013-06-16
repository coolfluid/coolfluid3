// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_TekoBlockedOperator_hpp
#define cf3_Math_LSS_TekoBlockedOperator_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include "Teuchos_RCP.hpp"
#include "Teko_BlockedEpetraOperator.hpp"

#include "common/CF.hpp"

#include "math/VariablesDescriptor.hpp"

#include "TrilinosCrsMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TekoBlockedOperator.hpp Build a blocked operator, based on the VariablesDescriptor and a CrsMatrix
  @author Bart Janssens

**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

Teuchos::RCP<Teko::Epetra::BlockedEpetraOperator> create_teko_blocked_operator(TrilinosCrsMatrix& matrix, const math::VariablesDescriptor& vars);

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_TekoBlockedOperator_hpp
