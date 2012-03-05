// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/StreamHelpers.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"
#include "common/Tags.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/PropertyList.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MergedParallelDistribution.hpp"
#include "mesh/ParallelDistribution.hpp"
#include "mesh/Elements.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"
#include "mesh/MeshTransformer.hpp"

#include "mesh/neu/Reader.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace neu {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < neu::Reader, MeshReader, LibNeu > aneuReader_Builder;

//////////////////////////////////////////////////////////////////////////////

Reader::Reader( const std::string& name )
: MeshReader(name),
  Shared()
{
  // options
  options().add_option("read_groups" ,true)
      .description("Reads neu Groups and splits the mesh in these subgroups")
      .pretty_name("Unified Zones");

  options().add_option("part", PE::Comm::instance().rank())
      .description("Number of the part of the mesh to read. (e.g. rank of processor)")
      .pretty_name("Part");

  options().add_option("nb_parts", PE::Comm::instance().size())
      .description("Total nb_partitions. (e.g. number of processors)");

  options().add_option("read_boundaries", true)
      .description("Read the surface elements for the boundary")
      .pretty_name("Read Boundaries");

  properties()["brief"] = std::string("neutral file mesh reader component");

  std::string desc;
  desc += "This component can read in parallel.\n";
  desc += "Available coolfluid-element types are:\n";
  boost_foreach(const std::string& supported_type, m_supported_types)
  desc += "  - " + supported_type + "\n";
  properties()["description"] = desc;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Reader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".neu");
  return extensions;
}

//////////////////////////////////////////////////////////////////////////////

void Reader::do_read_mesh_into(const URI& file, Mesh& mesh)
{

  // if the file is present open it
  boost::filesystem::path fp (file.path());
  if( boost::filesystem::exists(fp) )
  {
    CFinfo << "Opening file " <<  fp.string() << CFendl;
    m_file.open(fp,std::ios_base::in); // exists so open it
  }
  else // doesnt exist so throw exception
  {
     throw boost::filesystem::filesystem_error( fp.string() + " does not exist", boost::system::error_code() );
  }

  // set the internal mesh pointer
  m_mesh = Handle<Mesh>(mesh.handle<Component>());

  // Read file once and store positions
  get_file_positions();

  // Read mesh information
  read_headerData();

  m_mesh->initialize_nodes(0, m_headerData.NDFCD);

  cf3_assert(m_mesh->geometry_fields().coordinates().row_size() == m_headerData.NDFCD);

  // Create a hash
  m_hash = create_component<MergedParallelDistribution>("hash");
  std::vector<Uint> num_obj(2);
  num_obj[0] = m_headerData.NUMNP;
  num_obj[1] = m_headerData.NELEM;
  m_hash->options().configure_option("nb_obj",num_obj);

  // Create a region component inside the mesh with the name mesh_name
  //if (option("new_api").value<bool>())
    m_region = Handle<Region>(m_mesh->topology().create_region(m_headerData.mesh_name).handle<Component>());
  //else
  //  m_region = m_mesh->create_region(m_headerData.mesh_name,!option("Serial Handle<Region>(Merge").value<bool>()).handle<Component>());

  find_ghost_nodes();
  read_coordinates();
  read_connectivity();
  if (options().option("read_boundaries").value<bool>())
    read_boundaries();

  if (options().option("read_groups").value<bool>())
    read_groups();

  // clean-up
  // --------
  // Remove regions with empty connectivity tables
  remove_empty_element_regions(m_mesh->topology());


  boost_foreach(Elements& elements, find_components_recursively<Elements>(m_mesh->topology()))
  {
    elements.rank().resize(elements.size());
    Uint my_rank = options().option("part").value<Uint>();
    for (Uint e=0; e<elements.size(); ++e)
    {
      elements.rank()[e] = my_rank;
    }
  }


  // close the file
  m_file.close();

  m_mesh->elements().update();
  m_mesh->update_statistics();

  cf3_assert(m_mesh->geometry_fields().coordinates().row_size() == m_headerData.NDFCD);
  cf3_assert(m_mesh->properties().value<Uint>(common::Tags::dimension()) == m_headerData.NDFCD);

  // Fix global numbering
  /// @todo remove this and read glb_index ourself
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering")->transform(m_mesh);
}

