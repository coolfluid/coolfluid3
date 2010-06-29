#ifndef CF_Mesh_CMeshExtract_hpp
#define CF_Mesh_CMeshExtract_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_API CMeshExtract : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CMeshExtract> Ptr;
    typedef boost::shared_ptr<CMeshExtract const> ConstPtr;

private: // typedefs
  
public: // functions
  
  /// constructor
  CMeshExtract( const CName& name );
  
  /// Gets the Class name
  static std::string getClassName() { return "CMeshExtract"; }

  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual void transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args);

private: // functions
 
  std::string print_region_tree(const CRegion& region, Uint level=0);
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  CMesh::Ptr m_mesh;
  
}; // end CMeshExtract


////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshExtract_hpp
