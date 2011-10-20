// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CGNS_Shared_hpp
#define cf3_Mesh_CGNS_Shared_hpp

////////////////////////////////////////////////////////////////////////////////

#include <cgnslib.h>

#include "Mesh/CGNS/LibCGNS.hpp"
#include "Mesh/CGNS/CGNSExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {

  class Geometry;
  class CRegion;

namespace CGNS {

#define CALL_CGNS(cgns_func) {                                                 \
                               const Uint ierr = cgns_func;                    \
                               if (ierr)                                       \
                               {                                               \
                                 const char * error_msg = cg_get_error();      \
                                 throw CGNSException (FromHere(),error_msg);   \
                               }                                               \
                             }

#define CGNS_CHAR_MAX 1024
#define CGNS_VERT_IDX 0
#define CGNS_CELL_IDX 1
#define CGNS_BVRT_IDX 2

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNS mesh format common functionality
/// @author Willem Deconinck
class Mesh_CGNS_API Shared
{
public:

  /// constructor
  Shared();

  /// Gets the Class name
  static std::string type_name() { return "Shared"; }

  std::vector<std::string>& get_supported_element_types() { return m_supported_element_types; }

protected:

  std::map<ElementType_t,std::string> m_elemtype_CGNS_to_CF;
  std::map<std::string,ElementType_t> m_elemtype_CF3_to_CGNS;

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
    int total_nbVertices;
    int nbVertices[3];
    int nbElements;
    int nbBdryVertices;
    int coord_dim;
    int nbGrids;
    int nbSols;
    int nbSections;
    int nbBocos;
    Uint total_nbElements;
    Geometry* nodes;
    Uint nodes_start_idx;
    //
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
    int elemStartIdx;
    int elemEndIdx;
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


  std::map<int,CRegion*> m_base_map;
  std::map<int,CRegion*> m_zone_map;
  std::map<int,CRegion*> m_section_map;
  std::map<int,Geometry*>  m_nodes_map;
  std::map<int,CRegion*> m_boco_map;

private:
  std::vector<std::string> m_supported_element_types;

}; // end Shared


////////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Mesh_CGNS_Shared_hpp
