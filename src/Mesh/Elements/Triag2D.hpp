#ifndef CF_Mesh_P1_Triag2D_hpp
#define CF_Mesh_P1_Triag2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>

#include "Math/RealMatrix.hpp"

#include "Mesh/Elements/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  class Triag2D;

////////////////////////////////////////////////////////////////////////////////
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
struct Mesh_API Triag2D : public ElementTypeBase
{

  /// constructor
  Triag2D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Triag2D"; }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::TRIAG;
  
  /// @return number of faces
  static const Uint nb_faces = 3;
  
  /// @return number of edges
  static const Uint nb_edges = 3;
  
  /// @return m_dimensionality
  static const Uint dimensionality = 2;
  
  /// @return m_dimension
  static const Uint dimension = 2;

}; // end Triag2D
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Triag2D_hpp
