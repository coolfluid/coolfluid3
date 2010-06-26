#ifndef CF_Mesh_Neu_CWriter_hpp
#define CF_Mesh_Neu_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>
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

public: // typedefs

  typedef boost::shared_ptr<CWriter> Ptr;
  typedef boost::shared_ptr<CWriter const> ConstPtr;
  
public: // functions
  
  /// constructor
  CWriter( const CName& name );
  
  /// Gets the Class name
  static std::string getClassName() { return "CWriter"; }

  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path);

  virtual std::string get_format() { return "Neu"; }

  virtual std::vector<std::string> get_extensions();

private: // functions
  
  void write_headerData(std::fstream& file);

  void write_coordinates(std::fstream& file);

  void write_connectivity(std::fstream& file);

  void write_groups(std::fstream& file);

  void write_boundaries(std::fstream& file);

  void create_nodes_to_element_connectivity();

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data
  
  // supported types from coolfluid. Neutral can support more.
  std::vector<std::string> m_supported_types;
  
  std::map<GeoShape::Type,Uint> m_CFelement_to_NeuElement;

  /// implementation detail, raw pointers are safe as keys
  std::map<CRegion const*,Uint> m_global_start_idx;

  std::vector<std::vector<Uint> > m_faces_cf_to_neu;
  std::vector<std::vector<Uint> > m_faces_neu_to_cf;

  std::map<Uint,std::vector<std::pair<CRegion const*,Uint> > > m_n2e;

  boost::tuple<CRegion const* const,Uint,Uint> find_element_for_face(const CElements& face, const CTable::ConstRow& nodes, const Component& parent);

  CMesh::Ptr m_mesh;
  
}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // namespace Neu
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CWriter_hpp
