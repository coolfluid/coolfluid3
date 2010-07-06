#ifndef CF_Mesh_Tetra3D_hpp
#define CF_Mesh_Tetra3D_hpp

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
struct Mesh_API Tetra3D : public ElementTypeBase
{

  /// constructor
  Tetra3D();
  
  /// Gets the Class name
  static std::string type_name() { return "Tetra3D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::TETRA;
  
  /// @return number of faces
  static const Uint nb_faces = 4;
  
  /// @return number of edges
  static const Uint nb_edges = 6;
  
  /// @return m_dimensionality
  static const Uint dimensionality = 3;
  
  /// @return m_dimension
  static const Uint dimension = 3;

}; // end Tetra3D
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Tetra3D_hpp
