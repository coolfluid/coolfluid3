#ifndef CF_Mesh_Quad3D_hpp
#define CF_Mesh_Quad3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>

#include "Math/RealMatrix.hpp"

#include "Mesh/Elements/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 2D line mesh element
/// @author Willem Deconinck
struct Mesh_API Quad3D : public ElementTypeBase
{

  /// constructor
  Quad3D();
  
  /// Gets the Class name
  static std::string type_name() { return "Quad3D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::QUAD;
  
  /// @return number of faces
  static const Uint nb_faces = 1;
  
  /// @return number of edges
  static const Uint nb_edges = 4;
  
  /// @return m_dimensionality
  static const Uint dimensionality = DIM_2D;
  
  /// @return m_dimension
  static const Uint dimension = DIM_3D;

}; // end Quad3D
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Quad3D_hpp
