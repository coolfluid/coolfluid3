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
  static Real computeVolume(const std::vector<CArray::Row>& coord) ; 

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
  static std::string type_name() { return "Quad2D"; }

  /// Get the full name defining this element type uniquely
  virtual std::string getElementTypeName() { return "P1-Quad2D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) const 
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
  (coord[2][XX] - coord[0][XX]) * (coord[3][YY] - coord[1][YY]) -
  (coord[2][YY] - coord[0][YY]) * (coord[3][XX] - coord[1][XX]);
  
  return 0.5*diagonalsProd;
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Quad2D_hpp
