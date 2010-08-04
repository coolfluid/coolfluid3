#ifndef CF_Mesh_CGNS_CReader_hpp
#define CF_Mesh_CGNS_CReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CGNS/CGNSAPI.hpp"
#include "Mesh/CGNS/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////
#include "Mesh/CElements.hpp"

namespace CF {
namespace Mesh {
  class CRegion;
namespace CGNS {

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNS mesh format reader
/// @author Willem Deconinck
  class CGNS_API CReader : public CMeshReader, public CGNS::Shared
{
public: // typedefs
  
  typedef boost::shared_ptr<CReader> Ptr;
  typedef boost::shared_ptr<CReader const> ConstPtr;
  
private: // typedefs

  typedef std::pair<boost::shared_ptr<CElements>,Uint> Region_TableIndex_pair;

public: // functions
  
  /// Contructor
  /// @param name of the component
  CReader ( const CName& name );

  /// Gets the Class name
  static std::string type_name() { return "CReader"; }
  
  virtual void read_from_to(boost::filesystem::path& fp, const boost::shared_ptr<CMesh>& mesh);

  virtual std::string get_format() { return "CGNS"; }

  virtual std::vector<std::string> get_extensions();

  static void defineConfigOptions ( CF::Common::OptionList& options );

private: // functions
  
  void read_base(CRegion& parent_region);
  void read_zone(CRegion& parent_region);
  void read_coordinates(CRegion& parent_region);
  void read_section(CRegion& parent_region);
  void read_boco(CRegion& parent_region);
  Uint get_total_nbElements();

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  std::vector<Region_TableIndex_pair> m_global_to_region;
  boost::shared_ptr<CMesh> m_mesh;

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
  bool m_uniqueBase;

  struct CGNS_File
  {
    int idx;
    int nbBases;
  } m_file;

  struct CGNS_Base
  {
    int idx;
    int cell_dim;
    int phys_dim;
    std::string name;
    bool unique;
    int nbZones;
  } m_base;

  struct CGNS_Zone
  {
    int idx;
    bool unique;
    std::string name;
    ZoneType_t type;
    int nbVertices;
    int nbElements;
    int nbBdryVertices;
    int coord_dim;
    int nbGrids;
    int nbSols;
    int nbSections;
    int nbBocos;
    Uint total_nbElements;
  } m_zone;

  struct CGNS_Section
  {
    int idx;
    bool unique;
    std::string name;
    ElementType_t type;
    int eBegin;
    int eEnd;
    int nbBdry;
    int parentFlag;
    int elemNodeCount;
    int elemDataSize;
    int parentData;
  } m_section;

  struct CGNS_Boco
  {
    int idx;
    bool unique;
    std::string name;
    BCType_t boco_type;  // e.g. BCDirichlet, BCSubsonicInflow, ...
    PointSetType_t ptset_type; // PointList / PointRange / ElementList / ElementRange
    int nBC_elem;
    int normalIndex;
    int normalListFlag;
    DataType_t normalDataType;
    int nDataSet;
  } m_boco;

}; // end CReader


////////////////////////////////////////////////////////////////////////////////

} // namespace CGNS
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGNS_CReader_hpp
