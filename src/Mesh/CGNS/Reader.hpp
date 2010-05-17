#ifndef CF_Mesh_CGNS_Reader_hpp
#define CF_Mesh_CGNS_Reader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/MeshReader.hpp"
#include "Mesh/CGNS/CGNSAPI.hpp"
#include "Mesh/CGNS/Common.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CRegion;
namespace CGNS {

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNS mesh format reader
/// @author Willem Deconinck
  class CGNS_API Reader : public MeshReader, public CGNS::Common
{
public:
  
  /// constructor
  Reader();
  
  /// Gets the Class name
  static std::string getClassName() { return "Reader"; }
  
  virtual void read(boost::filesystem::path& fp, const boost::shared_ptr<CMesh>& mesh);

private:
  
  void read_base(CRegion::Ptr& parent_region);
  void read_zone(CRegion::Ptr& parent_region);
  void read_coordinates();
  void read_section(CRegion::Ptr& parent_region);
  void read_boco(CRegion::Ptr& parent_region);


  struct CGNS_Indexes
  {
    int file,base,zone,section,grid,boco,coord;
  } m_idx;

  struct CGNS_Size
  {
    int nbCoords;
    int nbVertices;
    int nbElements;
    int nbBoundaryVertices;
  } m_size;

  bool m_isCoordinatesCreated;

}; // end Reader


////////////////////////////////////////////////////////////////////////////////

} // namespace CGNS
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGNS_Reader_hpp
