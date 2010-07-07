#ifndef CF_Mesh_Hexa3D_hpp
#define CF_Mesh_Hexa3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>

#include "Math/RealMatrix.hpp"

#include "Mesh/Elements/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
struct Mesh_API Hexa3D : public ElementTypeBase
{

  /// constructor
  Hexa3D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Hexa3D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::HEXA;
  
  /// @return number of faces
  static const Uint nb_faces = 6;
  
  /// @return number of edges
  static const Uint nb_edges = 12;
  
  /// @return m_dimensionality
  static const Uint dimensionality = DIM_3D;
  
  /// @return m_dimension
  static const Uint dimension = DIM_3D;

}; // end Hexa3D
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Hexa3D_hpp
