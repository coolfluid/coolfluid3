#ifndef CF_Mesh_P1_Line2D_hpp
#define CF_Mesh_P1_Line2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Line2D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Line2D>
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord);
};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
class P1_API Line2D : public ElementType
{
public:
  
  typedef VolumeComputer<Line2D> VolumeComputerType;
  
  /// constructor
  Line2D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Line2D"; }

  /// Get the full name defining this element type uniquely
  virtual std::string getElementTypeName() { return "P1-Line2D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) const 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

  static const Line2D& instance();

private:
  
}; // Line2D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

inline Real VolumeComputer<P1::Line2D>::computeVolume(const std::vector<CArray::Row>& coord)
{
  // return the distance between the 2 nodes
  const Real dx = (coord[1][XX]-coord[0][XX]);
  const Real dy = (coord[1][YY]-coord[0][YY]);
  return std::sqrt(dx*dx + dy*dy);
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Line2D_hpp
