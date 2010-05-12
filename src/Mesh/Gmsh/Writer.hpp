#ifndef CF_Mesh_Gmsh_Writer_hpp
#define CF_Mesh_Gmsh_Writer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/Gmsh/GmshAPI.hpp"
#include "Mesh/MeshWriter.hpp"
#include "Mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Gmshtral mesh format writer
/// @author Willem Deconinck
class Gmsh_API Writer : public MeshWriter
{
public:
  
  /// constructor
  Writer();
  
  /// Gets the Class name
  static std::string getClassName() { return "Writer"; }
  
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
  
  virtual void write_impl(std::fstream& file)
  {    
    // must be in correct order!
    write_header(file);
    write_coordinates(file);
    write_connectivity(file);
  }
  
  std::map<GeoShape::Type,Uint> m_elementTypes;
  
}; // end Writer


////////////////////////////////////////////////////////////////////////////////

} // namespace Gmsh
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Gmsh_Writer_hpp
