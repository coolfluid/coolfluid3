#ifndef CF_Mesh_P1_Triag2D_HH
#define CF_Mesh_P1_Triag2D_HH

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1{
  
  class Triag2D;
  
////////////////////////////////////////////////////////////////////////////////
  
template<>
class P1_API VolumeComputer<Triag2D> 
{
public:
  static Real computeVolume(const std::vector<RealVector*>& coord); 
};

////////////////////////////////////////////////////////////////////////////////
    
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

private:
  
}; // end TriagD2
  
////////////////////////////////////////////////////////////////////////////////
  
Real VolumeComputer<Triag2D>::computeVolume(const std::vector<RealVector*>& coord) 
{
  RealMatrix matrix(3,3);
  for (Uint i = 0; i < 3; ++i) {
    for (Uint j = 0; j < 3; ++j) {
      if (j > 0) {
        matrix(i,j) = (*coord[i])[j-1];
      }
      else {
        matrix(i,j) = 1.0;
      }
    }
  }
  
  const Real volume = 0.5*matrix.determ3();
  
  return volume;
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace P1
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Triag2D_HH
