#ifndef CF_Mesh_Gmsh_CWriter_hpp
#define CF_Mesh_Gmsh_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/GeoShape.hpp"

#include "Mesh/Gmsh/GmshAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Gmsh mesh format writer
/// @author Willem Deconinck
class Gmsh_API CWriter : public CMeshWriter
{
public: // typedefs

    typedef boost::shared_ptr<CWriter> Ptr;
    typedef boost::shared_ptr<CWriter const> ConstPtr;

private: // typedefs
  
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
  
public: // functions
  
  /// constructor
  CWriter( const CName& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CWriter"; }

  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path);

  virtual std::string get_format() { return "Gmsh"; }

  virtual std::vector<std::string> get_extensions();

private: // functions
  
  void write_header(std::fstream& file);
  
  void write_coordinates(std::fstream& file);
  
  void write_connectivity(std::fstream& file);
  
  void write_nodal_data(std::fstream& file);
  
  void write_element_data(std::fstream& file);
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  PhysicalGroupMap m_groups;
  
  std::map<GeoShape::Type,Uint> m_elementTypes;
  
  std::map<CElements*,Uint> m_node_start_idx;
  std::map<CElements*,Uint> m_element_start_idx;
  
}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // namespace Gmsh
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Gmsh_CWriter_hpp
