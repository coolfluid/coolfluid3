#ifndef CF_Mesh_CMesh_HH
#define CF_Mesh_CMesh_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

  class CRegion;
  class CArray;

////////////////////////////////////////////////////////////////////////////////

/// Mesh component class
/// Mesh now stores:
///   - regions which subdivide in subregions
///   - arrays containing coordinates, variables, ...
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API CMesh : public Common::Component {

public:

  /// Contructor
  /// @param name of the component
  CMesh ( const CName& name );

  /// Virtual destructor
  virtual ~CMesh();

  /// Get the class name
  static std::string getClassName () { return "CMesh"; }

  // functions specific to the CMesh component


  /// create a region
  /// @param name of the region
  void create_region ( const CName& name );
  
  /// create an array
  /// @param name of the array
  /// @todo allow templates
  void create_array ( const CName& name );

private:

  /// storage of region pointers
  std::vector< boost::shared_ptr<CRegion> > m_regions;
  
  /// storage of array pointers
  std::vector< boost::shared_ptr<CArray> > m_arrays;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMesh_HH
