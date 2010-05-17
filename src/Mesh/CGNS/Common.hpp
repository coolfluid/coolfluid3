#ifndef CF_Mesh_CGNS_Common_hpp
#define CF_Mesh_CGNS_Common_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CGNS/CGNSAPI.hpp"
#include "cgnslib.h"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {

#define CGNS_CHAR_MAX 1024
#define CGNS_VERT_IDX 0
#define CGNS_CELL_IDX 1
#define CGNS_BVRT_IDX 2

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNStral mesh format Common
/// @author Willem Deconinck
class CGNS_API Common
{
public:
  
  /// constructor
  Common();
  
  /// Gets the Class name
  static std::string getClassName() { return "Common"; }
  
  std::vector<std::string>& get_supported_element_types() { return m_supported_element_types; }

protected:

  std::map<ElementType_t,std::string> m_elemtype_CGNS_to_CF;
  std::map<std::string,ElementType_t> m_elemtype_CF_to_CGNS;

private:
  std::vector<std::string> m_supported_element_types;
}; // end Common


////////////////////////////////////////////////////////////////////////////////

} // namespace CGNS
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGNS_Common_hpp
