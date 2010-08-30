#ifndef CF_Mesh_CMesh_hpp
#define CF_Mesh_CMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

  class CRegion;
  class CField;
  class ElementType;

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
  typedef boost::shared_ptr<CMesh const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMesh ( const CName& name );

  /// Virtual destructor
  virtual ~CMesh();

  /// Get the class name
  static std::string type_name () { return "CMesh"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CMesh component

  /// create a region
  /// @param name of the region
  CRegion& create_region ( const CName& name );

  /// create a domain
  /// @param name of the domain
  CRegion& create_domain( const CName& name );

  /// @return the geometry
  const CRegion& domain() const;
  
  /// @return the geometry
  CRegion& domain();
  
  /// create a field with a given support
  /// @param name of the field
  CField& create_field( const CName& name , CRegion& support);
  
  /// create a field with the default support being the full mesh geometry
  CField& create_field( const CName& name);
  
  /// @return the field with given name
  const CField& field(const CName& name) const;
  
  /// @return the field with given name
  CField& field(const CName& name);
  
  void update_statistics();
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private:

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMesh_hpp
