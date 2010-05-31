#ifndef CF_Mesh_Neu_CWriter_hpp
#define CF_Mesh_Neu_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/Neu/NeuAPI.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neu mesh format writer
/// @author Willem Deconinck
class Neu_API CWriter : public CMeshWriter
{
public:
  
  /// constructor
  CWriter( const CName& name );
  
  /// Gets the Class name
  static std::string getClassName() { return "CWriter"; }

  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path);

  virtual std::string get_format() { return "Neu"; }

private:
  
  void write_headerData(std::fstream& file);

  void write_coordinates(std::fstream& file);

  void write_connectivity(std::fstream& file);

  void write_groups(std::fstream& file);

  void write_boundaries(std::fstream& file);

  // supported types from coolfluid. Neutral can support more.
  std::vector<std::string> m_supported_types;
  
  std::map<GeoShape::Type,Uint> m_CFelement_to_NeuElement;

  std::map<CRegion::Ptr,Uint> m_global_start_idx;

  CMesh::Ptr m_mesh;

}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // namespace Neu
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CWriter_hpp
