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
  
  void write_header(std::fstream& file);
  
  struct PhysicalGroup
  {
    PhysicalGroup() {}
    PhysicalGroup(Uint phys_dim, Uint phys_number, std::string phys_name)
     : dimension(phys_dim), number(phys_number), name(phys_name) {}
    Uint dimension;
    Uint number;
    std::string name;
  };
  
  typedef std::map<std::string,PhysicalGroup> PhysicalGroupMap;
  PhysicalGroupMap m_groups;
  
  void write_coordinates(std::fstream& file);
  
  void write_connectivity(std::fstream& file);
  
  std::map<GeoShape::Type,Uint> m_elementTypes;
  
  CMesh::Ptr m_mesh;

}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // namespace Neu
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CWriter_hpp
