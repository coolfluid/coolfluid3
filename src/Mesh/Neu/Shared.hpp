#ifndef CF_Mesh_Neu_Shared_hpp
#define CF_Mesh_Neu_Shared_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/GeoShape.hpp"

#include "Mesh/Neu/NeuAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neu mesh format common functionality
/// @author Willem Deconinck
class Neu_API Shared
{
public:
  
  /// constructor
  Shared();
  
  /// Gets the Class name
  static std::string type_name() { return "Shared"; }
  
  std::vector<std::string>& get_supported_element_types() { return m_supported_types; }

protected:

  enum NeuElement {LINE=1,QUAD=2,TRIAG=3,HEXA=4,TETRA=6};
  
  std::map<GeoShape::Type,Uint> m_CFelement_to_NeuElement;
  std::vector<std::string> m_supported_types;    
  std::vector<std::vector<Uint> > m_faces_cf_to_neu;
  std::vector<std::vector<Uint> > m_faces_neu_to_cf;
  std::vector<std::vector<Uint> > m_nodes_cf_to_neu;
  std::vector<std::vector<Uint> > m_nodes_neu_to_cf;

}; // end Shared


////////////////////////////////////////////////////////////////////////////////

} // namespace Neu
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_Shared_hpp
