#ifndef CF_Mesh_P1_Quad2D_hpp
#define CF_Mesh_P1_Quad2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Quad2D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Quad2D> 
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord); 

};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 2D Quad mesh element
/// @author Willem Deconinck
class P1_API Quad2D : public ElementType
{
public:
  
  typedef VolumeComputer<Quad2D> VolumeComputerType;
  
  /// constructor
  Quad2D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Quad2D"; }

  /// Get the full name defining this element type uniquely
  static std::string getFullName() { return "P1-Quad2D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end Quad2D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Quad2D>::computeVolume(const std::vector<CArray::Row>& coord) 
{
  const Real diagonalsProd =
  (coord[2][0] - coord[0][0]) * (coord[3][1] - coord[1][1]) -
  (coord[2][1] - coord[0][1]) * (coord[3][0] - coord[1][0]);
  
  return 0.5*diagonalsProd;
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Quad2D_hpp
