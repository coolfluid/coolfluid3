#ifndef CF_Mesh_P1_Triag2D_HH
#define CF_Mesh_P1_Triag2D_HH

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Triag2D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Triag2D> 
{
public:
  static Real computeVolume(const std::vector<RealVector*>& coord); 
};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
class P1_API Triag2D : public ElementType
{
public:
  
  typedef VolumeComputer<Triag2D> VolumeComputerType;
  
  /// constructor
  Triag2D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Triag2D"; }
  
  Real computeVolume(const std::vector<RealVector*>& coord) 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end TriagD2
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////
  
Real VolumeComputer<P1::Triag2D>::computeVolume(const std::vector<RealVector*>& coord) 
{
//  RealMatrix matrix(3,3);
//  for (Uint i = 0; i < 3; ++i) {
//    for (Uint j = 0; j < 3; ++j) {
//      if (j > 0) {
//        matrix(i,j) = (*coord[i])[j-1];
//      }
//      else {
//        matrix(i,j) = 1.0;
//      }
//    }
//  }
//  
//  const Real volume = 0.5*matrix.determ3();
//  
//  return volume;

  RealVector* A,B,C;
  A=coord[0];
  B=coord[1];
  C=coord[2];
  
  return 0.5*((B[XX]-A[XX])*(C[YY]-A[YY])-(C[XX]-A[XX])*(B[YY]-A[YY]));
  
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Triag2D_HH
