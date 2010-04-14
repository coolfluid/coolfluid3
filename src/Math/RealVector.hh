#ifndef COOLFluiD_RealVector_hh
#define COOLFluiD_RealVector_hh

//////////////////////////////////////////////////////////////////////////////

#include "MathTools/MathTools.hh"
#include "MathTools/CFVector.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

//////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_CXX_EXPLICIT_TEMPLATES
  // explicit template instantiation
  MathTools_TEMPLATE template class MathTools_API MathTools::CFVector<CFreal>;
  MathTools_TEMPLATE template class MathTools_API MathTools::CFSliceVector<CFreal>;
#endif

/// RealVector is a CFVector templatized with a CFreal
typedef MathTools::CFVector<CFreal> RealVector;

/// RealSliceVector is a CFSliceVector templatized with a CFreal
typedef MathTools::CFSliceVector<CFreal> RealSliceVector;

//////////////////////////////////////////////////////////////////////////////

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_RealVector_hh
