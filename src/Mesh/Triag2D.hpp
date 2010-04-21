#ifndef CF_Triag2D_HH
#define CF_Triag2D_HH

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/MeshAPI.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealVector.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  class Triag2D;
  
////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<Triag2D> 
{
public:
  static Real computeVolume(const std::vector<RealVector*>& coord); 
};

////////////////////////////////////////////////////////////////////////////////
    
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
class Mesh_API Triag2D : public ElementType
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
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_HH
