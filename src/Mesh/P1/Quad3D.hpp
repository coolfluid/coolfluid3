#ifndef CF_Mesh_P1_Quad3D_hpp
#define CF_Mesh_P1_Quad3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Quad3D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Quad3D>
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord) ; 

};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 2D Quad mesh element
/// @author Willem Deconinck
class P1_API Quad3D : public ElementType
{
public:
  
  typedef VolumeComputer<Quad3D> VolumeComputerType;
  
  /// constructor
  Quad3D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Quad3D"; }

  /// Get the full name defining this element type uniquely
  static std::string getFullName() { return "P1-Quad3D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) const 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end Quad3D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Quad3D>::computeVolume(const std::vector<CArray::Row>& coord)
{
  RealVector V1 = RealVector(coord[2]) - RealVector(coord[0]);
  RealVector V2 = RealVector(coord[3]) - RealVector(coord[1]);
  RealVector cross(3);
  Math::MathFunctions::crossProd(V1,V2,cross);
  return 0.5*cross.norm2();
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Quad3D_hpp
