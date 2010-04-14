#ifndef COOLFluiD_MathTools_ZeroDeterminantException_hh
#define COOLFluiD_MathTools_ZeroDeterminantException_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hh"
#include "MathTools/MathTools.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

/// This class represents an Exception thrown when
/// a Zero determinant matrix is found
/// @author Andrea Lani
/// @author Tiago Quintino
class MathTools_API ZeroDeterminantException : public Common::Exception {
public:

  /// Constructor
  ZeroDeterminantException ( const Common::CodeLocation& where, const std::string& what);

}; // end of class ZeroDeterminantException

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_ZeroDeterminantException_hh
