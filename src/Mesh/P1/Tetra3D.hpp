#ifndef CF_Mesh_P1_Tetra3D_hpp
#define CF_Mesh_P1_Tetra3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"
#include "Math/RealVector.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Tetra3D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Tetra3D>
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord) ; 

};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 3D Tetra mesh element
/// @author Willem Deconinck
class P1_API Tetra3D : public ElementType
{
public:
  
  typedef VolumeComputer<Tetra3D> VolumeComputerType;
  
  /// constructor
  Tetra3D();
  
  /// Gets the Class name
  static std::string type_name() { return "Tetra3D"; }

  /// Get the full name defining this element type uniquely
  virtual std::string getElementTypeName() { return "P1-Tetra3D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) const 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end Tetra3D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Tetra3D>::computeVolume(const std::vector<CArray::Row>& coord)
{
  const Real x0 = coord[0][XX];
  const Real y0 = coord[0][YY];
  const Real z0 = coord[0][ZZ];

  const Real x1 = coord[1][XX];
  const Real y1 = coord[1][YY];
  const Real z1 = coord[1][ZZ];

  const Real x2 = coord[2][XX];
  const Real y2 = coord[2][YY];
  const Real z2 = coord[2][ZZ];

  const Real x3 = coord[3][XX];
  const Real y3 = coord[3][YY];
  const Real z3 = coord[3][ZZ];

  return
      (x2*y1*z0 - x3*y1*z0 - x1*y2*z0 + x3*y2*z0 + x1*y3*z0 -
      x2*y3*z0 - x2*y0*z1 + x3*y0*z1 + x0*y2*z1 - x3*y2*z1 -
      x0*y3*z1 + x2*y3*z1 + x1*y0*z2 - x3*y0*z2 - x0*y1*z2 +
      x3*y1*z2 + x0*y3*z2 - x1*y3*z2 - x1*y0*z3 + x2*y0*z3 +
      x0*y1*z3 - x2*y1*z3 - x0*y2*z3 + x1*y2*z3)/6.;

}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Tetra3D_hpp
