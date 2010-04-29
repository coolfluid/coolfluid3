#ifndef CF_Mesh_P1_Triag2D_hpp
#define CF_Mesh_P1_Triag2D_hpp

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
  static Real computeVolume(const std::vector<CArray::Row>& coord); 
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
  
  Real computeVolume(const std::vector<CArray::Row>& coord) 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end Triag2D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Triag2D>::computeVolume(const std::vector<RealVector*>& coord) 
{
  return 0.5*(((*coord[1])[XX]-(*coord[0])[XX])*((*coord[2])[YY]-(*coord[0])[YY])
    -((*coord[2])[XX]-(*coord[0])[XX])*((*coord[1])[YY]-(*coord[0])[YY]));
}

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Triag2D>::computeVolume(const std::vector<CArray::Row>& coord) 
{
  return 0.5*((coord[1][XX]-coord[0][XX])*(coord[2][YY]-coord[0][YY])
                -(coord[2][XX]-coord[0][XX])*(coord[1][YY]-coord[0][YY]));
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Triag2D_hpp
