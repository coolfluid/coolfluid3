#ifndef CF_Mesh_P1_Line2D_hpp
#define CF_Mesh_P1_Line2D_hpp

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
struct Mesh_API Line2D : public ElementTypeBase
{

  /// constructor
  Line2D();
  
  /// Gets the Class name
  static std::string type_name() { return "Line2D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::LINE;
  
  /// @return number of faces
  static const Uint nb_faces = 0;
  
  /// @return number of edges
  static const Uint nb_edges = 1;
  
  /// @return m_dimensionality
  static const Uint dimensionality = DIM_1D;
  
  /// @return m_dimension
  static const Uint dimension = DIM_2D;

}; // end Line2D
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Line2D_hpp
