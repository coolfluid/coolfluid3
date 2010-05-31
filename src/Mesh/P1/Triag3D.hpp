#ifndef CF_Mesh_P1_Triag3D_hpp
#define CF_Mesh_P1_Triag3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Triag3D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Triag3D>
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord); 
};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
class P1_API Triag3D : public ElementType
{
public:
  
  typedef VolumeComputer<Triag3D> VolumeComputerType;
  
  /// constructor
  Triag3D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Triag3D"; }

  /// Get the full name defining this element type uniquely
  static std::string getFullName() { return "P1-Triag3D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end Triag3D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Triag3D>::computeVolume(const std::vector<CArray::Row>& coord)
{
  RealVector V1(3), V2(3), cross(3);
  V1[XX] = coord[1][XX]-coord[0][XX];
  V1[YY] = coord[1][YY]-coord[0][YY];
  V1[ZZ] = coord[1][ZZ]-coord[0][ZZ];
  V2[XX] = coord[2][XX]-coord[0][XX];
  V2[YY] = coord[2][YY]-coord[0][YY];
  V2[ZZ] = coord[2][ZZ]-coord[0][ZZ];
  Math::MathFunctions::crossProd(V1,V2,cross);
  return 0.5*cross.norm2();
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Triag3D_hpp
