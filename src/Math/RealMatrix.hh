#ifndef COOLFluiD_RealMatrix_hh
#define COOLFluiD_RealMatrix_hh

//////////////////////////////////////////////////////////////////////////////

#include "MathTools/CFMatrix.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

//////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_CXX_EXPLICIT_TEMPLATES
  // explicit template instantiation
  MathTools_TEMPLATE template class MathTools_API MathTools::CFMatrix<CFreal>;
  MathTools_TEMPLATE template class MathTools_API MathTools::CFSliceMatrix<CFreal>;
#endif

/// RealMatrix is a CFMatrix templatized with a CFreal
typedef MathTools::CFMatrix<CFreal> RealMatrix;

/// RealSliceMatrix is a CFSliceMatrix templatized with a CFreal
typedef MathTools::CFSliceMatrix<CFreal> RealSliceMatrix;

//////////////////////////////////////////////////////////////////////////////

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_RealMatrix_hh
