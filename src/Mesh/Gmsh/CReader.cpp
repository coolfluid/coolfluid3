// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/OptionT.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/String/Conversion.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ConnectivityData.hpp"
#include "Mesh/CFlexTable.hpp"

#include "Mesh/Gmsh/CReader.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

  using namespace Common;
  using namespace Common::String;
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < Mesh::Gmsh::CReader,
                             Mesh::CMeshReader,
                             Mesh::Gmsh::LibGmsh,
                             1 >
aGmshReader_Builder ( "Gmsh" );

//////////////////////////////////////////////////////////////////////////////

CReader::CReader( const std::string& name )
: CMeshReader(name),
  Shared(),
  m_repartition(false)
{
  CF_DEBUG_POINT;

  BuildComponent<full>().build(this);

  CF_DEBUG_POINT;

  m_property_list["Repartition"].as_option().attach_trigger ( boost::bind ( &CReader::config_repartition,   this ) );
  
  m_property_list["brief"] = std::string("Gmsh file reader component");
  
  std::string desc;
  desc += "This component can read in parallel.\n";
  desc += "It can also read multiple files in serial, combining them in one large mesh.\n";
  desc += "Available coolfluid-element types are:\n";
  BOOST_FOREACH(const std::string& supported_type, m_supported_types)
  desc += "  - " + supported_type + "\n";
  m_property_list["description"] = desc;
}

void CReader::config_repartition()
{
  property("Repartition").put_value(m_repartition);
}
  
//////////////////////////////////////////////////////////////////////////////

void CReader::define_config_properties ( CF::Common::PropertyList& options )
{
  options.add_option<OptionT <bool> >("Serial Merge","New mesh will be merged with existing if mesh-names match",true);
  options.add_option<OptionT <bool> >("Unified Zones","Reads Neu Groups and splits the mesh in these subgroups",false);
  options.add_option<OptionT <Uint> >("Part","Number of the part of the mesh to read. (e.g. rank of processor)",PE::instance().is_init()?PE::instance().rank():0);
  options.add_option<OptionT <Uint> >("Number of Parts","Total number of parts. (e.g. number of processors)",PE::instance().is_init()?PE::instance().size():1);
	options.add_option<OptionT <bool> >("Read Boundaries","Read the surface elements for the boundary",true);

  
  options.add_option<OptionT <bool> >("Repartition","setting this to true, puts global indexes, for repartitioning later",false);
  options.add_option<OptionT <Uint> >("OutputRank","shows output for the specified rank",0);
}
  
//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CReader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".msh");
  return extensions;
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_from_to(boost::filesystem::path& fp, const CMesh::Ptr& mesh)
{

  // if the file is present open it
  if( boost::filesystem::exists(fp) )
  {
    CFLog(VERBOSE, "Opening file " <<  fp.string() << "\n");
    m_file.open(fp,std::ios_base::in); // exists so open it
  }
  else // doesnt exist so throw exception
  {
     throw boost::filesystem::filesystem_error( fp.string() + " does not exist", boost::system::error_code() );
  }

  m_file_basename = boost::filesystem::basename(fp);
  
  // set the internal mesh pointer
  m_mesh = mesh;

  // Read file once and store positions
  get_file_positions();
  
  // Read mesh information
  read_headerData();
  
  // Create a region component inside the mesh with the name mesh_name
  m_region = m_mesh->create_region(m_headerData.mesh_name,!property("Serial Merge").value<bool>()).get_type<CRegion>();

  // partition the nodes
  partition_nodes();
  
  //CFinfo << "nodes to read = " << m_nodes_to_read.size() << CFendl;
  //CFinfo << "elements to read = " << m_elements_to_read.size() << CFendl;
  
  read_coordinates();
  
  read_connectivity();

	if (property("Read Boundaries").value<bool>())
		read_boundaries();

  if (!property("Unified Zones").value<bool>())
    read_groups();
  
  
  // clean-up
  // --------
  // Remove regions with empty connectivity tables
  remove_empty_element_regions(get_component_typed<CRegion>(*m_mesh));

  // update the node lists contained by the element regions
  BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*m_region))
    elements.update_node_list();

  // update the number of cells and nodes in the mesh
  m_mesh->properties()["nb_cells"] = m_mesh->property("nb_cells").value<Uint>() + m_headerData.NELEM;
  m_mesh->properties()["nb_nodes"] = m_mesh->property("nb_nodes").value<Uint>() + m_headerData.NUMNP;

  // close the file
  m_file.close();

}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_headerData()
{
  m_file.seekg(0,std::ios::beg);
  
  Uint NUMNP, NELEM, NGRPS, NBSETS, NDFCD, NDFVL;
  std::string line;

  // skip 2 lines
  for (Uint i=0; i<2; ++i)
    getline(m_file,line);
  
  m_file >> m_headerData.mesh_name;   getline(m_file,line);
  
  // skip 3 lines
  for (Uint i=0; i<3; ++i)
    getline(m_file,line);

  // read number of points, elements, groups, sets, dimensions, velocitycomponents
  getline(m_file,line);
  std::stringstream ss(line);
  ss >> NUMNP >> NELEM >> NGRPS >> NBSETS >> NDFCD >> NDFVL;

  m_headerData.NUMNP  = NUMNP;
  m_headerData.NELEM  = NELEM;
  m_headerData.NGRPS  = NGRPS;
  m_headerData.NBSETS = NBSETS;
  m_headerData.NDFCD  = NDFCD;
  m_headerData.NDFVL  = NDFVL;
  
  getline(m_file,line);
}

