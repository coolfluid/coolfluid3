#include "MathTools/InverterT.hh"
#include "MathTools/LUInverter.hh"
#include "MathTools/SVDInverter.hh"
#include "MathTools/MatrixInverterT.hh"
#include "MathTools/InverterDiag.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

MatrixInverter*
MatrixInverter::create(const CFuint& size, const bool& isDiagonal)
{
  if (!isDiagonal)
  {
    switch(size)
    {
    case(4):
      return new InverterT<4>();
      break;
    case(3):
      return new InverterT<3>();
      break;
    case(2):
      return new InverterT<2>();
      break;
    default:
      return new LUInverter(size);
      break;
    }
  }
  else
  {
    return new InverterDiag();
  }

  // last resort, but it should not reach here
  return new LUInverter(size);
}
//////////////////////////////////////////////////////////////////////////////

MatrixInverter*
MatrixInverter::create(const CFuint& nbRows, const CFuint& nbCols, const bool& isDiagonal)
{
  if (nbRows == nbCols)
  {
    return create(nbRows,isDiagonal);
  }
  else
  {
    return new SVDInverter(nbRows,nbCols);
  }

  // last resort, but it should not reach here
  return new SVDInverter(nbRows,nbCols);
}

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////
