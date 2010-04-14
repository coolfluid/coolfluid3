#include "MathTools/ZeroDeterminantException.hh"

namespace COOLFluiD {
namespace MathTools {

ZeroDeterminantException::ZeroDeterminantException ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "ZeroDeterminantException")
{}

} // namespace MathTools
} // namespace COOLFluiD
