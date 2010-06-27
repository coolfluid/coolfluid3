#ifndef CF_Mesh_CGNS_Shared_hpp
#define CF_Mesh_CGNS_Shared_hpp

////////////////////////////////////////////////////////////////////////////////


#include <cgnslib.h>

#include "Mesh/CGNS/CGNSAPI.hpp"
#include "Mesh/CGNS/CGNSExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {

#define CALL_CGNS(cgns_func) {                                                 \
                               const Uint ierr = cgns_func;                    \
                               if (ierr)                                       \
                               {                                               \
                                 const char * error_msg = cg_get_error();      \
                                 throw CGNSException (FromHere(),error_msg);   \
                               }                                               \
                             }
  
#define CGNS_CHAR_MAX 1024
#define CGNS_VERT_IDX 0
#define CGNS_CELL_IDX 1
#define CGNS_BVRT_IDX 2

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNS mesh format common functionality
/// @author Willem Deconinck
class CGNS_API Shared
{
public:
  
  /// constructor
  Shared();
  
  /// Gets the Class name
  static std::string getClassName() { return "Shared"; }
  
  std::vector<std::string>& get_supported_element_types() { return m_supported_element_types; }

protected:

  std::map<ElementType_t,std::string> m_elemtype_CGNS_to_CF;
  std::map<std::string,ElementType_t> m_elemtype_CF_to_CGNS;

private:
  std::vector<std::string> m_supported_element_types;

}; // end Shared


////////////////////////////////////////////////////////////////////////////////

} // namespace CGNS
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGNS_Shared_hpp
