#ifndef CF_Mesh_P1_Quad2D_hpp
#define CF_Mesh_P1_Quad2D_hpp

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
struct Mesh_API Quad2D : public ElementTypeBase
{

  /// constructor
  Quad2D();
  
  /// Gets the Class name
  static std::string type_name() { return "Quad2D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::QUAD;
  
  /// @return number of faces
  static const Uint nb_faces = 4;
  
  /// @return number of edges
  static const Uint nb_edges = 4;
  
  /// @return m_dimensionality
  static const Uint dimensionality = 2;
  
  /// @return m_dimension
  static const Uint dimension = 2;

}; // end Quad2D
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Quad2D_hpp
