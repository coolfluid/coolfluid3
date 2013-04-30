// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
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

#include "math/VariablesDescriptor.hpp"

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
  options().add( "SectionsAreBCs", false )
      .description("Treat Sections of lower dimensionality as BC. "
                        "This means no BCs from cgns will be read");
  options().add( "zone_handling", false )
      .description("If zero, and there is only 1 zone, the zone is skipped"
                   " as nested region, and the zone's sections are added immediately.");
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
  m_mesh = Handle<Mesh>(mesh.handle());

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

  // Fix global numbering
  /// @todo remove this and read glb_index ourself
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering")->transform(m_mesh);

  mesh.raise_mesh_loaded();
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
  boost::algorithm::replace_all(m_base.name,":","_");
  boost::algorithm::replace_all(m_base.name,"/","_");
  
  if ( options().value<Uint>("dimension") != 0 )
  {
    m_base.phys_dim = options().value<Uint>("dimension");     
  }
  
  // Create basic region structure
//  Region& base_region = m_mesh->topology();
//  m_base_map[m_base.idx] = &base_region;

  // check how many zones we have
  CALL_CGNS(cg_nzones(m_file.idx,m_base.idx,&m_base.nbZones));
  m_zone.unique = m_base.nbZones == 1 ? true : false;
  // Read every zone in this base
  for (m_zone.idx = 1; m_zone.idx<=m_base.nbZones; ++m_zone.idx)
    read_zone(parent_region);

}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_zone(Mesh& mesh)
{
  // get zone type (CGNS_ENUMV( Structured ) or CGNS_ENUMV( Unstructured ))
  CALL_CGNS(cg_zone_type(m_file.idx,m_base.idx,m_zone.idx,&m_zone.type));

  // For now only CGNS_ENUMV( Unstructured ) and CGNS_ENUMV( Structured ) zone types are supported
  if (m_zone.type != CGNS_ENUMV( Structured ) && m_zone.type != CGNS_ENUMV( Unstructured ))
    throw NotImplemented (FromHere(),"Only CGNS_ENUMV( Unstructured ) and CGNS_ENUMV( Structured ) zone types are supported");

  // Read zone size and name
  if (m_zone.type == CGNS_ENUMV( Unstructured ))
  {
    cgsize_t size[3][1];
    char zone_name_char[CGNS_CHAR_MAX];
    CALL_CGNS(cg_zone_read(m_file.idx,m_base.idx,m_zone.idx,zone_name_char,size[0]));
    m_zone.name = zone_name_char;
    boost::algorithm::replace_all(m_zone.name," ","_");
    boost::algorithm::replace_all(m_zone.name,".","_");
    boost::algorithm::replace_all(m_zone.name,":","_");
    boost::algorithm::replace_all(m_zone.name,"/","_");
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

    // Coordinate dimension is reduced to mesh dimension, as configured by
    // options().value<Uint>("dimension")
    if( m_base.phys_dim < m_zone.coord_dim )
      m_zone.coord_dim = m_base.phys_dim;

    // Create a region for this zone
    Handle<Region> this_region;
    if (m_zone.unique && options().value<bool>("zone_handling") == false)
    {
      this_region = mesh.topology().handle<Region>(); 
    }
    else
    {
      this_region = mesh.topology().create_region(m_zone.name).handle<Region>();
    }
    
    this_region->add_tag("grid_zone");
    m_zone_map[m_zone.idx] = this_region.get();

    // read coordinates in this zone
    for (int i=1; i<=m_zone.nbGrids; ++i)
      read_coordinates_unstructured(*this_region);

    // read sections (or subregions) in this zone
    m_global_to_region.reserve(m_zone.total_nbElements);
    for (m_section.idx=1; m_section.idx<=m_zone.nbSections; ++m_section.idx)
      read_section(*this_region);

//    // Only read boco's if sections are not defined as BC's
//    if (!option("SectionsAreBCs")->value<bool>())
//    {
      // read boundaryconditions (or subregions) in this zone
      for (m_boco.idx=1; m_boco.idx<=m_zone.nbBocos; ++m_boco.idx)
        read_boco_unstructured(*this_region);
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
  else if(m_zone.type == CGNS_ENUMV( Structured ))
  {
    cgsize_t isize[3][3];
    char zone_name_char[CGNS_CHAR_MAX];
    CALL_CGNS(cg_zone_read(m_file.idx,m_base.idx,m_zone.idx,zone_name_char,isize[0]));
    m_zone.name = zone_name_char;
    boost::algorithm::replace_all(m_zone.name," ","_");
    boost::algorithm::replace_all(m_zone.name,".","_");
    boost::algorithm::replace_all(m_zone.name,":","_");
    boost::algorithm::replace_all(m_zone.name,"/","_");
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
    Handle<Region> this_region;
    if (m_zone.unique && options().value<bool>("zone_handling") == false)
    {
      this_region = mesh.topology().handle<Region>(); 
    }
    else
    {
      this_region = mesh.topology().create_region(m_zone.name).handle<Region>();
    }

    // Region& this_region = mesh.topology();
    this_region->add_tag("grid_zone");
    m_zone_map[m_zone.idx] = this_region.get();

    // read coordinates in this zone
    for (int i=1; i<=m_zone.nbGrids; ++i)
      read_coordinates_structured(*this_region);

    create_structured_elements(*this_region);

    // read boundaryconditions (or subregions) in this zone
    for (m_boco.idx=1; m_boco.idx<=m_zone.nbBocos; ++m_boco.idx)
      read_boco_structured(*this_region);

  }

  m_mesh->geometry_fields().update_structures();
  read_flowsolution();
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_coordinates_unstructured(Region& parent_region)
{

  CFinfo << "creating coordinates in " << parent_region.uri().string() << CFendl;

  Dictionary& nodes = m_mesh->geometry_fields();
  m_zone.nodes = &nodes;
  m_zone.nodes_start_idx = nodes.size();

  // read coordinates
  cgsize_t one = 1;
  Real *xCoord;
  Real *yCoord;
  Real *zCoord;
    
  switch (m_zone.coord_dim)
  {
    case 3:
      zCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateZ", CGNS_ENUMV( RealDouble ), &one, &m_zone.total_nbVertices, zCoord));
    case 2:
      yCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateY", CGNS_ENUMV( RealDouble ), &one, &m_zone.total_nbVertices, yCoord));
    case 1:
      xCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateX", CGNS_ENUMV( RealDouble ), &one, &m_zone.total_nbVertices, xCoord));
  }

  m_mesh->initialize_nodes(m_zone.total_nbVertices, (Uint)m_zone.coord_dim);
  common::Table<Real>& coords = nodes.coordinates();
  common::List<Uint>& rank = nodes.rank();

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

  cgsize_t one[3];
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
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateZ", CGNS_ENUMV( RealDouble ), one, m_zone.nbVertices, zCoord));
    case 2:
      yCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateY", CGNS_ENUMV( RealDouble ), one, m_zone.nbVertices, yCoord));
    case 1:
      xCoord = new Real[m_zone.total_nbVertices];
      CALL_CGNS(cg_coord_read(m_file.idx,m_base.idx,m_zone.idx, "CoordinateX", CGNS_ENUMV( RealDouble ), one, m_zone.nbVertices, xCoord));
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
  boost::algorithm::replace_all(m_section.name,":","_");
  boost::algorithm::replace_all(m_section.name,"/","_");

  // Create a new region for this section
  Region& this_region = parent_region.create_region(m_section.name);

  Dictionary& all_nodes = *m_zone.nodes;
  Uint start_idx = m_zone.nodes_start_idx;

  if (m_section.type == CGNS_ENUMV( MIXED )) // Different element types, Can also be faces
  {
    // Create Elements component for each element type.
    std::map<std::string,Handle< Elements > > cells = create_cells_in_region(this_region,all_nodes,get_supported_element_types());
    std::map<std::string,Handle< Elements > > faces = create_faces_in_region(this_region,all_nodes,get_supported_element_types());
    std::map<std::string,Handle< Elements > > elements;
    elements.insert(cells.begin(),cells.end());
    elements.insert(faces.begin(),faces.end());
    std::map<std::string, boost::shared_ptr< ArrayBufferT<Uint> > > buffer = create_connectivity_buffermap(elements);

    // Handle each element of this section separately to see in which Elements component it will be written
    for (int elem=m_section.eBegin;elem<=m_section.eEnd;++elem)
    {
      // Read the amount of nodes this 1 element contains
      CALL_CGNS(cg_ElementPartialSize(m_file.idx,m_base.idx,m_zone.idx,m_section.idx,elem,elem,(cgsize_t*)&m_section.elemNodeCount));
      m_section.elemNodeCount--; // subtract 1 as there is one index too many storing the element type

      // Storage for element type (index 0) and element nodes (index 1->elemNodeCount)
      cgsize_t elemNodes[1][1+m_section.elemNodeCount];

      // Read nodes of 1 element
      CALL_CGNS(cg_elements_partial_read(m_file.idx,m_base.idx,m_zone.idx,m_section.idx,elem,elem,*elemNodes,&m_section.parentData));

      // Store the cgns element type
      CGNS_ENUMT( ElementType_t ) etype_cgns = static_cast<CGNS_ENUMT( ElementType_t )>(elemNodes[0][0]);

      // Put the element nodes in a vector
      std::vector<Uint> row(m_section.elemNodeCount);
      for (int n=1;n<=m_section.elemNodeCount;++n)  // n=0 is the cell type
        row[n-1]=start_idx+(elemNodes[0][n]-1); // -1 because cgns has index-base 1 instead of 0

      // Convert the cgns element type to the CF element type
      const std::string& etype_CF = m_elemtype_CGNS_to_CF[etype_cgns]+to_str(m_zone.coord_dim)+"D";
      // Add the nodes to the correct Elements component using its buffer
      cf3_assert(buffer[etype_CF]);
      Uint table_idx = buffer[etype_CF]->add_row(row);

      // Store the global element number to a pair of (region , local element number)
      m_global_to_region.push_back(Region_TableIndex_pair(find_component_ptr_with_name<Elements>(this_region, "elements_"+etype_CF),table_idx));
      if ( ! m_global_to_region.back().first )
      {
        throw BadValue(FromHere(), etype_CF+" not found in "+this_region.uri().string());
      }
      cf3_assert( m_global_to_region.back().first );
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
    cgsize_t* elemNodes = new cgsize_t [m_section.elemDataSize];

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
  boost::algorithm::replace_all(m_boco.name,":","_");
  boost::algorithm::replace_all(m_boco.name,"/","_");

  // Read the element ID's
  cgsize_t* boco_elems = new cgsize_t [m_boco.nBC_elem];
  void* NormalList(NULL);
  CALL_CGNS(cg_boco_read(m_file.idx, m_base.idx, m_zone.idx, m_boco.idx, boco_elems, NormalList));

  // UNOFFICIAL CONVENTION/PRACTICE:
  // When there exists a CGNS section with the same name as a BC, then this section is taken as BC
  if (Handle<Component> section = parent_region.get_child(m_boco.name))
  {
    // Nothing to do, it is already added as a section.
    return;
  }

  switch (m_boco.ptset_type)
  {
    case CGNS_ENUMV( PointRange ):
    case CGNS_ENUMV( ElementRange ) : // all bc elements are within a range given by 2 global element numbers
    {
      if (m_zone.type != CGNS_ENUMV( Unstructured ))
        throw NotSupported(FromHere(),"CGNS: Boundary with pointset_type \"CGNS_ENUMV( ElementRange )\" is only supported for CGNS_ENUMV( Unstructured ) grids");

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
//      std::map<std::string,Handle< Elements > > cells = create_cells_in_region(this_region,nodes,get_supported_element_types());
//      std::map<std::string,Handle< Elements > > faces = create_faces_in_region(this_region,nodes,get_supported_element_types());
//      std::map<std::string,Handle< Elements > > elements;
//      elements.insert(cells.begin(),cells.end());
//      elements.insert(faces.begin(),faces.end());

      std::map<std::string,Handle< Elements > > elements = create_faces_in_region(this_region,nodes,get_supported_element_types());
      std::map<std::string,boost::shared_ptr< ArrayBufferT<Uint > > > buffer = create_connectivity_buffermap(elements);

      for (int global_element=boco_elems[0]-1;global_element<boco_elems[1];++global_element)
      {
        // Check which region this global_element belongs to
        Handle< Elements > element_region = m_global_to_region[global_element].first;

        // Check the local element number in this region
        Uint local_element = m_global_to_region[global_element].second;

        // Add the local element to the correct Elements component through its buffer
        std::cout << "element_region->element_type().derived_type_name() = " << element_region->element_type().derived_type_name() << std::endl;
        cf3_assert(buffer[element_region->element_type().derived_type_name()]);
        buffer[element_region->element_type().derived_type_name()]->add_row(element_region->geometry_space().connectivity()[local_element]);
      }

      // Flush all buffers and remove empty element regions
      for (BufferMap::iterator it=buffer.begin(); it!=buffer.end(); ++it)
        it->second->flush();
      buffer.clear();

      remove_empty_element_regions(this_region);
      break;
    }
    case CGNS_ENUMV( PointList ):
    case CGNS_ENUMV( ElementList ) : // all bc elements are listed as global element numbers
    {
      if (m_zone.type != CGNS_ENUMV( Unstructured ))
        throw NotSupported(FromHere(),"CGNS: Boundary with pointset_type \"ElementList\" is only supported for CGNS_ENUMV( Unstructured ) grids");

      // First do some simple checks to see if an entire region can be taken as a BC.
      std::cout << "boco_elems[0]-1 = " << boco_elems[0]-1 << std::endl;
      std::cout << m_global_to_region[boco_elems[0]-1].second << std::endl;
      cf3_assert(m_global_to_region[boco_elems[0]-1].first);
      Handle< Elements > first_elements = m_global_to_region[boco_elems[0]-1].first;
      cf3_assert(m_global_to_region[boco_elems[m_boco.nBC_elem-1]-1].first);
      Handle< Elements > last_elements = m_global_to_region[boco_elems[m_boco.nBC_elem-1]-1].first;
      if (first_elements->parent() == last_elements->parent())
      {
        Handle< Region > group_region = Handle<Region>(first_elements->parent());
        if (group_region->name() != m_boco.name)
        {
          if (group_region->recursive_elements_count(true) == Uint(boco_elems[m_boco.nBC_elem-1]-boco_elems[0]+1))
          {
            group_region->rename(m_boco.name);
            break;  // EXIT switch
          }
        }
      }


      // Create a region inside mesh/regions/bc-regions with the name of the cgns boco.
      Region& this_region = parent_region.create_region(m_boco.name);
      Dictionary& nodes = *m_zone.nodes;

      // Create Elements components for every possible element type supported.
      std::map<std::string,Handle< Elements > > elements = create_faces_in_region(this_region,nodes,get_supported_element_types());
//      std::map<std::string,Handle< Elements > > cells = create_cells_in_region(this_region,nodes,get_supported_element_types());
//      std::map<std::string,Handle< Elements > > faces = create_faces_in_region(this_region,nodes,get_supported_element_types());
//      std::map<std::string,Handle< Elements > > elements;
//      elements.insert(cells.begin(),cells.end());
//      elements.insert(faces.begin(),faces.end());

      std::map<std::string,boost::shared_ptr< ArrayBufferT<Uint > > > buffer = create_connectivity_buffermap(elements);

      for (int i=0; i<m_boco.nBC_elem; ++i)
      {
        Uint global_element = boco_elems[i]-1;

        // Check which region this global_element belongs to
        Handle< Elements > element_region = m_global_to_region[global_element].first;

        // Check the local element number in this region
        Uint local_element = m_global_to_region[global_element].second;

        // Add the local element to the correct Elements component through its buffer
        std::cout << "element_region->element_type().derived_type_name() = " << element_region->element_type().derived_type_name() << std::endl;
        cf3_assert(buffer[element_region->element_type().derived_type_name()]);
        buffer[element_region->element_type().derived_type_name()]->add_row(element_region->geometry_space().connectivity()[local_element]);
      }

      // Flush all buffers and remove empty element regions
      for (BufferMap::iterator it=buffer.begin(); it!=buffer.end(); ++it)
        it->second->flush();
      buffer.clear();

      remove_empty_element_regions(this_region);

      break;
    }
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
  boost::algorithm::replace_all(m_boco.name,":","_");
  boost::algorithm::replace_all(m_boco.name,"/","_");

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
  cgsize_t* boco_elems = new cgsize_t [m_boco.nBC_elem*m_base.cell_dim];
  void* NormalList(NULL);
  CALL_CGNS(cg_boco_read(m_file.idx, m_base.idx, m_zone.idx, m_boco.idx, boco_elems, NormalList));


  switch (m_boco.ptset_type)
  {
    case CGNS_ENUMV( ElementRange ) : // all bc elements are within a range given by 2 global element numbers
      throw NotSupported(FromHere(),"CGNS: Boundary with pointset_type \"CGNS_ENUMV( ElementRange )\" is only supported for CGNS_ENUMV( Unstructured ) grids");
    case CGNS_ENUMV( ElementList ) : // all bc elements are listed as global element numbers
      throw NotSupported(FromHere(),"CGNS: Boundary with pointset_type \"ElementList\" is only supported for CGNS_ENUMV( Unstructured ) grids");
    case CGNS_ENUMV( PointRange ) : // bc elements are given by node index range
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
    case CGNS_ENUMV( PointList ) : // bc elements are given by node index list
    default :
      throw NotImplemented(FromHere(),"CGNS: no boundary with pointset_type " + to_str<int>(m_boco.ptset_type) + " supported in CF yet");
  }


  remove_empty_element_regions(this_region);

}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_flowsolution()
{
//  std::cout << "nbsols = " << m_zone.nbSols << std::endl;
  for (m_flowsol.idx=1; m_flowsol.idx<=m_zone.nbSols; ++m_flowsol.idx)
  {
    char flowsol_name_char[CGNS_CHAR_MAX];
    CALL_CGNS(cg_sol_info(m_file.idx,m_base.idx,m_zone.idx,m_flowsol.idx,flowsol_name_char,&m_flowsol.grid_loc) );
    m_flowsol.name = flowsol_name_char;
    boost::algorithm::replace_all(m_flowsol.name," ","_");
    boost::algorithm::replace_all(m_flowsol.name,".","_");
    boost::algorithm::replace_all(m_flowsol.name,"-","_");
    boost::algorithm::replace_all(m_flowsol.name,":","_");
    boost::algorithm::replace_all(m_flowsol.name,"/","_");

    CALL_CGNS(cg_sol_ptset_info( m_file.idx,m_base.idx,m_zone.idx,m_flowsol.idx, &m_flowsol.ptset_type, &m_flowsol.npnts ));
    CALL_CGNS(cg_sol_size(m_file.idx,m_base.idx,m_zone.idx,m_flowsol.idx, &m_flowsol.data_dim, m_flowsol.dim_vals));
    CALL_CGNS(cg_nfields(m_file.idx,m_base.idx,m_zone.idx,m_flowsol.idx,&m_flowsol.nbFields));

    /// m_flowsol.ptset_type == CG_NULL    , m_flowsol.npnts == 0      --> flowsol covers entire zone
    /// m_flowsol.ptset_type == CGNS_ENUMV( PointRange ) , m_flowsol.npnts == 2      --> flowsol covers range
    /// m_flowsol.ptset_type == CGNS_ENUMV( PointList )  , m_flowsol.npnts == npnts  --> flowsol covers given list of points


    Handle<Dictionary> dict;
    Uint datasize=0;
    switch (m_flowsol.grid_loc)
    {
      case CGNS_ENUMV( Vertex ):
        datasize = m_zone.total_nbVertices;
        dict = m_mesh->geometry_fields().handle<Dictionary>();
        break;
      case CGNS_ENUMV( CellCenter ):
        throw NotImplemented(FromHere(), "CellCenter not implemented yet");
        datasize = m_zone.total_nbElements;
        if ( Handle<Component> found = m_mesh->get_child("CellCenter") )
          dict = found->handle<Dictionary>();
        else
          dict = m_mesh->create_discontinuous_space("CellCenter","cf3.mesh.LagrangeP0").handle<Dictionary>();
        break;
      default:
        throw NotSupported(FromHere(), "Flow solution Grid location ["+to_str((int)m_flowsol.grid_loc)+"] is not supported");
    }

    cf3_assert(datasize == m_zone.total_nbVertices);
    cf3_assert(datasize == m_mesh->geometry_fields().size());

    boost::shared_ptr<math::VariablesDescriptor> variables = allocate_component<math::VariablesDescriptor>("variables");
    variables->options().set("dimension",static_cast<Uint>(m_base.phys_dim));

    for (m_field.idx=1; m_field.idx<=m_flowsol.nbFields; ++m_field.idx)
    {
      char field_name_char[CGNS_CHAR_MAX];
      CALL_CGNS(cg_field_info(m_file.idx,m_base.idx,m_zone.idx,m_flowsol.idx,m_field.idx,&m_field.datatype,field_name_char));
      std::string var_name = field_name_char;
      boost::algorithm::replace_all(var_name," ","_");
      boost::algorithm::replace_all(var_name,".","_");
      boost::algorithm::replace_all(var_name,"-","_");
      boost::algorithm::replace_all(var_name,":","_");
      boost::algorithm::replace_all(var_name,"/","_");

      variables->push_back(var_name, math::VariablesDescriptor::Dimensionalities::SCALAR);
    }

    Field& flowsol_field = dict->create_field(m_flowsol.name,variables->description());
    // std::cout << "flowsol_field.size() = " <<  flowsol_field.size() << std::endl;
    for (m_field.idx=1; m_field.idx<=m_flowsol.nbFields; ++m_field.idx)
    {
      char field_name_char[CGNS_CHAR_MAX];
      CALL_CGNS(cg_field_info(m_file.idx,m_base.idx,m_zone.idx,m_flowsol.idx,m_field.idx,&m_field.datatype,field_name_char));
      m_field.name=field_name_char;

      std::vector<double> field_data(datasize);
      cgsize_t imin = 1;
      cgsize_t imax = datasize;
      CALL_CGNS(cg_field_read( m_file.idx,m_base.idx,m_zone.idx,m_flowsol.idx,
                               field_name_char,CGNS_ENUMV( RealDouble ),&imin,&imax,(void*)(&field_data[0]) ));

      cf3_assert(field_data.size() == flowsol_field.size());
      cf3_assert(flowsol_field.nb_vars() == m_flowsol.nbFields);
      cf3_assert(flowsol_field.row_size() == m_flowsol.nbFields);
      for (Uint i=0; i< field_data.size(); ++i)
      {
        flowsol_field[i][m_field.idx-1] = field_data[i];
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

Uint Reader::get_total_nbElements()
{
  Uint nbElements = 0;

  if (m_zone.type == CGNS_ENUMV( Unstructured ))
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
  else if (m_zone.type == CGNS_ENUMV( Structured ))
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
