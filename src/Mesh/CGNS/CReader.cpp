#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include <boost/progress.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CGNS/CReader.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {
  
using namespace Common;
  
////////////////////////////////////////////////////////////////////////////////

ObjectProvider < CReader,
                 CMeshReader,
                 CGNSLib,
                 1 >
aCGNSReader_Provider ( "CGNS" );

//////////////////////////////////////////////////////////////////////////////

CReader::CReader(const CName& name)
: CMeshReader(name), Shared(),
  m_isCoordinatesCreated(false)
{
  BUILD_COMPONENT;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CReader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".cgns");
  return extensions;
}

//////////////////////////////////////////////////////////////////////////////

void CReader::defineConfigOptions ( OptionList& options )
{
  options.add< OptionT<bool> >
      ( "SectionsAreBCs",
        ("Treat Sections of lower dimensionality as BC. "
        "This means no BCs from cgns will be read"),
        true );
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_from_to(boost::filesystem::path& fp, const CMesh::Ptr& mesh)
{

  // Set the internal mesh pointer
  m_mesh = mesh;

  // Create basic region structure
  CRegion::Ptr regions        = m_mesh->create_region("regions");
  CRegion& volume_regions = regions->create_region("volume-regions");
  CRegion& bc_regions     = regions->create_region("bc-regions");

  // open file in read mode
  CALL_CGNS(cg_open(fp.string().c_str(),CG_MODE_READ,&m_file.idx));

  // check how many bases we have
  CALL_CGNS(cg_nbases(m_file.idx,&m_file.nbBases));

  // Store if there is only 1 base
  m_base.unique = m_file.nbBases==1 ? true : false;
  
  // Read every base (usually there is only 1)
  for (m_base.idx = 1; m_base.idx<=m_file.nbBases; ++m_base.idx)
    read_base(volume_regions);

  // remove bc_regions component if there are no bc's defined
  if (range_typed<CRegion>(bc_regions).empty())
    regions->remove_component(bc_regions.name());

  // close the CGNS file
  CALL_CGNS(cg_close(m_file.idx));

}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_base(CRegion& parent_region)
{

  // get the name, dimension and physical dimension from the base
  char base_name_char[CGNS_CHAR_MAX];
  CALL_CGNS(cg_base_read(m_file.idx,m_base.idx,base_name_char,&m_base.cell_dim,&m_base.phys_dim));
  m_base.name=base_name_char;
  boost::algorithm::replace_all(m_base.name," ","_");


  // check how many zones we have
  CALL_CGNS(cg_nzones(m_file.idx,m_base.idx,&m_base.nbZones));

  // create region for the base in mesh
  CRegion& base_region = m_base.unique ? parent_region : parent_region.create_region(m_base.name);

  // Read every zone in this base
  for (m_zone.idx = 1; m_zone.idx<=m_base.nbZones; ++m_zone.idx)
    read_zone(base_region);

}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_zone(CRegion& parent_region)
{
  // get zone type (Structured or Unstructured)
  CALL_CGNS(cg_zone_type(m_file.idx,m_base.idx,m_zone.idx,&m_zone.type));
  
  // For now only Unstructured zone types are supported
  if (m_zone.type == Structured) throw NotImplemented (FromHere(),"For now only Unstructured zone types are supported");

  // Read zone size and name
  char zone_name_char[CGNS_CHAR_MAX];
  int size[3];
  CALL_CGNS(cg_zone_read(m_file.idx,m_base.idx,m_zone.idx,zone_name_char,size));
  m_zone.name = zone_name_char;
  boost::algorithm::replace_all(m_zone.name," ","_");
  m_zone.nbVertices     = size[CGNS_VERT_IDX];
  m_zone.nbElements     = size[CGNS_CELL_IDX];
  m_zone.nbBdryVertices = size[CGNS_BVRT_IDX];

  // get the number of grids
  CALL_CGNS(cg_ngrids(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbGrids));
  // nb coord dims
  CALL_CGNS(cg_ncoords(m_file.idx,m_base.idx,m_zone.idx, &m_zone.coord_dim));
  // find out number of solutions
  CALL_CGNS(cg_nsols(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbSols));
  // find out how many sections
  CALL_CGNS(cg_nsections(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbSections));
  // find out number of BCs that exist under this zone
  CALL_CGNS(cg_nbocos(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbBocos));
  // Add up all the nb elements from all sections
  m_zone.total_nbElements = get_total_nbElements();


  // Create a region for this zone
  CRegion& this_region = parent_region.create_region(m_zone.name);

  // read coordinates in this zone
  for (int i=1; i<=m_zone.nbGrids; ++i)
    read_coordinates();

  // read sections (or subregions) in this zone
  m_global_to_region.reserve(m_zone.total_nbElements);
  for (m_section.idx=1; m_section.idx<=m_zone.nbSections; ++m_section.idx)
    read_section(this_region);

  // Only read boco's if sections are not defined as BC's
  if (!option("SectionsAreBCs")->value<bool>())
  {
    // read boundaryconditions (or subregions) in this zone
    for (m_boco.idx=1; m_boco.idx<=m_zone.nbBocos; ++m_boco.idx)
      read_boco();

    // Remove regions flagged as bc
    BOOST_FOREACH(CRegion& region, recursive_range_typed<CRegion>(this_region))
    {
      if(range_typed<CRegion>(region).empty())
      {
        if (get_named_component_typed<CElements>(region, "type").element_type().dimensionality() < static_cast<Uint>(m_base.cell_dim))
        {
          Component::Ptr region_to_rm = region.get_parent();
          // CFinfo << "Removing region flagged as bc : " << region_to_rm->name() << "\n" << CFflush;
          region_to_rm->get_parent()->remove_component(region_to_rm->name());
          region_to_rm.reset();
        }
      }
    }
  }

  // Cleanup:

  // truely deallocate the global_to_region vector
  m_global_to_region.resize(0);
  std::vector<Region_TableIndex_pair>().swap (m_global_to_region);


}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_coordinates()
{


  // create coordinates component  mesh/coordinates
  if (!m_isCoordinatesCreated)
  {
    m_mesh->create_array("coordinates");
    get_named_component_typed<CArray>(*m_mesh, "coordinates").initialize(m_zone.coord_dim);
    m_isCoordinatesCreated = true;
  }

  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");

  // read coordinates
  int one = 1;
  Real *xCoord;
  Real *yCoord;
  Real *zCoord;
  switch (m_zone.coord_dim)
  {
    case 3:
      zCoord = new Real[m_zone.nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateZ", RealDouble, &one, &m_zone.nbVertices, zCoord));
    case 2:
      yCoord = new Real[m_zone.nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateY", RealDouble, &one, &m_zone.nbVertices, yCoord));
    case 1:
      xCoord = new Real[m_zone.nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateX", RealDouble, &one, &m_zone.nbVertices, xCoord));
  }

  CArray::Buffer buffer = coordinates->create_buffer();
  buffer.increase_array_size(m_zone.nbVertices);
  std::vector<Real> row(m_zone.coord_dim);
  for (int i=0; i<m_zone.nbVertices; ++i)
  {
    switch (m_zone.coord_dim)
    {
      case 3:
        row[2] = zCoord[i];
       case 2:
        row[1] = yCoord[i];
       case 1:
        row[0] = xCoord[i];
     }
    buffer.add_row_directly(row);
  }

  delete_ptr(xCoord);
  delete_ptr(yCoord);
  delete_ptr(zCoord);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_section(CRegion& parent_region)
{
  char section_name_char[CGNS_CHAR_MAX];
  
  // read section information
  cg_section_read(m_file.idx, m_base.idx, m_zone.idx, m_section.idx, section_name_char, &m_section.type,
                          &m_section.eBegin, &m_section.eEnd, &m_section.nbBdry, &m_section.parentFlag);
  m_section.name=section_name_char;
  
  // replace whitespace by underscore
  boost::algorithm::replace_all(m_section.name," ","_");

  // Create a new region for this section
  CRegion& this_region = parent_region.create_region(m_section.name);
  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");

  if (m_section.type == MIXED)
  {
    // Create CElements component for each element type.
    BufferMap buffer = create_element_regions_with_buffermap(this_region,coordinates,get_supported_element_types());
    
    // Handle each element of this section separately to see in which CElements component it will be written
    for (int elem=m_section.eBegin;elem<=m_section.eEnd;++elem)
    {
      // Read the amount of nodes this 1 element contains
      CALL_CGNS(cg_ElementPartialSize(m_file.idx,m_base.idx,m_zone.idx,m_section.idx,elem,elem,&m_section.elemNodeCount));
      m_section.elemNodeCount--; // subtract 1 as there is one index too many storing the element type

      // Storage for element type (index 0) and element nodes (index 1->elemNodeCount)
      int elemNodes[1][1+m_section.elemNodeCount];
      
      // Read nodes of 1 element
      CALL_CGNS(cg_elements_partial_read(m_file.idx,m_base.idx,m_zone.idx,m_section.idx,elem,elem,*elemNodes,&m_section.parentData));
      
      // Store the cgns element type
      ElementType_t etype_cgns = static_cast<ElementType_t>(elemNodes[0][0]);

      // Put the element nodes in a vector
      std::vector<Uint> row(m_section.elemNodeCount);
      for (int n=1;n<=m_section.elemNodeCount;++n)  // n=0 is the cell type
        row[n-1]=(elemNodes[0][n]-1); // -1 because cgns has index-base 1 instead of 0
      
      // Convert the cgns element type to the CF element type
      const std::string& etype_CF = m_elemtype_CGNS_to_CF[etype_cgns]+StringOps::to_str<int>(m_base.phys_dim)+"DLagrangeP1";

      // Add the nodes to the correct CElements component using its buffer
      Uint table_idx = buffer[etype_CF]->add_row(row);
      
      // Store the global element number to a pair of (region , local element number)
      m_global_to_region.push_back(Region_TableIndex_pair(get_named_component_typed_ptr<CElements>(this_region, etype_CF),table_idx));
    } // for elem
  } // if mixed
  else // Single element type in this section
  {

    // Read the number of nodes in this section
    CALL_CGNS(cg_npe(m_section.type,&m_section.elemNodeCount));

    // Read the size of all elements
    CALL_CGNS(cg_ElementDataSize(m_file.idx,m_base.idx,m_zone.idx,m_section.idx,&m_section.elemDataSize	));

    // Calculate the number of elements
    int nbElems = m_section.elemDataSize/m_section.elemNodeCount;

    // Convert the CGNS element type to the CF element type
    const std::string& etype_CF = m_elemtype_CGNS_to_CF[m_section.type]+StringOps::to_str<int>(m_base.phys_dim)+"DLagrangeP1";

    // Create element component in this region for this CF element type
    CElements& element_region = this_region.create_elements(etype_CF,coordinates);

    // Create a buffer for this element component, to start filling in the elements we will read.
    CTable::Buffer buffer = element_region.connectivity_table().create_buffer();

    // Create storage for element nodes
    int* elemNodes = new int [m_section.elemDataSize];
    
    // Read in the element nodes
    cg_elements_read	(m_file.idx,m_base.idx,m_zone.idx,m_section.idx, elemNodes,&m_section.parentData);

    // --------------------------------------------- Fill connectivity table
    std::vector<Uint> row(m_section.elemNodeCount);
    buffer.increase_array_size(nbElems);  // we can use increase_array_size + add_row_directly because we know apriori the change
    for (int elem=0; elem<nbElems; ++elem) //, ++progress)
    {
      for (int node=0;node<m_section.elemNodeCount;++node)
        row[node] = elemNodes[node+elem*m_section.elemNodeCount]-1; // -1 because cgns has index-base 1 instead of 0

      buffer.add_row_directly(row);
      
      // Store the global element number to a pair of (region , local element number)
      m_global_to_region.push_back(Region_TableIndex_pair(boost::dynamic_pointer_cast<CElements>(element_region.shared_from_this()),elem));
    } // for elem
    
    // Delete storage for element nodes
    delete_ptr(elemNodes);
  } // else not mixed

  remove_empty_element_regions(this_region);

  // Move components that are BC's in the "bc-regions" region
  if (option("SectionsAreBCs")->value<bool>())
  {
    bool is_bc_region = false;
    BOOST_FOREACH(const CRegion& region, recursive_range_typed<CRegion>(this_region))
    {
      if(range_typed<CRegion>(region).empty())  // if no other regions inside this region
      {
        if (get_component_typed<CElements>(region).element_type().dimensionality() < static_cast<Uint>(m_base.cell_dim))
        {
          is_bc_region = is_bc_region || true;
        }
      }
    }

    if (is_bc_region)
    {
      this_region.move_component( get_named_component_ptr( get_named_component(*m_mesh, "regions"), "bc-regions") );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_boco()
{

  // Read the info for this boundary condition.
  char boco_name_char[CGNS_CHAR_MAX];
  cg_boco_info(m_file.idx, m_base.idx, m_zone.idx, m_boco.idx, boco_name_char, &m_boco.boco_type, &m_boco.ptset_type,
               &m_boco.nBC_elem, &m_boco.normalIndex, &m_boco.normalListFlag, &m_boco.normalDataType, &m_boco.nDataSet);
  m_boco.name = boco_name_char;
  
  // replace whitespace by underscore
  boost::algorithm::replace_all(m_boco.name," ","_");

  // Read the element ID's
  int* boco_elems = new int [m_boco.nBC_elem];
  void* NormalList(NULL);
  CALL_CGNS(cg_boco_read(m_file.idx, m_base.idx, m_zone.idx, m_boco.idx, boco_elems, NormalList));

  // Create a region inside mesh/regions/bc-regions with the name of the cgns boco.
  CRegion& bc_region = get_named_component_typed<CRegion>(get_named_component(*m_mesh, "regions"), "bc-regions").create_region(m_boco.name);
  CArray::Ptr coordinates = get_named_component_typed_ptr<CArray>(*m_mesh, "coordinates");
  
  // Create CElements components for every possible element type supported.
  BufferMap buffer = create_element_regions_with_buffermap(bc_region,coordinates,get_supported_element_types());

  /// @todo The following code, only adds the full (volume) element as a bc element. Shouldn't this 
  /// be a Face element?
  switch (m_boco.ptset_type)
  {
    case ElementRange : // all bc elements are within a range given by 2 global element numbers
    {
      for (int global_element=boco_elems[0]-1;global_element<boco_elems[1];++global_element)
      {
        // Check which region this global_element belongs to
        CElements::Ptr region = m_global_to_region[global_element].first;
        
        // Check the local element number in this region
        Uint local_element = m_global_to_region[global_element].second;
        
        // Add the local element to the correct CElements component through its buffer
        buffer[region->name()]->add_row(region->connectivity_table().table()[local_element]);
      }
      break;
    }
    case ElementList : // all bc elements are listed as global element numbers
    {
      for (int i=0; i<m_boco.nBC_elem; ++i)
      {
        Uint global_element = boco_elems[i]-1;
        
        // Check which region this global_element belongs to
        CElements::Ptr region = m_global_to_region[global_element].first;
        Uint local_element = m_global_to_region[global_element].second;
        buffer[region->name()]->add_row(region->connectivity_table().table()[local_element]);
      }
      break;
    }
    case PointRange :  // see default
    case PointList :   // see default
    default :
      throw NotImplemented(FromHere(),"CGNS: no boundary with pointset_type " + StringOps::to_str<int>(m_boco.ptset_type) + " supported in CF yet"); 
  }

  // Flush all buffers and remove empty element regions
  for (BufferMap::iterator it=buffer.begin(); it!=buffer.end(); ++it)
    it->second->flush();

  remove_empty_element_regions(bc_region);

}

//////////////////////////////////////////////////////////////////////////////

Uint CReader::get_total_nbElements()
{
  // read sections (or subregions) in this zone
  Uint nbElements(0);
  for (m_section.idx=1; m_section.idx<=m_zone.nbSections; ++m_section.idx)
  {
    char section_name_char[CGNS_CHAR_MAX];
    CALL_CGNS(cg_section_read(m_file.idx, m_base.idx, m_zone.idx, m_section.idx, section_name_char, &m_section.type,
                            &m_section.eBegin, &m_section.eEnd, &m_section.nbBdry, &m_section.parentFlag));
    nbElements += m_section.eEnd - m_section.eBegin + 1;
  }
  return nbElements;
}

//////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // CF