//////////////////////////////////////////////////////////////////////////////

void Reader::get_file_positions()
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

void Reader::read_headerData()
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

void Reader::find_ghost_nodes()
{
  m_ghost_nodes.clear();

  // Only find ghost nodes if the domain is split up
  if (options().option("nb_parts").value<Uint>() > 1)
  {
    m_file.seekg(m_elements_cells_position,std::ios::beg);
    // skip next line
    std::string line;
    getline(m_file,line);


    // read every line and store the connectivity in the correct region through the buffer
    Uint elementNumber, elementType, nbElementNodes;
    for (Uint i=0; i<m_headerData.NELEM; ++i)
    {
      if (m_headerData.NELEM > 100000)
      {
        if(i%(m_headerData.NELEM/20)==0)
          CFinfo << 100*i/m_headerData.NELEM << "% " << CFendl;
      }

      if (m_hash->subhash(ELEMS).owns(i))
      {
        // element description
        m_file >> elementNumber >> elementType >> nbElementNodes;

        // check if element nodes are ghost
        std::vector<Uint> neu_element_nodes(nbElementNodes);
        for (Uint j=0; j<nbElementNodes; ++j)
        {
          m_file >> neu_element_nodes[j];
          if (!m_hash->subhash(NODES).owns(neu_element_nodes[j]-1))
          {
            m_ghost_nodes.insert(neu_element_nodes[j]);
          }
        }
      }
      // finish the line
      getline(m_file,line);
    }
    getline(m_file,line);  // ENDOFSECTION
  }
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_coordinates()
{
  m_file.seekg(m_nodal_coordinates_position,std::ios::beg);

  // Create the nodes

  Dictionary& nodes = m_mesh->geometry_fields();

  nodes.resize(m_hash->subhash(NODES).nb_objects_in_part(PE::Comm::instance().rank()) + m_ghost_nodes.size());
  std::string line;
  // skip one line
  getline(m_file,line);

  // declare and allocate one coordinate row
  std::vector<Real> rowVector(m_headerData.NDFCD);

  std::set<Uint>::const_iterator not_found = m_ghost_nodes.end();

  Uint coord_idx=0;
  for (Uint node_idx=1; node_idx<=m_headerData.NUMNP; ++node_idx)
  {
    if (m_headerData.NUMNP > 100000)
    {
      if(node_idx%(m_headerData.NUMNP/20)==0)
        CFinfo << 100*node_idx/m_headerData.NUMNP << "% " << CFendl;
    }
    getline(m_file,line);
    if (m_hash->subhash(NODES).owns(node_idx-1))
    {
      nodes.rank()[coord_idx] = m_hash->subhash(NODES).part_of_obj(node_idx-1);
      nodes.glb_idx()[coord_idx] = node_idx;
      m_node_to_coord_idx[node_idx]=coord_idx;
      std::stringstream ss(line);
      Uint nodeNumber;
      ss >> nodeNumber;
      for (Uint dim=0; dim<m_headerData.NDFCD; ++dim)
        ss >> nodes.coordinates()[coord_idx][dim];
      coord_idx++;
    }
    else
    {
      if (m_ghost_nodes.find(node_idx) != not_found)
      {
        // add global node index
        nodes.rank()[coord_idx] = m_hash->subhash(NODES).part_of_obj(node_idx-1);
        nodes.glb_idx()[coord_idx] = node_idx;
        m_node_to_coord_idx[node_idx]=coord_idx;
        std::stringstream ss(line);
        Uint nodeNumber;
        ss >> nodeNumber;
        for (Uint dim=0; dim<m_headerData.NDFCD; ++dim)
          ss >> nodes.coordinates()[coord_idx][dim];
        coord_idx++;
      }
    }
  }
  getline(m_file,line);
}


//////////////////////////////////////////////////////////////////////////////

void Reader::read_connectivity()
{
  Dictionary& nodes = m_mesh->geometry_fields();
  m_tmp = Handle<Region>(m_region->create_region("main").handle<Component>());

  m_global_to_tmp.clear();
  m_file.seekg(m_elements_cells_position,std::ios::beg);

  std::map<std::string,Handle< Elements > > elements = create_cells_in_region(*m_tmp,nodes,m_supported_types);
  std::map<std::string,boost::shared_ptr< Connectivity::Buffer > > buffer = create_connectivity_buffermap(elements);

  // skip next line
  std::string line;
  getline(m_file,line);

  // read every line and store the connectivity in the correct region through the buffer
  std::string etype_CF;
  std::vector<Uint> cf_element;
  Uint neu_node_number;
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

    // get element nodes
    if (m_hash->subhash(ELEMS).owns(i))
    {
      cf_element.resize(nbElementNodes);
      for (Uint j=0; j<nbElementNodes; ++j)
      {
        cf_idx = m_nodes_neu_to_cf[elementType][j];
        m_file >> neu_node_number;
        cf_node_number = m_node_to_coord_idx[neu_node_number];
        cf_element[cf_idx] = cf_node_number;
      }
      etype_CF = element_type(elementType,nbElementNodes);
      table_idx = buffer[etype_CF]->add_row(cf_element);
      m_global_to_tmp[elementNumber] = std::make_pair(elements[etype_CF],table_idx);
    }
    else
    {
      for (Uint j=0; j<nbElementNodes; ++j)
      {
        m_file >> neu_node_number;
      }
    }

    // finish the line
    getline(m_file,line);
  }
  getline(m_file,line);  // ENDOFSECTION

  m_node_to_coord_idx.clear();

}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_groups()
{
  Dictionary& nodes = m_mesh->geometry_fields();
  cf3_assert(m_element_group_positions.size() == m_headerData.NGRPS)

  std::vector<GroupData> groups(m_headerData.NGRPS);
  std::string line;
  int dummy;

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

    // Read first to see howmany elements to allocate
    int p = m_file.tellg();
    Uint nb_elems_in_group = 0;
    for (Uint i=0; i<NELGP; ++i)
    {
      m_file >> I;
      if (m_hash->subhash(ELEMS).owns(I-1))
        nb_elems_in_group++;
    }
    // now allocate and read again
    groups[g].ELEM.reserve(nb_elems_in_group);
    m_file.seekg(p,std::ios::beg);
    for (Uint i=0; i<NELGP; ++i)
    {
      m_file >> I;
      if (m_hash->subhash(ELEMS).owns(I-1))
        groups[g].ELEM.push_back(I);     // set element index
    }

    getline(m_file,line);  // finish the line (read new line)
    getline(m_file,line);  // ENDOFSECTION
  }

  // Create Region for each group
  boost_foreach(GroupData& group, groups)
  {

    Region& region = m_region->create_region(group.ELMMAT);

    //CFinfo << "region " << region.uri().string() << " created" << CFendl;
    // Create regions for each element type in each group-region
    std::map<std::string,Handle< Elements > > elements = create_cells_in_region(region,nodes,m_supported_types);
    std::map<std::string,boost::shared_ptr< Connectivity::Buffer > > buffer = create_connectivity_buffermap(elements);

    // Copy elements from tmp_region in the correct region
    boost_foreach(Uint global_element, group.ELEM)
    {
      Handle< Elements > tmp_elems = m_global_to_tmp[global_element].first;
      Uint local_element = m_global_to_tmp[global_element].second;
      std::string etype = tmp_elems->element_type().derived_type_name();

      Uint idx = buffer[etype]->add_row(tmp_elems->geometry_space().connectivity().array()[local_element]);
      std::string new_elems_name = tmp_elems->name();
      m_global_to_tmp[global_element] = std::make_pair(Handle<Elements>(region.get_child(new_elems_name)),idx);
    }
  }

  m_region->remove_component(m_tmp->name());
  m_tmp.reset();

}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_boundaries()
{
  cf3_assert(m_boundary_condition_positions.size() == m_headerData.NBSETS)

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
      throw common::NotSupported(FromHere(),"error: supports only boundary condition data 1 (element/cell): page C-11 of user's guide");
    }
    if (IBCODE1!=6) {
      throw common::NotSupported(FromHere(),"error: supports only IBCODE1 6 (ELEMENT_SIDE)");
    }

    Region& bc_region = m_region->create_region(NAME);
    Dictionary& nodes = m_mesh->geometry_fields();

    // create all kind of element type regions
    std::map<std::string,Handle< Elements > > elements = create_faces_in_region (bc_region,nodes,m_supported_types);
    std::map<std::string,boost::shared_ptr< Connectivity::Buffer > > buffer = create_connectivity_buffermap (elements);

    // read boundary elements connectivity
    for (int i=0; i<NENTRY; ++i)
    {
      int ELEM, ETYPE, FACE;
      m_file >> ELEM >> ETYPE >> FACE;

      Uint global_element = ELEM;

      std::map<Uint,Region_TableIndex_pair>::iterator it = m_global_to_tmp.find(global_element);
      if (it != m_global_to_tmp.end())
      {
        Handle< Elements > tmp_elements = it->second.first;
        Uint local_element = it->second.second;

        //Uint elementType = ETYPE;
        Uint faceIdx = m_faces_neu_to_cf[ETYPE][FACE];

        const ElementType& etype = tmp_elements->element_type();
        const ElementType::FaceConnectivity& face_connectivity = etype.faces();

        // make a row of nodes
        const Connectivity::Row& elem_nodes = tmp_elements->geometry_space().connectivity()[local_element];
        std::vector<Uint> row;
        row.reserve(face_connectivity.stride[faceIdx]);
        boost_foreach(const Uint& node, face_connectivity.nodes_range(faceIdx))
          row.push_back(elem_nodes[node]);

        // add the row to the buffer of the face region
        std::string face_type = etype.face_type(faceIdx).derived_type_name();
        cf3_assert_desc(to_str(row.size())+"!="+to_str(buffer[face_type]->get_appointed().shape()[1]),row.size() == buffer[face_type]->get_appointed().shape()[1]);
        buffer[face_type]->add_row(row);

      }
      getline(m_file,line);  // finish the line (read new line)
    }
    getline(m_file,line);  // ENDOFSECTION

  }
}

