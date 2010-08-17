#ifndef CF_Mesh_CMeshInfo_hpp
#define CF_Mesh_CMeshInfo_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  class CField;

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_API CMeshInfo : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CMeshInfo> Ptr;
    typedef boost::shared_ptr<CMeshInfo const> ConstPtr;

private: // typedefs
  
public: // functions
  
  /// constructor
  CMeshInfo( const CName& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CMeshInfo"; }

  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual void transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args);
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;
  
private: // functions
 
  std::string print_region_tree(const CRegion& region, Uint level=0);
  std::string print_field_tree(const CField& field, Uint level=0);
  std::string print_elements(const Component& region, Uint level=0);

  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  CMesh::Ptr m_mesh;
  
}; // end CMeshInfo


////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshInfo_hpp
