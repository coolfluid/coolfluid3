#ifndef CF_Mesh_CMesh_hpp
#define CF_Mesh_CMesh_hpp

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
/// @author Tiago Quintino
/// @author Willem Deconinck
class Mesh_API CMesh : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CMesh> Ptr;
  typedef Common::Component_iterator<CMesh> Iterator;

public: // functions

  /// Contructor
  /// @param name of the component
  CMesh ( const CName& name );

  /// Virtual destructor
  virtual ~CMesh();

  /// Get the class name
  static std::string getClassName () { return "CMesh"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CMesh component


  /// create a region
  /// @param name of the region
  boost::shared_ptr<CRegion> create_region ( const CName& name );
  
  /// create an array
  /// @param name of the array
  /// @todo allow templates
  boost::shared_ptr<CArray> create_array ( const CName& name );

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

#endif // CF_Mesh_CMesh_hpp
