#ifndef CF_Mesh_OpenFOAM_BlockMeshReader_hpp
#define CF_Mesh_OpenFOAM_BlockMeshReader_hpp

#include "Mesh/CMeshReader.hpp"

namespace CF {
namespace Mesh {
namespace OpenFOAM {

//////////////////////////////////////////////////////////////////////////////

/// This class defines OpenFOAM BlockMesh mesh format reader
/// @author Bart Janssens
class BlockMeshReader : public CMeshReader
{
public:

  /// constructor
  BlockMeshReader(const CName& name);

  /// Gets the Class name
  static std::string type_name() { return "BlockMeshReader"; }
  
  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual std::string get_format() { return "blockMeshDict"; }

  virtual std::vector<std::string> get_extensions();
  
  virtual void read_from_to(boost::filesystem::path& path, const CMesh::Ptr& mesh);
  
private:
  static void regist_signals ( BlockMeshReader* self ) {}

}; // end BlockMeshReader


////////////////////////////////////////////////////////////////////////////////

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_OpenFOAM_BlockMeshReader_hpp */
