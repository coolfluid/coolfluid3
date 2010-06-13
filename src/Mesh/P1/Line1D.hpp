#ifndef CF_Mesh_P1_Line1D_hpp
#define CF_Mesh_P1_Line1D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Line1D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Line1D>
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord);
};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
class P1_API Line1D : public ElementType
{
public:
  
  typedef VolumeComputer<Line1D> VolumeComputerType;
  
  /// constructor
  Line1D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Line1D"; }

  /// Get the full name defining this element type uniquely
  static std::string getFullName() { return "P1-Line1D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) const 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end Line1D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Line1D>::computeVolume(const std::vector<CArray::Row>& coord)
{
  // return the distance between the 2 nodes
  return std::abs(coord[1][XX]-coord[0][XX]);
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Line1D_hpp
