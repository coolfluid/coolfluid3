#ifndef CF_Mesh_P1_Line3D_hpp
#define CF_Mesh_P1_Line3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Line3D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Line3D>
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord);
};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
class P1_API Line3D : public ElementType
{
public:
  
  typedef VolumeComputer<Line3D> VolumeComputerType;
  
  /// constructor
  Line3D();
  
  /// Gets the Class name
  static std::string type_name() { return "Line3D"; }

  /// Get the full name defining this element type uniquely
  virtual std::string getElementTypeName() { return "P1-Line3D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) const 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end Line3D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

inline Real VolumeComputer<P1::Line3D>::computeVolume(const std::vector<CArray::Row>& coord)
{
  // return the distance between the 2 nodes
  return std::sqrt(std::pow(coord[1][XX]-coord[0][XX],2)+
                   std::pow(coord[1][YY]-coord[0][YY],2)+
                   std::pow(coord[1][ZZ]-coord[0][ZZ],2));
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Line3D_hpp
