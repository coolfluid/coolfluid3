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
  
  /// create a field
  /// @param name of the field
  CField& create_field( const CName& name , CRegion& support);
//  CField& create_field_with_shapefunction( const CName& name , const CRegion& support, const ElementType& shape_function);
//  CField& create_field( const CName& name , const CField& other_field);
//  CField& create_field_with_shapefunction( const CName& name , const CField& other_field, const ElementType& shape_function);

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
