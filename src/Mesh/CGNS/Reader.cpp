#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include "Common/ObjectProvider.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CGNS/Reader.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ObjectProvider < Mesh::CGNS::Reader,
                         Mesh::MeshReader,
                         Mesh::CGNS::CGNSLib >
aCGNSReader_Provider ( "Mesh::CGNS::Reader" );

//////////////////////////////////////////////////////////////////////////////

Reader::Reader()
: MeshReader(),
  m_isCoordinatesCreated(false)
{

}

//////////////////////////////////////////////////////////////////////////////

void Reader::read(boost::filesystem::path& fp, const CMesh::Ptr& mesh)
{

  set_mesh(mesh);

  CRegion::Ptr regions = m_mesh->create_region("regions");

  // open file in read mode
  cg_open(fp.string().c_str(),CG_MODE_READ,&m_idx.file);

  // check how many bases we have
  int nbBases;
  cg_nbases(m_idx.file,&nbBases);

  CFinfo << "nb bases : " << nbBases << "\n" << CFendl;
  for (m_idx.base = 1; m_idx.base<=nbBases; ++m_idx.base)
  {
    CFinfo << "m_idx.base = " << m_idx.base << "\n" << CFendl;
    read_base(regions);
  }

  // close the CGNS file
  cg_close(m_idx.file);

}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_base(CRegion::Ptr& parent_region)
{


  // get the name, dimension and physical dimension from the base
  char base_name_char[CGNS_CHAR_MAX];
  std::string base_name;
  int cell_dim;
  int phys_dim;
  cg_base_read(m_idx.file,m_idx.base,base_name_char,&cell_dim,&phys_dim);
  base_name=base_name_char;
  boost::algorithm::replace_all(base_name," ","_");
  //if (bitr->m_cell_dim != bitr->m_phys_dim) throw CGNSException (FromHere(),"Cannot handle cell dimensions different than physical dimensions.");

  CFinfo << "base name     : " << base_name  << "\n" << CFendl;
  CFinfo << "base cell dim : " << cell_dim  << "\n" << CFendl;
  CFinfo << "base phys dim : " << phys_dim  << "\n" << CFendl;

  // check how many zones we have
  int nbZones;
  cg_nzones(m_idx.file,m_idx.base,&nbZones);
  CFinfo << "number of zones     : " << nbZones  << "\n" << CFendl;

  // create region for the base in mesh
  CRegion::Ptr base_region = parent_region->create_region(base_name);

  for (m_idx.zone = 1; m_idx.zone<=nbZones; ++m_idx.zone)
  {
    CFinfo << "m_idx.zone = " << m_idx.zone << "\n" << CFendl;
    read_zone(base_region);
  }
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_zone(CRegion::Ptr& parent_region)
{


  // get zone type
  ZoneType_t zone_type;
  cg_zone_type(m_idx.file,m_idx.base,m_idx.zone,&zone_type);
  //if (zone_type == Structured) throw CGNSException (FromHere(),"Cannot handle structured meshes.");
  CFinfo << "zone type: " << CFendl;
  if (zone_type == Structured)
    CFinfo << "Structured \n" << CFendl;
  else if (zone_type == Unstructured)
    CFinfo << "Unstructured \n" << CFendl;
  else
    CFinfo << "Unknown zone_type \n" << CFendl;


  // get zone size and name
  char zone_name_char[CGNS_CHAR_MAX];
  std::string zone_name;
  int size[3];
  cg_zone_read(m_idx.file,m_idx.base,m_idx.zone,zone_name_char,size);
    zone_name = zone_name_char;
    boost::algorithm::replace_all(zone_name," ","_");
  CFinfo << "zone name   : " << zone_name << "\n" << CFendl;

    m_size.nbVertices = size[CGNS_VERT_IDX];
    m_size.nbElements = size[CGNS_CELL_IDX];
    m_size.nbBoundaryVertices = size[CGNS_BVRT_IDX];
  // get the number of grids
  int nbGrids;
  cg_ngrids(m_idx.file,m_idx.base,m_idx.zone,&nbGrids);
  // nb coord dims
  cg_ncoords(m_idx.file,m_idx.base,m_idx.zone, &m_size.nbCoords);
  // find out number of solutions
  int nbSols;
  cg_nsols(m_idx.file,m_idx.base,m_idx.zone,&nbSols);
  // find out how many sections
  int nbSections;
  cg_nsections(m_idx.file,m_idx.base,m_idx.zone,&nbSections);
  // find out number of BCs that exist under this zone
  int nbBocos;
  cg_nbocos(m_idx.file,m_idx.base,m_idx.zone,&nbBocos);

  // Print zone info
  CFinfo << "nb coords   : " << m_size.nbCoords << "\n" << CFendl;
  CFinfo << "nb nodes    : " << m_size.nbVertices << "\n" << CFendl;
  CFinfo << "nb cells    : " << m_size.nbElements << "\n" << CFendl;
  CFinfo << "nb bnodes   : " << m_size.nbBoundaryVertices << "\n" << CFendl;
  CFinfo << "nb grids    : " << nbGrids << "\n" << CFendl;
  CFinfo << "nb sols     : " << nbSols << "\n" << CFendl;
  CFinfo << "nb sections : " << nbSections << "\n" << CFendl;
  CFinfo << "nb bcs      : " << nbBocos << "\n" << CFendl;

  // Create a region for this zone
  parent_region->create_region(zone_name);
  CRegion::Ptr this_region = parent_region->get_component<CRegion>(zone_name);

  // read coordinates in this zone
  for (int i=1; i<=nbGrids; ++i)
    read_coordinates();

  // read sections (or subregions) in this zone
  for (m_idx.section=1; m_idx.section<=nbSections; ++m_idx.section)
    read_section(this_region);

  // read boundaryconditions (or subregions) in this zone
  for (m_idx.boco=1; m_idx.boco<=nbBocos; ++m_idx.boco)
    read_boco(this_region);

 /* // lower range index
  zitr->m_solutions.resize(zitr->m_nsols);
  sitr = zitr->m_solutions.begin();
  index_sol = 1;
  for (; sitr != zitr->m_solutions.end(); ++sitr, ++index_sol)
  {
    readSolution();
  }

  // resize storage of bcs
  zitr->m_bocos.resize(zitr->nbocos);
  bcitr = zitr->m_bocos.begin();
  index_boco = 1;
  for (; bcitr != zitr->m_bocos.end(); ++bcitr, ++index_boco)
  {
    readBoco();
  }
*/
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_coordinates()
{


  // create coordinates component  mesh/coordinates
  if (!m_isCoordinatesCreated)
  {
    m_mesh->create_array("coordinates");
    m_mesh->get_component<CArray>("coordinates")->initialize(m_size.nbCoords);
    m_isCoordinatesCreated = true;
  }

  CArray::Ptr coordinates = m_mesh->get_component<CArray>("coordinates");

  // read coordinates
  int one = 1;
  Real *xCoord;
  Real *yCoord;
  Real *zCoord;
  switch (m_size.nbCoords)
  {
    case 3:
      zCoord = new Real[m_size.nbVertices];
      cg_coord_read(m_idx.file,m_idx.base,m_idx.zone, "CoordinateZ", RealDouble, &one, &m_size.nbVertices, zCoord);
     case 2:
       yCoord = new Real[m_size.nbVertices];
       cg_coord_read(m_idx.file,m_idx.base,m_idx.zone, "CoordinateY", RealDouble, &one, &m_size.nbVertices, yCoord);
     case 1:
       xCoord = new Real[m_size.nbVertices];
       cg_coord_read(m_idx.file,m_idx.base,m_idx.zone, "CoordinateX", RealDouble, &one, &m_size.nbVertices, xCoord);
  }

  CArray::Buffer buffer = coordinates->create_buffer(m_size.nbVertices);

  std::vector<Real> row(m_size.nbCoords);
  for (int i=0; i<m_size.nbVertices; ++i)
  {
    switch (m_size.nbCoords)
    {
      case 3:
        row[2] = zCoord[i];
       case 2:
        row[1] = yCoord[i];
       case 1:
        row[0] = xCoord[i];
     }
    buffer.add_row(row);
  }

  delete_ptr(xCoord);
  delete_ptr(yCoord);
  delete_ptr(zCoord);
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_section(CRegion::Ptr& parent_region)
{


  char section_name_char[CGNS_CHAR_MAX];
  std::string section_name;
  ElementType_t element_type;
  int eBegin, eEnd, nBdry, parentFlag;
  cg_section_read(m_idx.file, m_idx.base, m_idx.zone, m_idx.section, section_name_char, &element_type,
                          &eBegin, &eEnd, &nBdry, &parentFlag);

  section_name=section_name_char;
  boost::algorithm::replace_all(section_name," ","_");

  CFinfo << "\n\nsection: " << section_name << "\n" << CFendl;
  CRegion::Ptr region = parent_region->create_region(section_name);

  if (element_type == MIXED)
  {
    CFinfo << "etype: MIXED --> create subregions for each element type \n" << CFendl;
    std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
        create_buffermap_for_elementConnectivity(region,get_supported_element_types());
    for (int elem=eBegin;elem<=eEnd;++elem)
    {
      // Read one line of connectivity
      int elemNodeCount;
      cg_ElementPartialSize(m_idx.file,m_idx.base,m_idx.zone,m_idx.section,elem,elem,&elemNodeCount);
      int elemNodes[1][elemNodeCount];
      int parent_data;
      cg_elements_partial_read(m_idx.file,m_idx.base,m_idx.zone,m_idx.section,elem,elem,*elemNodes,&parent_data);
      elemNodeCount--;
      ElementType_t etype = static_cast<ElementType_t>(elemNodes[0][0]);

      // Take out the nodes and put in the buffer of this element type
      std::vector<Uint> row;
      row.reserve(elemNodeCount);
      for (int n=1;n<=elemNodeCount;++n)  // n=0 is the cell type
        row.push_back(elemNodes[0][n]-1); // -1 because cgns has index-base 1 instead of 0
      buffer[m_elemtype_CGNS_to_CF[etype]]->add_row(row);
    }
  }
  else
  {
    CFinfo << "etype: " << cg_ElementTypeName(element_type) << "\n" << CFendl;

    CRegion::Ptr leaf_region = region->create_leaf_region(m_elemtype_CGNS_to_CF[element_type]);
    CTable::Buffer buffer = leaf_region->get_component<CTable>("table")->create_buffer();

    int elemNodeCount;
    cg_npe(element_type,&elemNodeCount);
    CFinfo << "elemNodeCount = " << elemNodeCount << "\n" << CFendl;

    int elementDataSize;
    cg_ElementDataSize(m_idx.file,m_idx.base,m_idx.zone,m_idx.section,&elementDataSize	);
    CFinfo << "elementDataSize = " << elementDataSize << "\n" << CFendl;

    int nbNodes = elementDataSize/elemNodeCount;
    CFinfo << "nbNodes = " << nbNodes << "\n" << CFendl;


    int elemNodes[nbNodes][elemNodeCount];
    int parent_data;
    cg_elements_read	(m_idx.file,m_idx.base,m_idx.zone,m_idx.section,*elemNodes,&parent_data);

    // fill connectivity table
    for (int i=0; i<nbNodes; i++)
    {
      std::vector<Uint> row;
      row.reserve(elemNodeCount);
      for (int n=0;n<elemNodeCount;++n)
        row.push_back(elemNodes[i][n]-1); // -1 because cgns has index-base 1 instead of 0
      buffer.add_row(row);
    }

  }

  remove_empty_leaf_regions(region);
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_boco(CRegion::Ptr& parent_region)
{
  // Read the info for this boundary condition.
  char boco_name_char[CGNS_CHAR_MAX];
  std::string boco_name;
  BCType_t boco_type;
  PointSetType_t ptset_type;
  int nBC_elem;
  int normalIndex;
  int normalListFlag;
  DataType_t normalDataType;
  int nDataSet;
  cg_boco_info(m_idx.file, m_idx.base, m_idx.zone, m_idx.boco, boco_name_char, &boco_type, &ptset_type,
               &nBC_elem, &normalIndex, &normalListFlag, &normalDataType, &nDataSet);
  boco_name = boco_name_char;

  // Read the element ID's
  int* boco_elems = new int [nBC_elem];
  void* NormalList(NULL);
  cg_boco_read(m_idx.file, m_idx.base, m_idx.zone, m_idx.boco, boco_elems, NormalList);

       /* And much more to make it fit into the */
       /* internal data structures. */
}

//////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // CF
