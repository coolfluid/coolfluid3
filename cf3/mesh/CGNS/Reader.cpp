// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include <boost/progress.hpp>

#include "common/BoostFilesystem.hpp"

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/FindComponents.hpp"
#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/Space.hpp"
#include "mesh/MeshTransformer.hpp"

#include "mesh/CGNS/Reader.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace CGNS {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder< Reader, MeshReader, LibCGNS > aCGNSReader_Builder;

//////////////////////////////////////////////////////////////////////////////

Reader::Reader(const std::string& name)
: MeshReader(name), Shared()
{
  options().add_option( "SectionsAreBCs", false )
      .description("Treat Sections of lower dimensionality as BC. "
                        "This means no BCs from cgns will be read");

}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Reader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".cgns");
  return extensions;
}

//////////////////////////////////////////////////////////////////////////////

void Reader::do_read_mesh_into(const URI& file, Mesh& mesh)
{
  // Set the internal mesh pointer
  m_mesh = Handle<Mesh>(mesh.handle<Component>());

  // open file in read mode
  CALL_CGNS(cg_open(file.path().c_str(),CG_MODE_READ,&m_file.idx));

  // check how many bases we have
  CALL_CGNS(cg_nbases(m_file.idx,&m_file.nbBases));

  // Store if there is only 1 base
  m_base.unique = m_file.nbBases==1 ? true : false;

  // Read every base (usually there is only 1)
  for (m_base.idx = 1; m_base.idx<=m_file.nbBases; ++m_base.idx)
    read_base(*m_mesh);

  // close the CGNS file
  CALL_CGNS(cg_close(m_file.idx));

  m_mesh->elements().update();
  m_mesh->update_statistics();

  // Fix global numbering
  /// @todo remove this and read glb_index ourself
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering")->transform(m_mesh);
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_base(Mesh& parent_region)
{

  // get the name, dimension and physical dimension from the base
  char base_name_char[CGNS_CHAR_MAX];
  CALL_CGNS(cg_base_read(m_file.idx,m_base.idx,base_name_char,&m_base.cell_dim,&m_base.phys_dim));
  m_base.name=base_name_char;
  boost::algorithm::replace_all(m_base.name," ","_");
  boost::algorithm::replace_all(m_base.name,".","_");

  // Create basic region structure
  Region& base_region = m_mesh->topology();
  m_base_map[m_base.idx] = &base_region;

  // check how many zones we have
  CALL_CGNS(cg_nzones(m_file.idx,m_base.idx,&m_base.nbZones));
  m_zone.unique = m_base.nbZones == 1 ? true : false;
  // Read every zone in this base
  for (m_zone.idx = 1; m_zone.idx<=m_base.nbZones; ++m_zone.idx)
    read_zone(base_region);

}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_zone(Region& parent_region)
{
  // get zone type (Structured or Unstructured)
  CALL_CGNS(cg_zone_type(m_file.idx,m_base.idx,m_zone.idx,&m_zone.type));

  // For now only Unstructured and Structured zone types are supported
  if (m_zone.type != Structured && m_zone.type != Unstructured)
    throw NotImplemented (FromHere(),"Only Unstructured and Structured zone types are supported");

  // Read zone size and name
  if (m_zone.type == Unstructured)
  {
    int size[3][1];
    char zone_name_char[CGNS_CHAR_MAX];
    CALL_CGNS(cg_zone_read(m_file.idx,m_base.idx,m_zone.idx,zone_name_char,size[0]));
    m_zone.name = zone_name_char;
    boost::algorithm::replace_all(m_zone.name," ","_");
    boost::algorithm::replace_all(m_zone.name,".","_");

    m_zone.total_nbVertices = size[CGNS_VERT_IDX][0];
    m_zone.nbElements       = size[CGNS_CELL_IDX][0];
    m_zone.nbBdryVertices   = size[CGNS_BVRT_IDX][0];



    // get the number of grids
    CALL_CGNS(cg_ngrids(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbGrids));
    // nb coord dims
    CALL_CGNS(cg_ncoords(m_file.idx,m_base.idx,m_zone.idx, &m_zone.coord_dim));
    // find out number of solutions
    CALL_CGNS(cg_nsols(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbSols));
    // find out how many sections
    CALL_CGNS(cg_nsections(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbSections));
    m_section.unique = m_zone.nbSections == 1 ? true : false;
    // find out number of BCs that exist under this zone
    CALL_CGNS(cg_nbocos(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbBocos));
    m_boco.unique = m_zone.nbBocos == 1 ? true : false;
    // Add up all the nb elements from all sections
    m_zone.total_nbElements = get_total_nbElements();


    // Create a region for this zone if there is more than one
    //Region& this_region = m_zone.unique? parent_region : parent_region.create_region(m_zone.name);
    //this_region.add_tag("grid_zone");

    Region& this_region = parent_region.create_region(m_zone.name);
    this_region.add_tag("grid_zone");
    m_zone_map[m_zone.idx] = &this_region;

    // read coordinates in this zone
    for (int i=1; i<=m_zone.nbGrids; ++i)
      read_coordinates_unstructured(this_region);

    // read sections (or subregions) in this zone
    m_global_to_region.reserve(m_zone.total_nbElements);
    for (m_section.idx=1; m_section.idx<=m_zone.nbSections; ++m_section.idx)
      read_section(this_region);

//    // Only read boco's if sections are not defined as BC's
//    if (!option("SectionsAreBCs")->value<bool>())
//    {
      // read boundaryconditions (or subregions) in this zone
      for (m_boco.idx=1; m_boco.idx<=m_zone.nbBocos; ++m_boco.idx)
        read_boco_unstructured(this_region);
//
//      // Remove regions flagged as bc
//      BOOST_FOREACH(Region& region, find_components_recursively_with_tag<Region>(this_region,"remove_this_tmp_component"))
//      {
//        region.parent().remove_component(region.name());
//      }
//    }

    // Cleanup:

    // truely deallocate the global_to_region vector
    m_global_to_region.resize(0);
    std::vector<Region_TableIndex_pair>().swap (m_global_to_region);




  }
  else if(m_zone.type == Structured)
  {
    int isize[3][3];
    char zone_name_char[CGNS_CHAR_MAX];
    CALL_CGNS(cg_zone_read(m_file.idx,m_base.idx,m_zone.idx,zone_name_char,isize[0]));
    m_zone.name = zone_name_char;
    boost::algorithm::replace_all(m_zone.name," ","_");
    boost::algorithm::replace_all(m_zone.name,".","_");

    m_zone.nbVertices[XX] = isize[CGNS_VERT_IDX][XX];
    m_zone.nbVertices[YY] = isize[CGNS_VERT_IDX][YY];
    m_zone.nbVertices[ZZ] = isize[CGNS_VERT_IDX][ZZ];

    // m_zone.nbElements     = size[CGNS_CELL_IDX];
    // m_zone.nbBdryVertices = size[CGNS_BVRT_IDX];

    // get the number of grids
    CALL_CGNS(cg_ngrids(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbGrids));
    // nb coord dims
    CALL_CGNS(cg_ncoords(m_file.idx,m_base.idx,m_zone.idx, &m_zone.coord_dim));
    m_zone.total_nbVertices = 1;
    for (int d=0; d<m_zone.coord_dim; ++d)
      m_zone.total_nbVertices *= m_zone.nbVertices[d];

    // find out number of solutions
    CALL_CGNS(cg_nsols(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbSols));
    // find out how many sections
    CALL_CGNS(cg_nsections(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbSections));
    m_section.unique = m_zone.nbSections == 1 ? true : false;
    // find out number of BCs that exist under this zone
    CALL_CGNS(cg_nbocos(m_file.idx,m_base.idx,m_zone.idx,&m_zone.nbBocos));
    m_boco.unique = m_zone.nbBocos == 1 ? true : false;
    // Add up all the nb elements from all sections
    m_zone.total_nbElements = get_total_nbElements();

    // Create a region for this zone
    Region& this_region = parent_region.create_region(m_zone.name);
    this_region.add_tag("grid_zone");
    m_zone_map[m_zone.idx] = &this_region;

    // read coordinates in this zone
    for (int i=1; i<=m_zone.nbGrids; ++i)
      read_coordinates_structured(this_region);

    create_structured_elements(this_region);

    // read boundaryconditions (or subregions) in this zone
    for (m_boco.idx=1; m_boco.idx<=m_zone.nbBocos; ++m_boco.idx)
      read_boco_structured(this_region);

  }





}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_coordinates_unstructured(Region& parent_region)
{

  CFinfo << "creating coordinates in " << parent_region.uri().string() << CFendl;

  Dictionary& nodes = m_mesh->geometry_fields();
  m_zone.nodes = &nodes;
  m_zone.nodes_start_idx = nodes.size();

  // read coordinates
  int one = 1;
  Real *xCoord;
  Real *yCoord;
  Real *zCoord;
  switch (m_zone.coord_dim)
  {
    case 3:
      zCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateZ", RealDouble, &one, &m_zone.total_nbVertices, zCoord));
    case 2:
      yCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateY", RealDouble, &one, &m_zone.total_nbVertices, yCoord));
    case 1:
      xCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateX", RealDouble, &one, &m_zone.total_nbVertices, xCoord));
  }

  m_mesh->initialize_nodes(m_zone.total_nbVertices, (Uint)m_zone.coord_dim);

  common::Table<Real>& coords = nodes.coordinates();
  common::List<Uint>& rank = nodes.rank();
  //  common::Table<Real>::Buffer buffer = nodes.coordinates().create_buffer();
  //buffer.increase_array_size(m_zone.total_nbVertices);
  //std::vector<Real> row(m_zone.coord_dim);
  for (int i=0; i<m_zone.total_nbVertices; ++i)
  {
    switch (m_zone.coord_dim)
    {
      case 3:
        coords[i][2] = zCoord[i];
       case 2:
        coords[i][1] = yCoord[i];
       case 1:
        coords[i][0] = xCoord[i];
     }
    rank[i] = 0;
  }

  switch (m_zone.coord_dim)
  {
    case 3:
      delete_ptr_array(zCoord);
    case 2:
      delete_ptr_array(yCoord);
    case 1:
      delete_ptr_array(xCoord);
  }

}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_coordinates_structured(Region& parent_region)
{
  Dictionary& nodes = m_mesh->geometry_fields();
  m_zone.nodes = &nodes;
  m_zone.nodes_start_idx = nodes.size();

  int one[3];
  one[0]= 1;
  one[1]= 1;
  one[2]= 1;

  // read coordinates
  Real *xCoord = nullptr;
  Real *yCoord = nullptr;
  Real *zCoord = nullptr;
  switch (m_zone.coord_dim)
  {
    case 3:
      zCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateZ", RealDouble, one, m_zone.nbVertices, zCoord));
    case 2:
      yCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateY", RealDouble, one, m_zone.nbVertices, yCoord));
    case 1:
      xCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateX", RealDouble, one, m_zone.nbVertices, xCoord));
  }

  common::Table<Real>& coords = nodes.coordinates();
  m_mesh->initialize_nodes(m_zone.total_nbVertices,m_zone.coord_dim);
  Uint n(0);
  switch (m_zone.coord_dim)
  {
    case 3:
      for (int k=0; k<m_zone.nbVertices[ZZ]; ++k)
        for (int j=0; j<m_zone.nbVertices[YY]; ++j)
          for (int i=0; i<m_zone.nbVertices[XX]; ++i)
          {
            common::Table<Real>::Row row = coords[n++];
            row[0] = xCoord[structured_node_idx(i,j,k)];
            row[1] = yCoord[structured_node_idx(i,j,k)];
            row[2] = zCoord[structured_node_idx(i,j,k)];
          }
      break;
    case 2:
      for (int j=0; j<m_zone.nbVertices[YY]; ++j)
        for (int i=0; i<m_zone.nbVertices[XX]; ++i)
        {
          common::Table<Real>::Row row = coords[n++];
          row[0] = xCoord[structured_node_idx(i,j,0)];
          row[1] = yCoord[structured_node_idx(i,j,0)];
        }
      break;

    case 1:
      for (int i=0; i<m_zone.nbVertices[XX]; ++i)
      {
        common::Table<Real>::Row row = coords[n++];
        row[0] = xCoord[i];
      }
      break;
  }

  delete_ptr_array(xCoord);
  delete_ptr_array(yCoord);
  delete_ptr_array(zCoord);
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_section(Region& parent_region)
{

  char section_name_char[CGNS_CHAR_MAX];

  // read section information
  cg_section_read(m_file.idx, m_base.idx, m_zone.idx, m_section.idx, section_name_char, &m_section.type,
                          &m_section.eBegin, &m_section.eEnd, &m_section.nbBdry, &m_section.parentFlag);
  m_section.name=section_name_char;

  // replace whitespace by underscore
  boost::algorithm::replace_all(m_section.name," ","_");
  boost::algorithm::replace_all(m_section.name,".","_");

  BOOST_FOREACH(Region& existing_region, find_components<Region>(parent_region))
  if (follow_link(existing_region))
  {
    if (existing_region.properties().check("cgns_section_name"))
    {
      if (existing_region.properties().value<std::string>("cgns_section_name") == m_section.name)
      {
        existing_region.rename(properties().value<std::string>("cgns_section_name"));
        existing_region.properties()["previous_elem_count"] = existing_region.recursive_elements_count(true);
        break;
      }
    }
  }

  // Create a new region for this section
  Region& this_region = parent_region.create_region(m_section.name);

  Dictionary& all_nodes = *m_zone.nodes;
  Uint start_idx = m_zone.nodes_start_idx;

  if (m_section.type == MIXED)
  {
    // Create Elements component for each element type.
    std::map<std::string,Handle< Elements > > elements = create_cells_in_region(this_region,all_nodes,get_supported_element_types());
    std::map<std::string, boost::shared_ptr< ArrayBufferT<Uint> > > buffer = create_connectivity_buffermap(elements);

    // Handle each element of this section separately to see in which Elements component it will be written
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
        row[n-1]=start_idx+(elemNodes[0][n]-1); // -1 because cgns has index-base 1 instead of 0

      // Convert the cgns element type to the CF element type
      const std::string& etype_CF = m_elemtype_CGNS_to_CF[etype_cgns]+to_str<int>(m_base.phys_dim)+"D";

      // Add the nodes to the correct Elements component using its buffer
      Uint table_idx = buffer[etype_CF]->add_row(row);

      // Store the global element number to a pair of (region , local element number)
      m_global_to_region.push_back(Region_TableIndex_pair(find_component_ptr_with_name<Elements>(this_region, etype_CF),table_idx));
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
    const std::string& etype_CF = m_elemtype_CGNS_to_CF[m_section.type]+to_str<int>(m_base.phys_dim)+"D";

    // Create element component in this region for this CF element type, automatically creates connectivity_table
    this_region.create_elements(etype_CF,all_nodes);

    Elements& element_region= *Handle<Elements>(this_region.get_child("elements_"+etype_CF));

    // Create a buffer for this element component, to start filling in the elements we will read.
    Connectivity& node_connectivity = element_region.geometry_space().connectivity();

    // Create storage for element nodes
    int* elemNodes = new int [m_section.elemDataSize];

    // Read in the element nodes
    cg_elements_read	(m_file.idx,m_base.idx,m_zone.idx,m_section.idx, elemNodes,&m_section.parentData);

    // --------------------------------------------- Fill connectivity table
    std::vector<Uint> coords_added;
    std::vector<Uint> row(m_section.elemNodeCount);
    node_connectivity.resize(nbElems);

    for (int elem=0; elem<nbElems; ++elem) //, ++progress)
    {
      for (int node=0;node<m_section.elemNodeCount;++node)
        node_connectivity[elem][node] = start_idx + elemNodes[node+elem*m_section.elemNodeCount]-1;  // -1 because cgns has index-base 1 instead of 0;

      // Store the global element number to a pair of (region , local element number)
      m_global_to_region.push_back(Region_TableIndex_pair(element_region.handle<Elements>(),elem));
    } // for elem



    // Delete storage for element nodes
    delete_ptr(elemNodes);
  } // else not mixed

  remove_empty_element_regions(this_region);

//  // Mark BC regions as temporary if option SectionsAreBCs is false
//  if (!option("SectionsAreBCs")->value<bool>() && option("SharedCoordinates")->value<bool>())
//  {
//    bool is_bc_region = false;
//    BOOST_FOREACH(const Elements& element_region, find_components<Elements>(this_region))
//    {
//      if (element_region.element_type().dimensionality() < static_cast<Uint>(m_base.cell_dim))
//        is_bc_region = is_bc_region || true;
//    }
//    if (is_bc_region)
//      this_region.add_tag("remove_this_tmp_component");
//  }
}

//////////////////////////////////////////////////////////////////////////////

void Reader::create_structured_elements(Region& parent_region)
{
  Dictionary& nodes = *m_zone.nodes;

  std::string etype_CF;
  switch (m_base.cell_dim)
  {
    case 3: // Hexahedrons
      etype_CF = "cf3.mesh.LagrangeP1.Hexa"+to_str<int>(m_base.phys_dim)+"D";
      break;
    case 2: // Quadrilaterals
      etype_CF = "cf3.mesh.LagrangeP1.Quad"+to_str<int>(m_base.phys_dim)+"D";
      break;
    case 1: // Segments
      etype_CF = "cf3.mesh.LagrangeP1.Line"+to_str<int>(m_base.phys_dim)+"D";
    default:
      break;
  }

  Region& this_region = parent_region.create_region("Inner");
  Elements& element_region = this_region.create_elements(etype_CF,nodes);

  // Create a buffer for this element component, to start filling in the elements we will read.
  Connectivity& node_connectivity = element_region.geometry_space().connectivity();
  node_connectivity.resize(m_zone.total_nbElements);
  // --------------------------------------------- Fill connectivity table


  switch (m_base.cell_dim)
  {
    case 3: // Hexahedrons
    {
      Uint e(0);
      for (int k=0; k<m_zone.nbVertices[ZZ]-1; ++k)
        for (int j=0; j<m_zone.nbVertices[YY]-1; ++j)
          for (int i=0; i<m_zone.nbVertices[XX]-1; ++i)
          {
            Connectivity::Row row = node_connectivity[e++];
            row[0] = structured_node_idx(i  ,j  ,k  );
            row[1] = structured_node_idx(i+1,j  ,k  );
            row[2] = structured_node_idx(i+1,j+1,k  );
            row[3] = structured_node_idx(i  ,j+1,k  );
            row[4] = structured_node_idx(i  ,j  ,k+1);
            row[5] = structured_node_idx(i+1,j  ,k+1);
            row[6] = structured_node_idx(i+1,j+1,k+1);
            row[7] = structured_node_idx(i  ,j+1,k+1);
          }
      break;
    }
    case 2: // Quadrilaterals
    {
    Uint e(0);
      int k=0;
      for (int j=0; j<m_zone.nbVertices[YY]-1; ++j)
        for (int i=0; i<m_zone.nbVertices[XX]-1; ++i)
          {
            Connectivity::Row row = node_connectivity[e++];
            row[0] = structured_node_idx(i  ,j  ,k  );
            row[1] = structured_node_idx(i+1,j  ,k  );
            row[2] = structured_node_idx(i+1,j+1,k  );
            row[3] = structured_node_idx(i  ,j+1,k  );
          }
      break;
    }
    case 1: // Segments
    {
    Uint e(0);
      int j=0, k=0;
      for (int i=0; i<m_zone.nbVertices[XX]-1; ++i)
      {
        Connectivity::Row row = node_connectivity[e++];
        row[0] = structured_node_idx(i  ,j  ,k  );
        row[1] = structured_node_idx(i+1,j  ,k  );
      }
      break;
    }
    default:
      break;
  }

  //remove_empty_element_regions(this_region);
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_boco_unstructured(Region& parent_region)
{

  // Read the info for this boundary condition.
  char boco_name_char[CGNS_CHAR_MAX];
  cg_boco_info(m_file.idx, m_base.idx, m_zone.idx, m_boco.idx, boco_name_char, &m_boco.boco_type, &m_boco.ptset_type,
               &m_boco.nBC_elem, &m_boco.normalIndex, &m_boco.normalListFlag, &m_boco.normalDataType, &m_boco.nDataSet);
  m_boco.name = boco_name_char;

  // replace whitespace by underscore
  boost::algorithm::replace_all(m_boco.name," ","_");
  boost::algorithm::replace_all(m_boco.name,".","_");

  // Read the element ID's
  int* boco_elems = new int [m_boco.nBC_elem];
  void* NormalList(NULL);
  CALL_CGNS(cg_boco_read(m_file.idx, m_base.idx, m_zone.idx, m_boco.idx, boco_elems, NormalList));

  switch (m_boco.ptset_type)
  {
    case PointRange:
    case ElementRange : // all bc elements are within a range given by 2 global element numbers
    {
      if (m_zone.type != Unstructured)
        throw NotSupported(FromHere(),"CGNS: Boundary with pointset_type \"ElementRange\" is only supported for Unstructured grids");

      // First do some simple checks to see if an entire region can be taken as a BC.
      Handle< Elements > first_elements = m_global_to_region[boco_elems[0]-1].first;
      Handle< Elements > last_elements = m_global_to_region[boco_elems[1]-1].first;
      if (first_elements->parent() == last_elements->parent())
      {
        Handle< Region > group_region = Handle<Region>(first_elements->parent());
        Uint prev_elm_count = group_region->properties().check("previous_elem_count") ? group_region->properties().value<Uint>("previous_elem_count") : 0;
        if (group_region->recursive_elements_count(true) == prev_elm_count + Uint(boco_elems[1]-boco_elems[0]+1))
        {
          group_region->properties()["cgns_section_name"] = group_region->name();
          group_region->rename(m_boco.name);
          break;
        }
      }


      // Create a region inside mesh/regions/bc-regions with the name of the cgns boco.
      Region& this_region = parent_region.create_region(m_boco.name);
      Dictionary& nodes = *m_zone.nodes;

      // Create Elements components for every possible element type supported.
      std::map<std::string,Handle< Elements > > elements = create_faces_in_region(this_region,nodes,get_supported_element_types());
      std::map<std::string,boost::shared_ptr< ArrayBufferT<Uint > > > buffer = create_connectivity_buffermap(elements);

      for (int global_element=boco_elems[0]-1;global_element<boco_elems[1];++global_element)
      {
        // Check which region this global_element belongs to
        Handle< Elements > element_region = m_global_to_region[global_element].first;

        // Check the local element number in this region
        Uint local_element = m_global_to_region[global_element].second;

        // Add the local element to the correct Elements component through its buffer
        buffer[element_region->element_type().derived_type_name()]->add_row(element_region->geometry_space().connectivity()[local_element]);
      }

      // Flush all buffers and remove empty element regions
      for (BufferMap::iterator it=buffer.begin(); it!=buffer.end(); ++it)
        it->second->flush();
      buffer.clear();

      remove_empty_element_regions(this_region);
      break;
    }
    case PointList:
    case ElementList : // all bc elements are listed as global element numbers
    {
      if (m_zone.type != Unstructured)
        throw NotSupported(FromHere(),"CGNS: Boundary with pointset_type \"ElementList\" is only supported for Unstructured grids");

      // First do some simple checks to see if an entire region can be taken as a BC.
      Handle< Elements > first_elements = m_global_to_region[boco_elems[0]-1].first;
      Handle< Elements > last_elements = m_global_to_region[boco_elems[m_boco.nBC_elem-1]-1].first;
      if (first_elements->parent() == last_elements->parent())
      {
        Handle< Region > group_region = Handle<Region>(first_elements->parent());
        Uint prev_elm_count = group_region->properties().check("previous_elem_count") ? group_region->properties().value<Uint>("previous_elem_count") : 0;
        if (group_region->recursive_elements_count(true) == prev_elm_count + Uint(boco_elems[m_boco.nBC_elem-1]-boco_elems[0]+1))
        {
          group_region->properties()["cgns_section_name"] = group_region->name();
          group_region->rename(m_boco.name);
          break;
        }
      }

      // Create a region inside mesh/regions/bc-regions with the name of the cgns boco.
      Region& this_region = parent_region.create_region(m_boco.name);
      Dictionary& nodes = *m_zone.nodes;

      // Create Elements components for every possible element type supported.
      std::map<std::string,Handle< Elements > > elements = create_faces_in_region(this_region,nodes,get_supported_element_types());
      std::map<std::string,boost::shared_ptr< ArrayBufferT<Uint > > > buffer = create_connectivity_buffermap(elements);

      for (int i=0; i<m_boco.nBC_elem; ++i)
      {
        Uint global_element = boco_elems[i]-1;

        // Check which region this global_element belongs to
        Handle< Elements > element_region = m_global_to_region[global_element].first;

        // Check the local element number in this region
        Uint local_element = m_global_to_region[global_element].second;

        // Add the local element to the correct Elements component through its buffer
        buffer[element_region->element_type().derived_type_name()]->add_row(element_region->geometry_space().connectivity()[local_element]);
      }

      // Flush all buffers and remove empty element regions
      for (BufferMap::iterator it=buffer.begin(); it!=buffer.end(); ++it)
        it->second->flush();
      buffer.clear();

      remove_empty_element_regions(this_region);

      break;
    }
//    case PointRange : // bc elements are given by node index range
//    throw NotSupported(FromHere(),"CGNS: Boundary "+m_boco.name+" with pointset_type \"PointRange\" (="+to_str<int>(PointRange)+") is only supported for Structured grids");
//    case PointList : // bc elements are given by node index list
//    throw NotSupported(FromHere(),"CGNS: Boundary "+m_boco.name+"  with pointset_type \"PointList\" (="+to_str<int>(PointRange)+") is only supported for Structured grids");
    default :
      throw NotImplemented(FromHere(),"CGNS: pointset_type " + to_str<int>(m_boco.ptset_type) + " for boundary "+m_boco.name+" not supported in CF yet");
  }

  delete_ptr(boco_elems);
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_boco_structured(Region& parent_region)
{

  // Read the info for this boundary condition.
  char boco_name_char[CGNS_CHAR_MAX];
  cg_boco_info(m_file.idx, m_base.idx, m_zone.idx, m_boco.idx, boco_name_char, &m_boco.boco_type, &m_boco.ptset_type,
               &m_boco.nBC_elem, &m_boco.normalIndex, &m_boco.normalListFlag, &m_boco.normalDataType, &m_boco.nDataSet);
  m_boco.name = boco_name_char;

  // replace whitespace by underscore
  boost::algorithm::replace_all(m_boco.name," ","_");
  boost::algorithm::replace_all(m_boco.name,".","_");

  // Create a region inside mesh/regions/bc-regions with the name of the cgns boco.
  Region& this_region = parent_region.create_region(m_boco.name);
  Dictionary& nodes = *m_zone.nodes;

  // Which BC_element type will we need?
  std::string etype_CF;
  std::string etypeBC_CF;
  switch (m_base.cell_dim)
  {
    case 3: // Hexahedrons
      etype_CF = "cf3.mesh.LagrangeP1.Hexa"+to_str<int>(m_base.phys_dim)+"D";
      etypeBC_CF = "cf3.mesh.LagrangeP1.Quad"+to_str<int>(m_base.phys_dim)+"D";
      break;
    case 2: // Quadrilaterals
      etype_CF = "cf3.mesh.LagrangeP1.Quad"+to_str<int>(m_base.phys_dim)+"D";
      etypeBC_CF = "cf3.mesh.LagrangeP1.Line"+to_str<int>(m_base.phys_dim)+"D";
      break;
    case 1: // Segments
      etype_CF = "cf3.mesh.LagrangeP1.Line"+to_str<int>(m_base.phys_dim)+"D";
      etypeBC_CF = "cf3.mesh.LagrangeP1.Point"+to_str<int>(m_base.phys_dim)+"D";
    default:
      break;
  }

  Elements& elements = this_region.create_elements(etypeBC_CF,nodes);
  //common::Table<Uint>& source_elements = parent_region.get_child("Inner")->get_child_ptr<Elements>("elements_"+etype_CF)->geometry_space().connectivity();
  Connectivity& node_connectivity = elements.geometry_space().connectivity();



  // Read the Node ID's
  int* boco_elems = new int [m_boco.nBC_elem*m_base.cell_dim];
  void* NormalList(NULL);
  CALL_CGNS(cg_boco_read(m_file.idx, m_base.idx, m_zone.idx, m_boco.idx, boco_elems, NormalList));


  switch (m_boco.ptset_type)
  {
    case ElementRange : // all bc elements are within a range given by 2 global element numbers
      throw NotSupported(FromHere(),"CGNS: Boundary with pointset_type \"ElementRange\" is only supported for Unstructured grids");
    case ElementList : // all bc elements are listed as global element numbers
      throw NotSupported(FromHere(),"CGNS: Boundary with pointset_type \"ElementList\" is only supported for Unstructured grids");
    case PointRange : // bc elements are given by node index range
    {
      int imin(0), jmin(0), kmin(0);
      int imax(0), jmax(0), kmax(0);

      imin=boco_elems[XX+0*m_base.cell_dim]-1; imax=boco_elems[XX+1*m_base.cell_dim]-1;
      jmin=boco_elems[YY+0*m_base.cell_dim]-1; jmax=boco_elems[YY+1*m_base.cell_dim]-1;
      kmin=boco_elems[ZZ+0*m_base.cell_dim]-1; kmax=boco_elems[ZZ+1*m_base.cell_dim]-1;

      if (imin == imax) // i = constant plane
      {
        m_boco.nBC_elem = (jmax-jmin)*(kmax-kmin);
        node_connectivity.resize(m_boco.nBC_elem);
        Uint e(0);
        if (imin == 0)
          for (int k=kmin; k<kmax; ++k)
            for (int j=jmin; j<jmax; ++j)
            {
              Connectivity::Row row = node_connectivity[e++];
              row[0] = structured_node_idx(imin, j  , k  );
              row[1] = structured_node_idx(imin, j  , k+1);
              row[2] = structured_node_idx(imin, j+1, k+1);
              row[3] = structured_node_idx(imin, j+1, k  );
            }
        else
          for (int k=kmin; k<kmax; ++k)
            for (int j=jmin; j<jmax; ++j)
            {
              Connectivity::Row row = node_connectivity[e++];
              row[0] = structured_node_idx(imax, j  ,k  );
              row[1] = structured_node_idx(imax ,j+1,k  );
              row[2] = structured_node_idx(imax, j+1,k+1);
              row[3] = structured_node_idx(imax, j  ,k+1);
            }
      }

      if (jmin == jmax) // j = constant plane
      {
        m_boco.nBC_elem = (imax-imin)*(kmax-kmin);
        node_connectivity.resize(m_boco.nBC_elem);
        Uint e(0);
        if (jmin == 0)
          for (int k=kmin; k<kmax; ++k)
            for (int i=imin; i<imax; ++i)
            {
              Connectivity::Row row = node_connectivity[e++];
              row[0] = structured_node_idx(i  , jmin, k  );
              row[1] = structured_node_idx(i+1, jmin, k  );
              row[2] = structured_node_idx(i+1, jmin, k+1);
              row[3] = structured_node_idx(i  , jmin, k+1);
            }
        else
          for (int k=kmin; k<kmax; ++k)
            for (int i=imin; i<imax; ++i)
            {
              Connectivity::Row row = node_connectivity[e++];
              row[0] = structured_node_idx(i  ,jmax, k  );
              row[1] = structured_node_idx(i  ,jmax, k+1);
              row[2] = structured_node_idx(i+1,jmax, k+1);
              row[3] = structured_node_idx(i+1,jmax, k  );
            }
      }

      if (kmin == kmax) // k = constant plane
      {
        m_boco.nBC_elem = (imax-imin)*(jmax-jmin);
        node_connectivity.resize(m_boco.nBC_elem);
        Uint e(0);
        if (kmin == 0)
          for (int i=imin; i<imax; ++i)
            for (int j=jmin; j<jmax; ++j)
            {
              Connectivity::Row row = node_connectivity[e++];
              row[0] = structured_node_idx(i  ,j  , kmin  );
              row[1] = structured_node_idx(i  ,j+1, kmin  );
              row[2] = structured_node_idx(i+1,j+1, kmin  );
              row[3] = structured_node_idx(i+1,j  , kmin  );
            }
        else
          for (int i=imin; i<imax; ++i)
            for (int j=jmin; j<jmax; ++j)
            {
              Connectivity::Row row = node_connectivity[e++];
              row[0] = structured_node_idx(i  ,j  , kmax);
              row[1] = structured_node_idx(i+1,j  , kmax);
              row[2] = structured_node_idx(i+1,j+1, kmax);
              row[3] = structured_node_idx(i  ,j+1, kmax);
            }
      }
      delete_ptr_array(boco_elems);

      break;
    }
    case PointList : // bc elements are given by node index list
    default :
      throw NotImplemented(FromHere(),"CGNS: no boundary with pointset_type " + to_str<int>(m_boco.ptset_type) + " supported in CF yet");
  }


  remove_empty_element_regions(this_region);

}

//////////////////////////////////////////////////////////////////////////////

Uint Reader::get_total_nbElements()
{
  Uint nbElements = 0;

  if (m_zone.type == Unstructured)
  {
    nbElements=0;
    // read sections (or subregions) in this zone
    for (m_section.idx=1; m_section.idx<=m_zone.nbSections; ++m_section.idx)
    {
      char section_name_char[CGNS_CHAR_MAX];
      CALL_CGNS(cg_section_read(m_file.idx, m_base.idx, m_zone.idx, m_section.idx, section_name_char, &m_section.type,
                                &m_section.eBegin, &m_section.eEnd, &m_section.nbBdry, &m_section.parentFlag));
      nbElements += m_section.eEnd - m_section.eBegin + 1;
    }
  }
  else if (m_zone.type == Structured)
  {
    nbElements=1;
    for (int d=0; d<m_zone.coord_dim; ++d)
      nbElements *= m_zone.nbVertices[d]-1;
  }

  return nbElements;
}


//////////////////////////////////////////////////////////////////////////////

} // CGNS
} // mesh
} // cf3