//////////////////////////////////////////////////////////////////////////////

std::string Reader::element_type(const Uint neu_type, const Uint nb_nodes)
{
  std::string cf_type;
  std::string dim = to_str<int>(m_headerData.NDFCD);
  if      (neu_type==LINE  && nb_nodes==2) cf_type = "cf3.mesh.LagrangeP1.Line"  + dim + "D";  // line
  else if (neu_type==QUAD  && nb_nodes==4) cf_type = "cf3.mesh.LagrangeP1.Quad"  + dim + "D";  // quadrilateral
  else if (neu_type==TRIAG && nb_nodes==3) cf_type = "cf3.mesh.LagrangeP1.Triag" + dim + "D";  // triangle
  else if (neu_type==HEXA  && nb_nodes==8) cf_type = "cf3.mesh.LagrangeP1.Hexa"  + dim + "D";  // hexahedron
  else if (neu_type==TETRA && nb_nodes==4) cf_type = "cf3.mesh.LagrangeP1.Tetra" + dim + "D";  // tetrahedron
  /// @todo to be implemented
  else if (neu_type==5 && nb_nodes==6) // wedge (prism)
    throw common::NotImplemented(FromHere(),"wedge or prism element not able to convert to COOLFluiD yet.");
  else if (neu_type==7 && nb_nodes==5) // pyramid
    throw common::NotImplemented(FromHere(),"pyramid element not able to convert to COOLFluiD yet.");
  else {
    throw common::NotSupported(FromHere(),"no support for element type/nodes "
                               + to_str<int>(neu_type) + "/" + to_str<int>(nb_nodes) +
                               " in neutral format");
  }

  return cf_type;
}

//////////////////////////////////////////////////////////////////////////////

} // neu
} // mesh
} // cf3
