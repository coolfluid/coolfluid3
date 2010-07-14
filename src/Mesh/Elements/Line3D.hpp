#ifndef CF_Mesh_Line3D_hpp
#define CF_Mesh_Line3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>

#include "Math/RealMatrix.hpp"

#include "Mesh/Elements/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 3D line mesh element
/// @author Willem Deconinck
struct Mesh_API Line3D : public ElementTypeBase
{

  /// constructor
  Line3D();
  
  /// Gets the Class name
  static std::string type_name() { return "Line3D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::LINE;
  
  /// @return number of faces
  static const Uint nb_faces = 0;
  
  /// @return number of edges
  static const Uint nb_edges = 1;
  
  /// @return m_dimensionality
  static const Uint dimensionality = DIM_1D;
  
  /// @return m_dimension
  static const Uint dimension = DIM_3D;

}; // end Line3D
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Line3D_hpp
