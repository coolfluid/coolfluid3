#include "Math/InverterT.hpp"
#include "Math/LUInverter.hpp"
#include "Math/SVDInverter.hpp"
#include "Math/MatrixInverterT.hpp"
#include "Math/InverterDiag.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

MatrixInverter*
MatrixInverter::create(const Uint& size, const bool& isDiagonal)
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
////////////////////////////////////////////////////////////////////////////////

MatrixInverter*
MatrixInverter::create(const Uint& nbRows, const Uint& nbCols, const bool& isDiagonal)
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

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////
