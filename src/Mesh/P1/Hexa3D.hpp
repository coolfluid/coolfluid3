#ifndef CF_Mesh_P1_Hexa3D_hpp
#define CF_Mesh_P1_Hexa3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/P1/P1API.hpp"
#include "Mesh/ElementType.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Hexa3D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Hexa3D> 
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord); 

};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 3D Hexa mesh element
/// @author Willem Deconinck
class P1_API Hexa3D : public ElementType
{
public:
  
  typedef VolumeComputer<Hexa3D> VolumeComputerType;
  
  /// constructor
  Hexa3D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Hexa3D"; }

  /// Get the full name defining this element type uniquely
  static std::string getFullName() { return "P1-Hexa3D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

private:
  
}; // end Hexa3D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Hexa3D>::computeVolume(const std::vector<CArray::Row>& coord) 
{
  const Real diagonalsProd =
  (coord[2][0] - coord[0][0]) * (coord[3][1] - coord[1][1]) -
  (coord[2][1] - coord[0][1]) * (coord[3][0] - coord[1][0]);
  
  return 0.5*diagonalsProd;



  static RealVector subX6X1(3);
  static RealVector subX7X0(3);
  static RealVector subX6X3(3);
  static RealVector subX2X0(3);
  static RealVector subX5X0(3);
  static RealVector subX6X4(3);
  static RealVector tmp(3);
  static RealMatrix det(3,3, 0.0);

  for (Uint i=0; i<3; i++)
  {
    subX6X1[i] = coord[6][i] - coord[1][i];
    subX7X0[i] = coord[7][i] - coord[0][i];
    subX6X3[i] = coord[6][i] - coord[3][i];
    subX2X0[i] = coord[2][i] - coord[0][i];
    subX5X0[i] = coord[5][i] - coord[0][i];
    subX6X4[i] = coord[6][i] - coord[4][i];
  }


  tmp = subX6X1 + subX7X0;
  det.setRow(tmp, 0);
  det.setRow(subX6X3,1);
  det.setRow(subX2X0,2);
  Real volume = det.determ3();

  tmp = subX6X3 + subX5X0;
  det.setRow(subX7X0, 0);
  det.setRow(tmp,1);
  det.setRow(subX6X4,2);
  volume += det.determ3();

  tmp = subX6X4 + subX2X0;
  det.setRow(subX6X1, 0);
  det.setRow(subX5X0,1);
  det.setRow(tmp,2);
  volume += det.determ3();

  if (volume < 0.0) {
    CFinfo << "VolumeComputer<P1::Hexa3D> => volume < 0 => " <<  volume/12. << "\n" << CFflush;
  }

  return volume/12.;

}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Hexa3D_hpp