//////////////////////////////////////////////////////////////////////////////
  
void CReader::get_file_positions()
{   
  std::string nodal_coordinates("NODAL COORDINATES");
  std::string elements_cells("ELEMENTS/CELLS");
  std::string element_group("ELEMENT GROUP");
  std::string boundary_condition("BOUNDARY CONDITIONS");
  
  m_element_group_positions.resize(0);
  m_boundary_condition_positions.resize(0);
  
  int p;
  std::string line;
  while (!m_file.eof())
  {
    p = m_file.tellg();
    getline(m_file,line);
    if (line.find(nodal_coordinates)!=std::string::npos)
      m_nodal_coordinates_position=p;
    else if (line.find(elements_cells)!=std::string::npos)
      m_elements_cells_position=p;
    else if (line.find(element_group)!=std::string::npos)
      m_element_group_positions.push_back(p);
    else if (line.find(boundary_condition)!=std::string::npos)
      m_boundary_condition_positions.push_back(p);
  }
  m_file.clear();
  
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_coordinates()
{   
  Uint global_start_idx = m_mesh->properties()["nb_nodes"].value<Uint>();
  
  m_file.seekg(m_nodal_coordinates_position,std::ios::beg);
  
  // Create the coordinates array
  m_coordinates = m_region->create_coordinates(m_headerData.NDFCD).get_type<CArray>();
  
  CArray& coordinates = *m_coordinates;
  Uint coord_start_idx = coordinates.size();
  coordinates.resize(coordinates.size()+m_nodes_to_read.size());

  
  CList<Uint>& global_node_idx = get_tagged_component_typed<CList<Uint> >(*m_coordinates,"global_node_indices");
  global_node_idx.resize(global_node_idx.size()+m_nodes_to_read.size());

  CList<bool>& is_ghost = *m_coordinates->get_child_type<CList<bool> >("is_ghost");
  is_ghost.resize(is_ghost.size()+m_nodes_to_read.size());

  
  std::string line;
  // skip one line
  getline(m_file,line);

  // declare and allocate one coordinate row
  std::vector<Real> rowVector(m_headerData.NDFCD);

  std::set<Uint>::const_iterator it;

  Uint coord_idx=coord_start_idx;
  for (Uint node_idx=1; node_idx<=m_headerData.NUMNP; ++node_idx) 
  {
    if (m_headerData.NUMNP > 100000)
    {  
      if(node_idx%(m_headerData.NUMNP/20)==0)
        CFinfo << 100*node_idx/m_headerData.NUMNP << "% " << CFendl;
    }
    getline(m_file,line);
    it = m_nodes_to_read.find(node_idx);
    if (it != m_nodes_to_read.end())
    {
      // add global node index
      global_node_idx[coord_idx] = global_start_idx + node_idx - 1; // -1 because base zero
      bool is_ghost_node = !(m_ghost_nodes.find(node_idx) == m_ghost_nodes.end());
      is_ghost[coord_idx] = is_ghost_node;
      m_node_to_coord_idx[node_idx]=coord_idx;
      std::stringstream ss(line);
      Uint nodeNumber;
      ss >> nodeNumber;
      //CFinfo << "reading node " << nodeNumber << CFendl;
      for (Uint dim=0; dim<m_headerData.NDFCD; ++dim)
        ss >> coordinates[coord_idx][dim];
      coord_idx++;
    }
  }
  
  getline(m_file,line);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::partition_nodes()
{
  Uint p=property("Part").value<Uint>();
  Uint np=property("Number of Parts").value<Uint>();
  std::pair<Uint,Uint> node_range = std::make_pair(m_headerData.NUMNP/np * p ,(p == np-1 ? m_headerData.NUMNP : m_headerData.NUMNP/np*(p+1)));  
  std::pair<Uint,Uint> elem_range = std::make_pair(m_headerData.NELEM/np * p ,(p == np-1 ? m_headerData.NELEM : m_headerData.NELEM/np*(p+1)));  
  m_nodes_to_read.clear();
  m_elements_to_read.clear();
  m_ghost_nodes.clear();
  
  m_file.seekg(m_elements_cells_position,std::ios::beg);
  // skip next line
  std::string line;
  getline(m_file,line);
  
  
  for (Uint i=node_range.first+1; i<node_range.second+1; ++i)
  {
    m_nodes_to_read.insert(i);
  }
  
  // read every line and store the connectivity in the correct region through the buffer
  Uint elementNumber, elementType, nbElementNodes;
  for (Uint i=0; i<m_headerData.NELEM; ++i) 
  {
    if (m_headerData.NELEM > 100000)
    {
      if(i%(m_headerData.NELEM/20)==0)
        CFinfo << 100*i/m_headerData.NELEM << "% " << CFendl;
    }

    if (i>=elem_range.first && i<elem_range.second)
    {
      // element description
      m_file >> elementNumber >> elementType >> nbElementNodes;
    
      m_elements_to_read.insert(elementNumber);
    
      // check if element nodes are ghost
      std::vector<Uint> gmsh_element_nodes(nbElementNodes);
      for (Uint j=0; j<nbElementNodes; ++j)
      {
        m_file >> gmsh_element_nodes[j];
        if (gmsh_element_nodes[j]<node_range.first+1 || gmsh_element_nodes[j]>=node_range.second+1)
        {
          m_ghost_nodes.insert(gmsh_element_nodes[j]);
          m_nodes_to_read.insert(gmsh_element_nodes[j]);
        }
      }
    }
    // finish the line
    getline(m_file,line);
  }
  getline(m_file,line);  // ENDOFSECTION

}
    
//////////////////////////////////////////////////////////////////////////////
  
void CReader::read_connectivity()
{  
  m_tmp = m_region->create_region("main").get_type<CRegion>();

  m_global_to_tmp.clear();
  m_file.seekg(m_elements_cells_position,std::ios::beg);
    
  CArray& coordinates = *m_coordinates;
  
  CFlexTable& node_to_glb_elem_connectivity = get_tagged_component_typed<CFlexTable>(coordinates,"glb_elem_connectivity");
  CList<bool>& is_ghost = *m_coordinates->get_child_type<CList<bool> >("is_ghost");
  
  m_node_to_glb_elements.resize(m_nodes_to_read.size());
  std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
      create_element_regions_with_buffermap(*m_tmp,coordinates,m_supported_types);

  std::map<std::string,CElements::Ptr> element_regions;
  BOOST_FOREACH(const std::string& etype, m_supported_types)
    element_regions[etype] = get_named_component_typed_ptr<CElements>(*m_tmp, "elements_" + etype);

  std::map<std::string,boost::shared_ptr<CList<Uint>::Buffer> > glb_elm_indices;
  BOOST_FOREACH(const std::string& etype, m_supported_types)
    glb_elm_indices[etype] = boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(get_tagged_component_typed_ptr< CList<Uint> >(*element_regions[etype], "global_element_indices")->create_buffer()) );

  // skip next line
  std::string line;
  getline(m_file,line);

  // read every line and store the connectivity in the correct region through the buffer
  std::string etype_CF;
  std::set<Uint>::const_iterator it;
  std::vector<Uint> cf_element;
  Uint gmsh_node_number;
  Uint cf_node_number;
  Uint cf_idx;
  Uint table_idx;
  
  for (Uint i=0; i<m_headerData.NELEM; ++i) 
  {
    if (m_headerData.NELEM > 100000)
    {
      if(i%(m_headerData.NELEM/20)==0)
        CFinfo << 100*i/m_headerData.NELEM << "% " << CFendl;
    }
    
    // element description
    Uint elementNumber, elementType, nbElementNodes;
    m_file >> elementNumber >> elementType >> nbElementNodes;
    
    bool read_this_elem = ( m_elements_to_read.find(elementNumber) != m_elements_to_read.end() );
      
    // get element nodes
    if (read_this_elem)
    {
      cf_element.resize(nbElementNodes);
      for (Uint j=0; j<nbElementNodes; ++j)
      {
        cf_idx = m_nodes_gmsh_to_cf[elementType][j];
        m_file >> gmsh_node_number;
        cf_node_number = m_node_to_coord_idx[gmsh_node_number];
        cf_element[cf_idx] = cf_node_number;
        if (!is_ghost[cf_node_number])
        {
          if (m_nodes_to_read.find(gmsh_node_number) != m_nodes_to_read.end() )
            m_node_to_glb_elements[cf_node_number].insert(elementNumber-1); 
        }
      }
      etype_CF = element_type(elementType,nbElementNodes);
      table_idx = buffer[etype_CF]->add_row(cf_element);
      m_global_to_tmp[elementNumber] = std::make_pair(element_regions[etype_CF],table_idx);
      glb_elm_indices[etype_CF]->add_row(elementNumber-1);
    }
    else
    {
      for (Uint j=0; j<nbElementNodes; ++j)
      {
        m_file >> gmsh_node_number;
        cf_node_number = m_node_to_coord_idx[gmsh_node_number];
        if (!is_ghost[cf_node_number])
        {
          if (m_nodes_to_read.find(gmsh_node_number) != m_nodes_to_read.end() )
            m_node_to_glb_elements[cf_node_number].insert(elementNumber-1);
        }   
      }
    }

    // finish the line
    getline(m_file,line);
  }
  getline(m_file,line);  // ENDOFSECTION
  
  
  node_to_glb_elem_connectivity.add_rows(m_node_to_glb_elements);
  
  m_node_to_coord_idx.clear();
  m_node_to_glb_elements.clear();
  
}

//////////////////////////////////////////////////////////////////////////////

std::string CReader::element_type(const Uint gmsh_type, const Uint nb_nodes)
{
  std::string cf_type;
  std::string dim = to_str<int>(m_headerData.NDFCD);
  if      (gmsh_type==P1LINE  && nb_nodes==2) cf_type = "Line"  + dim + "DLagrangeP1";  // line
  else if (gmsh_type==P1QUAD  && nb_nodes==4) cf_type = "Quad"  + dim + "DLagrangeP1";  // quadrilateral
  else if (gmsh_type==P1TRIAG && nb_nodes==3) cf_type = "Triag" + dim + "DLagrangeP1";  // triangle
  else if (gmsh_type==P1HEXA  && nb_nodes==8) cf_type = "Hexa"  + dim + "DLagrangeP1";  // hexahedron
  else if (gmsh_type==P1TETRA && nb_nodes==4) cf_type = "Tetra" + dim + "DLagrangeP1";  // tetrahedron
  /// @todo to be implemented
  else if (gmsh_type==5 && nb_nodes==6) // wedge (prism)
    throw Common::NotImplemented(FromHere(),"wedge or prism element not able to convert to COOLFluiD yet.");
  else if (gmsh_type==7 && nb_nodes==5) // pyramid
    throw Common::NotImplemented(FromHere(),"pyramid element not able to convert to COOLFluiD yet.");
  else {
    throw Common::NotSupported(FromHere(),"no support for element type/nodes "
                               + to_str<int>(gmsh_type) + "/" + to_str<int>(nb_nodes) +
                               " in Gambit Neutral format");
  }
  return cf_type;
}
  
//////////////////////////////////////////////////////////////////////////////

void CReader::read_groups()
{
  
  cf_assert(m_element_group_positions.size() == m_headerData.NGRPS)
  
  std::vector<GroupData> groups(m_headerData.NGRPS);
  std::string line;
  int dummy;
  
  CArray& coordinates = *m_coordinates;
  std::set<Uint>::const_iterator it;
  
  for (Uint g=0; g<m_headerData.NGRPS; ++g)
  {    
    m_file.seekg(m_element_group_positions[g],std::ios::beg);
    
    std::string ELMMAT;
    Uint NGP, NELGP, MTYP, NFLAGS, I;
    getline(m_file,line);  // ELEMENT GROUP...
    m_file >> line >> NGP >> line >> NELGP >> line >> MTYP >> line >> NFLAGS >> ELMMAT;
    groups[g].NGP    = NGP;
    groups[g].NELGP  = NELGP;
    groups[g].MTYP   = MTYP;
    groups[g].NFLAGS = NFLAGS;
    groups[g].ELMMAT = ELMMAT;
    //groups[g].print();
    
    for (Uint i=0; i<NFLAGS; ++i)
      m_file >> dummy;
    
    
    // 2 cases:
    // 1) there is only one group --> The tmp region can just be renamed
    //    and put in the filesystem as subcomponent of "mesh/regions"
    if (m_headerData.NGRPS == 1)
    {
      m_tmp->rename(groups[0].ELMMAT);
      m_tmp.reset();
      return;
    }
    // 2) there are multiple groups --> New regions have to be created
    //    and the elements from the tmp region have to be distributed among
    //    these new regions.
    
    //groups[g].ELEM.reserve(NELGP);
    for (Uint i=0; i<NELGP; ++i) 
    {
      m_file >> I;
      it = m_elements_to_read.find(I);
      if (it != m_elements_to_read.end())
        groups[g].ELEM.push_back(I);     // set element index
    }
    getline(m_file,line);  // finish the line (read new line)
    getline(m_file,line);  // ENDOFSECTION
  }
  

  // Create Region for each group
  BOOST_FOREACH(GroupData& group, groups)
  {
    
    CRegion& region = m_region->create_region(group.ELMMAT);
    
    //CFinfo << "region " << region.full_path().string() << " created" << CFendl;
    // Create regions for each element type in each group-region
    std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
    create_element_regions_with_buffermap(region,coordinates,m_supported_types);
    
    std::map<std::string,CElements::Ptr> element_regions;
    BOOST_FOREACH(const std::string& etype, m_supported_types)
      element_regions[etype] = get_named_component_typed_ptr<CElements>(region, "elements_" + etype);

    std::map<std::string,boost::shared_ptr<CList<Uint>::Buffer> > glb_elm_indices;
    BOOST_FOREACH(const std::string& etype, m_supported_types)
      glb_elm_indices[etype] = boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(get_tagged_component_typed_ptr< CList<Uint> >(*element_regions[etype], "global_element_indices")->create_buffer()) );
    
    
    // Copy elements from tmp_region in the correct region
    BOOST_FOREACH(Uint global_element, group.ELEM)
    {
      CElements::Ptr tmp_region = m_global_to_tmp[global_element].first;
      Uint local_element = m_global_to_tmp[global_element].second;
      std::string etype = tmp_region->element_type().getElementTypeName();
      
      Uint idx = buffer[etype]->add_row(tmp_region->connectivity_table().array()[local_element]);
      std::string new_region_name = "elements_" + tmp_region->element_type().getElementTypeName();
      m_global_to_tmp[global_element] = std::make_pair(region.get_child_type<CElements>(new_region_name),idx);
      Uint local_elm_idx = glb_elm_indices[etype]->add_row(global_element-1);
      
      if (local_elm_idx != idx)
      {
        throw BadValue(FromHere(), "global_indexes don't match with connectivity table: "+to_str(local_elm_idx)+"!="+to_str(idx));
      }
    }
  }

  m_region->remove_component(m_tmp->name());
  m_tmp.reset();
  
}
  
