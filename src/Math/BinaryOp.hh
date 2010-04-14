#ifndef COOLFluiD_MathTools_BinaryOp_hh
#define COOLFluiD_MathTools_BinaryOp_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/COOLFluiD.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {
namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

/// Provides some functors applying binary functions.
/// @author Andrea Lani
#define MYBINARYOP(__name__,__op__) \
template <class T> \
class __name__ {\
public: \
static void op(T& a, const T& b) {a __op__ b;} \
};

MYBINARYOP(EqualBin,=)
MYBINARYOP(AddBin,+=)
MYBINARYOP(SubBin,-=)
MYBINARYOP(MultBin,*=)
MYBINARYOP(DivBin,/=)

#undef MYBINARYOP

//////////////////////////////////////////////////////////////////////////////

} // namespace MathTools
} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_BinaryOp_hh
