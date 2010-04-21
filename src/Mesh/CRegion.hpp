#ifndef CF_Mesh_CRegion_HH
#define CF_Mesh_CRegion_HH

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Region component class
/// This class stores
///   - subregions (same class)
///   - mesh connectivity table, assuming same element type
///   - element type
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API CRegion : public Common::Component {

public:

  /// Contructor
  /// @param name of the component
  CRegion ( const CName& name );

  /// Virtual destructor
  virtual ~CRegion();

  /// Get the class name
  static std::string getClassName () { return "CRegion"; }

  // functions specific to the CRegion component

  /// create a CRegion component
  /// @param name of the region
  void create_region ( const CName& name );
  
  /// create a CTable component
  /// @param name of the region
  void create_connectivityTable ( const CName& name );
  
  /// create a CElements component
  /// @param name of the region
  void create_elementType ( const CName& name );
  
private:

  std::vector< boost::shared_ptr<CRegion> > m_subregions;
  
  boost::shared_ptr<CTable> m_connTable;
  
  boost::shared_ptr<CElements> m_elementType;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_HH
