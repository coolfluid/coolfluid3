#ifndef CF_Mesh_Gmsh_CWriter_hpp
#define CF_Mesh_Gmsh_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/Gmsh/GmshAPI.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Gmsh mesh format writer
/// @author Willem Deconinck
class Gmsh_API CWriter : public CMeshWriter
{
public:
  
  /// constructor
  CWriter( const CName& name );
  
  /// Gets the Class name
  static std::string getClassName() { return "CWriter"; }

  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path);

  virtual std::string get_format() { return "Gmsh"; }

  virtual std::vector<std::string> get_extensions();

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

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // namespace Gmsh
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Gmsh_CWriter_hpp