//////////////////////////////////////////////////////////////////////////////

void CReader::read_boundaries()
{

 /*
  Uint glb_element_count = m_headerData.NELEM;
  cf_assert(m_boundary_condition_positions.size() == m_headerData.NBSETS)
  
  CArray& coordinates = *m_coordinates;
  
  std::string line;
  for (Uint t=0; t<m_headerData.NBSETS; ++t) {

    m_file.seekg(m_boundary_condition_positions[t],std::ios::beg);
      
    std::string NAME;
    int ITYPE, NENTRY, NVALUES, IBCODE1, IBCODE2, IBCODE3, IBCODE4, IBCODE5;

    // read header
    getline(m_file,line);  // BOUNDARY CONDITIONS...
    getline(m_file,line);  // header
    std::stringstream ss(line);
    ss >> NAME >> ITYPE >> NENTRY >> NVALUES >> IBCODE1 >> IBCODE2 >> IBCODE3 >> IBCODE4 >> IBCODE5;
    if (ITYPE!=1) {
      throw Common::NotSupported(FromHere(),"error: supports only boundary condition data 1 (element/cell): page C-11 of user's guide");
    }
    if (IBCODE1!=6) {
      throw Common::NotSupported(FromHere(),"error: supports only IBCODE1 6 (ELEMENT_SIDE)");
    }

    CRegion& bc_region = m_region->create_region(NAME);

    // create all kind of element type regions
    BufferMap buffer = create_element_regions_with_buffermap (bc_region,coordinates,m_supported_types);


    std::map<std::string,CElements::Ptr> element_regions;
    BOOST_FOREACH(const std::string& etype, m_supported_types)
      element_regions[etype] = get_named_component_typed_ptr<CElements>(bc_region, "elements_" + etype);

    std::map<std::string,boost::shared_ptr<CList<Uint>::Buffer> > glb_elm_indices;
    BOOST_FOREACH(const std::string& etype, m_supported_types)
      glb_elm_indices[etype] = boost::shared_ptr<CList<Uint>::Buffer> ( new CList<Uint>::Buffer(get_tagged_component_typed_ptr< CList<Uint> >(*element_regions[etype], "global_element_indices")->create_buffer()) );

    // read boundary elements connectivity
    for (int i=0; i<NENTRY; ++i) 
    {
      int ELEM, ETYPE, FACE;
      m_file >> ELEM >> ETYPE >> FACE;

      Uint global_element = ELEM;
      
      std::map<Uint,Region_TableIndex_pair>::iterator it = m_global_to_tmp.find(global_element);
      if (it != m_global_to_tmp.end())
      {
        CElements::Ptr tmp_region = it->second.first;
        Uint local_element = it->second.second;
                
        //Uint elementType = ETYPE;
        Uint faceIdx = m_faces_gmsh_to_cf[ETYPE][FACE];

        const ElementType& etype = tmp_region->element_type();
        const ElementType::FaceConnectivity& face_connectivity = etype.face_connectivity();
        
        // make a row of nodes
        const CTable::Row& elem_nodes = tmp_region->connectivity_table()[local_element];
        std::vector<Uint> row;
        row.reserve(face_connectivity.face_node_counts[faceIdx]);
        BOOST_FOREACH(const Uint& node, face_connectivity.face_node_range(faceIdx))
          row.push_back(elem_nodes[node]);

        // add the row to the buffer of the face region
        std::string face_type = etype.face_type(faceIdx).getElementTypeName();
        buffer[face_type]->add_row(row);

        glb_elm_indices[face_type]->add_row(glb_element_count++);
      }
      getline(m_file,line);  // finish the line (read new line)
    }
    getline(m_file,line);  // ENDOFSECTION

  }

  */
}


//////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF
