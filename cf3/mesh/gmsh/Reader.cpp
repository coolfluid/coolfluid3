// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"
#include "common/StreamHelpers.hpp"
#include "common/StringConversion.hpp"
#include "common/Table.hpp"
#include "common/List.hpp"
#include "common/DynTable.hpp"

#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/ConnectivityData.hpp"
#include "mesh/MergedParallelDistribution.hpp"
#include "mesh/ParallelDistribution.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Cells.hpp"

#include "mesh/gmsh/Reader.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace gmsh {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < gmsh::Reader, MeshReader, LibGmsh> aGmshReader_Builder;

//////////////////////////////////////////////////////////////////////////////

Reader::Reader( const std::string& name )
: MeshReader(name),
  Shared()
{

  // options

  options().add_option("part", PE::Comm::instance().rank() )
      .description("Number of the part of the mesh to read. (e.g. rank of processor)")
      .pretty_name("Part");

  options().add_option("nb_parts", PE::Comm::instance().size() )
      .description("Total number of parts. (e.g. number of processors)")
      .pretty_name("nb_parts");

  options().add_option("read_fields", true)
      .description("Read the data from the mesh")
      .pretty_name("Read Fields")
      .mark_basic();

  // properties

  properties()["brief"] = std::string("Gmsh file reader component");

  std::string desc;
  desc += "This component can read in parallel.\n";
  desc += "It can also read multiple files in serial, combining them in one large mesh.\n";
  desc += "Available coolfluid-element types are:\n";
  boost_foreach(const std::string& supported_type, m_supported_types)
  desc += "  - " + supported_type + "\n";
  properties()["description"] = desc;

  IO_rank = 1;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Reader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".msh");
  return extensions;
}

//////////////////////////////////////////////////////////////////////////////

void Reader::do_read_mesh_into(const URI& file, Mesh& mesh)
{

  // if the file is present open it
  boost::filesystem::path fp (file.path());
  if( boost::filesystem::exists(fp) )
  {
    CFinfo <<  "Opening file " <<  fp.string() << CFendl;
    m_file.open(fp,std::ios_base::in); // exists so open it
  }
  else // doesnt exist so throw exception
  {
     throw boost::filesystem::filesystem_error( fp.string() + " does not exist", boost::system::error_code() );
  }

  m_file_basename = boost::filesystem::basename(fp);

  // set the internal mesh pointer
  m_mesh = Handle<Mesh>(mesh.handle<Component>());

  // Create a region component inside the mesh with a generic mesh name
  // NOTE: since gmsh contains several 'physical entities' in one mesh, we create one region per physical entity
  m_region = Handle<Region>(m_mesh->topology().handle<Component>());

  // Read file once and store positions
  get_file_positions();

  m_mesh->initialize_nodes(0, m_mesh_dimension);


  find_used_nodes();

  read_coordinates();

  read_connectivity();

  fix_negative_volumes(*m_mesh);

  if (options().option("read_fields").value<bool>())
  {
    read_element_data();

    read_node_data();
  }

  m_node_idx_gmsh_to_cf.clear();
  m_elem_idx_gmsh_to_cf.clear();


  boost_foreach(Elements& elements, find_components_recursively<Elements>(m_mesh->topology()))
  {
    elements.rank().resize(elements.size());
    Uint my_rank = options().option("part").value<Uint>();
    for (Uint e=0; e<elements.size(); ++e)
    {
      elements.rank()[e] = my_rank;
    }
  }


  m_mesh->elements().update();
  m_mesh->update_statistics();

  // clean-up
  m_ghost_nodes.clear();
  m_used_nodes.clear();
  m_node_idx_gmsh_to_cf.clear();

  // close the file
  m_file.close();

}

//////////////////////////////////////////////////////////////////////////////

void Reader::get_file_positions()
{

  std::string region_names("$PhysicalNames");
  std::string nodes("$Nodes");
  std::string elements("$Elements");
  std::string element_data("$ElementData");
  std::string node_data("$NodeData");
  std::string element_node_data("$ElementNodeData");

  m_element_data_positions.clear();
  m_node_data_positions.clear();
  m_element_node_data_positions.clear();

  int p;
  std::string line;
  while (!m_file.eof())
  {
    p = m_file.tellg();
    getline(m_file,line);
    if (line.find(region_names)!=std::string::npos) {
      m_region_names_position=p;
      m_file >> m_nb_regions;
      m_region_list.resize(m_nb_regions);

      m_nb_gmsh_elem_in_region.resize(m_nb_regions);
      for(Uint ir = 0; ir < m_nb_regions; ++ir)
      {
        m_nb_gmsh_elem_in_region[ir].resize(Shared::nb_gmsh_types);
        for(Uint type = 0; type < Shared::nb_gmsh_types; ++ type)
           (m_nb_gmsh_elem_in_region[ir])[type] = 0;
      }

      m_mesh_dimension = DIM_1D;
      for(Uint ir = 0; ir < m_nb_regions; ++ir)
      {
        Uint phys_group_dimensionality;
        Uint phys_group_index;
        std::string phys_group_name;
        m_file >> phys_group_dimensionality >> phys_group_index >> phys_group_name;
        m_region_list[phys_group_index-1].dim=phys_group_dimensionality;
        m_region_list[phys_group_index-1].index=phys_group_index;
        //The original name of the region in the mesh file has quotes, we want to strip them off
        m_region_list[phys_group_index-1].name=phys_group_name.substr(1,phys_group_name.length()-2);
        m_region_list[phys_group_index-1].region = create_region(m_region_list[phys_group_index-1].name);
        m_mesh_dimension = std::max(m_region_list[phys_group_index-1].dim,m_mesh_dimension);
      }
    }
    else if (line.find(nodes)!=std::string::npos) {
      m_coordinates_position=p;
      m_file >> m_total_nb_nodes;
      //CFinfo << "The total number of nodes is " << m_total_nb_nodes << CFendl;
    }
    else if (line.find(elements)!=std::string::npos)
    {
      m_elements_position = p;
      m_file >> m_total_nb_elements;
      //CFinfo << "The total number of elements is " << m_total_nb_elements << CFendl;

      //Create a hash
      m_hash = create_component<MergedParallelDistribution>("hash");
      std::vector<Uint> num_obj(2);
      num_obj[0] = m_total_nb_nodes;
      num_obj[1] = m_total_nb_elements;
      m_hash->options().configure_option("nb_parts",options().option("nb_parts").value<Uint>());
      m_hash->options().configure_option("nb_obj",num_obj);


      Uint elem_idx, elem_type, nb_tags, phys_tag;

      //Let's count how many elements of each type are present
      for(Uint ie = 0; ie < m_total_nb_elements; ++ie)
      {
          m_file >> elem_idx;
          m_file >> elem_type;
          m_file >> nb_tags;
          m_file >> phys_tag;
          cf3_assert(phys_tag > 0);
          getline(m_file,line);
          if (m_hash->subhash(ELEMS).owns(ie))
            (m_nb_gmsh_elem_in_region[phys_tag-1])[elem_type]++;
          m_region_list[phys_tag-1].element_types.insert(elem_type);
      }
    }
    else if (line.find(element_data)!=std::string::npos)
    {
      m_element_data_positions.push_back(p);
    }
    else if (line.find(node_data)!=std::string::npos)
    {
      m_node_data_positions.push_back(p);
    }
    else if (line.find(element_data)!=std::string::npos)
    {
      m_element_node_data_positions.push_back(p);
    }

  }
  m_file.clear();

  if (m_element_node_data_positions.size())
    CFwarn << "ElementNodeData record(s) found. The gmsh reader has not implemented reading this record yet. They will be ignored" << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

Handle< Region > Reader::create_region(std::string const& relative_path)
{
  typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
  boost::char_separator<char> sep("/");
  Tokenizer tokens(relative_path, sep);

  Handle< Region > region = m_region;
  for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
  {
    std::string name = *tok_iter;
    Handle< Component > new_region = region->get_child(name);
    if (is_null(new_region))  region->create_component<Region>(name);
    region = Handle<Region>(region->get_child(name));
  }
  return region;
}

//////////////////////////////////////////////////////////////////////////////

void Reader::find_used_nodes()
{
  m_used_nodes.clear();
  std::string line;

  m_file.seekg(m_elements_position,std::ios::beg);
  // skip next line
  getline(m_file,line);
//  if (PE::Comm::instance().rank()==IO_rank)
//    std::cout << "header = " << line << std::endl;
  //Skip the line containing the actual number of elements
  getline(m_file,line);
//  if (PE::Comm::instance().rank()==IO_rank)
//    std::cout << "nb_elems = " << line << std::endl;

  // read every line and store the connectivity in the correct region through the buffer
  Uint elementNumber, elementType, nbElementNodes;
  Uint gmsh_node_number, nb_tags, phys_tag, other_tag;

  std::set<Uint>::iterator it=m_used_nodes.begin();

//  if (PE::Comm::instance().rank()==IO_rank)
//  {
//    std::cout << "nb_obj = "   << m_hash->subhash(ELEMS).options()["nb_obj"].value<Uint>() << std::endl;
//    std::cout << "nb_parts = " <<m_hash->subhash(ELEMS).options()["nb_parts"].value<Uint>() << std::endl;
//  }
  for (Uint i=0; i<m_total_nb_elements; ++i)
  {
    if (m_total_nb_elements > 100000)
    {
      if(i%(m_total_nb_elements/20)==0)
        CFinfo << 100*i/m_total_nb_elements << "% " << CFendl;
    }

    if (m_hash->subhash(ELEMS).owns(i))
    {
      // element description
      m_file >> elementNumber >> elementType;
      nbElementNodes = Shared::m_nodes_in_gmsh_elem[elementType];

//      if (PE::Comm::instance().rank()==IO_rank)
//        std::cout << i << ":  elem["<<m_hash->subhash(ELEMS).part_of_obj(i)<<"] " << elementNumber << " :    ";
      m_file >> nb_tags;
      m_file >> phys_tag;
      for(Uint itag = 0; itag < (nb_tags-1); ++itag)
          m_file >> other_tag;

      for (Uint j=0; j<nbElementNodes; ++j)
      {
        m_file >> gmsh_node_number;
        it = m_used_nodes.insert(it,gmsh_node_number);
//        if (PE::Comm::instance().rank()==IO_rank)
//          std::cout << "  " << gmsh_node_number;
      }
//      if (PE::Comm::instance().rank()==IO_rank)
//        std::cout << "\n";
    }
    getline(m_file,line); // finnish the line
  }

  // Now we have all nodes, used by the elements

  // Now add owned nodes
  m_file.seekg(m_coordinates_position,std::ios::beg);
  //Skip the line with keyword '$Nodes':
  getline(m_file,line);
//  if (PE::Comm::instance().rank()==IO_rank)
//    std::cout << "header = " << line << std::endl;
  // skip one line, which says how many (total) nodes are  present in the mesh
  getline(m_file,line);
//  if (PE::Comm::instance().rank()==IO_rank)
//    std::cout << "nb_nodes = " << line << std::endl;

  for (Uint node_idx=0; node_idx<m_total_nb_nodes; ++node_idx)
  {
    if (m_total_nb_nodes > 100000)
    {
      if(node_idx%(m_total_nb_nodes/20)==0)
        CFinfo << 100*node_idx/m_total_nb_nodes << "% " << CFendl;
    }
    getline(m_file,line);

    std::stringstream ss(line);
    ss >> gmsh_node_number;
    getline(ss,line);
    if (m_hash->subhash(NODES).owns(node_idx))
    {
//      if (PE::Comm::instance().rank()==IO_rank)
//        std::cout << "node " << gmsh_node_number << "\n";

      it = m_used_nodes.insert(it,gmsh_node_number);
    }
    else
    {
      it = m_used_nodes.find(gmsh_node_number);
      if (it != m_used_nodes.end())
      {
        m_ghost_nodes.insert(node_idx);
//        if (PE::Comm::instance().rank()==IO_rank)
//          std::cout << "ghostnode " << gmsh_node_number << "\n";
      }
    }
  }
//  if (PE::Comm::instance().rank()==IO_rank)
//    std::cout << std::flush;
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_coordinates()
{

  m_file.seekg(m_coordinates_position,std::ios::beg);

  // Find the region which has the highest dimensionality present in the mesh:
  Uint master_region = 0;
  while((m_region_list[master_region].dim != m_mesh_dimension) && (master_region < m_nb_regions))
  {
     master_region++;
  }

  Dictionary& nodes = m_mesh->geometry_fields();
  nodes.resize(m_used_nodes.size());
  m_used_nodes.clear();

  Uint part = options().option("part").value<Uint>();

  std::string line;
  //Skip the line with keyword '$Nodes':
  getline(m_file,line);
  // skip one line, which says how many (total) nodes are  present in the mesh
  getline(m_file,line);


  // declare and allocate one coordinate row
//  std::vector<Real> rowVector(m_mesh_dimension);

  std::set<Uint>::const_iterator it;

  Uint coord_idx=0;
  Uint gmsh_node_number;

  for (Uint node_idx=0; node_idx<m_total_nb_nodes; ++node_idx)
  {
    if (m_total_nb_nodes > 100000)
    {
      if(node_idx%(m_total_nb_nodes/20)==0)
        CFinfo << 100*node_idx/m_total_nb_nodes << "% " << CFendl;
    }
    getline(m_file,line);

    if (m_hash->subhash(NODES).owns(node_idx))
    {
      std::stringstream ss(line);
      ss >> gmsh_node_number;
      m_node_idx_gmsh_to_cf[gmsh_node_number]=coord_idx;

//      if (PE::Comm::instance().rank()==IO_rank)
//        std::cout << gmsh_node_number << " --> " << coord_idx << std::endl;

      for (Uint dim=0; dim<m_mesh_dimension; ++dim)
        ss >> nodes.coordinates()[coord_idx][dim];
      if(m_mesh_dimension < DIM_3D) getline(ss,line); //Gmsh always stores 3 coordinates, even for 2D meshes

      nodes.rank()[coord_idx] = part;
      nodes.glb_idx()[coord_idx] = gmsh_node_number-1;

      coord_idx++;
    }
    else
    {
      it = m_ghost_nodes.find(node_idx);
      if (it != m_ghost_nodes.end())
      {
        std::stringstream ss(line);
        ss >> gmsh_node_number;
        m_node_idx_gmsh_to_cf[gmsh_node_number]=coord_idx;

//        if (PE::Comm::instance().rank()==IO_rank)
//          std::cout << gmsh_node_number << " --> " << coord_idx << std::endl;

        for (Uint dim=0; dim<m_mesh_dimension; ++dim)
        ss >> nodes.coordinates()[coord_idx][dim];
        if(m_mesh_dimension < DIM_3D) getline(ss, line);

        nodes.rank()[coord_idx] = m_hash->subhash(NODES).part_of_obj(node_idx);
        nodes.glb_idx()[coord_idx] = gmsh_node_number-1;

        coord_idx++;
      }
    }


  } //loop over nodes



  /*
  CFinfo << "Printing all nodal coordinates:" << CFendl;
  for(Uint i = 0; i < m_total_nb_nodes; ++i)
  {
      CFinfo << "\t";
      for(Uint dim = 0; dim < m_mesh_dimension; ++dim)
        {
            CFinfo << nodes.coordinates()[i][dim] << " ";
        }
      CFinfo << CFendl;
  }
  */

  getline(m_file,line);
}

//////////////////////////////////////////////////////////////////////////////

void Reader::read_connectivity()
{

  Dictionary& nodes = m_mesh->geometry_fields();


  Uint part = options().option("part").value<Uint>();

  //Each entry of this vector holds a map (gmsh_type_idx, pointer to connectivity table of this gmsh type).
 //Each row corresponds to one region of the mesh
 std::vector<std::map<Uint, Entities* > > conn_table_idx;
 conn_table_idx.resize(m_nb_regions);
 for(Uint ir = 0; ir < m_nb_regions; ++ir)
 {
    conn_table_idx[ir].clear();
 }

 std::map<Uint, Entities*>::iterator elem_table_iter;

// std::vector<std::map<std::string,Handle< Elements > > > elements(m_nb_regions);
// std::vector<std::map<std::string,Handle< Connectivity::Buffer > > > buffer(m_nb_regions);


 m_elem_idx_gmsh_to_cf.clear();
 //Loop over all regions and allocate a connectivity table of proper size for each element type that
 //is present in each region. Counting of elements was done during the first pass in the function
 //get_file_positions
 for(Uint ir = 0; ir < m_nb_regions; ++ir)
 {
   // create new region
   Handle< Region > region = m_region_list[ir].region;

//   elements[ir] = create_cells_in_region(*region,m_mesh->geometry_fields(),m_supported_types);
//   buffer[ir] = create_connectivity_buffermap(elements[ir]);


   // Take the gmsh element types present in this region and generate new names of elements which correspond
   // to coolfuid naming:
   for(Uint etype = 0; etype < Shared::nb_gmsh_types; ++etype)
     if(m_region_list[ir].element_types.find(etype) !=  m_region_list[ir].element_types.end())
     {
       const std::string cf_elem_name = Shared::gmsh_name_to_cf_name(m_mesh_dimension,etype);

       boost::shared_ptr< ElementType > allocated_type = build_component_abstract_type<ElementType>(cf_elem_name,"tmp");
       boost::shared_ptr< Entities > elements;
       if (allocated_type->dimensionality() == allocated_type->dimension()-1)
         elements = build_component_abstract_type<Entities>("cf3.mesh.Faces",allocated_type->shape_name());
       else if(allocated_type->dimensionality() == allocated_type->dimension())
         elements = build_component_abstract_type<Entities>("cf3.mesh.Cells",allocated_type->shape_name());
       else
         elements = build_component_abstract_type<Entities>("cf3.mesh.Elements",allocated_type->shape_name());
      region->add_component(elements);
      elements->initialize(cf_elem_name,nodes);

      // Celements& elements = region->create_component<Elements>(cf_elem_name);
      // elements.initialize(cf_elem_name,nodes);

       Connectivity& elem_table = Handle<Elements>(elements)->geometry_space().connectivity();
       elem_table.set_row_size(Shared::m_nodes_in_gmsh_elem[etype]);
       elem_table.resize((m_nb_gmsh_elem_in_region[ir])[etype]);
       elements->rank().resize(m_nb_gmsh_elem_in_region[ir][etype]);
       conn_table_idx[ir].insert(std::pair<Uint,Entities*>(etype,elements.get()));
     }


 }

   std::string etype_CF;
   std::set<Uint>::const_iterator it;
   std::vector<Uint> cf_element;
   Uint element_number, gmsh_element_type, nb_element_nodes;
   Uint gmsh_node_number, nb_tags, phys_tag, other_tag;
   Uint cf_node_number;
   Uint cf_idx;

   m_file.seekg(m_elements_position,std::ios::beg);
   // skip next line
   std::string line;
   //Re-read the line that contains the keyword '$Elements':
   getline(m_file,line);
   //Parse the following line (containing the actual number of elements)
   getline(m_file,line);

   for(Uint ir = 0; ir < m_nb_regions; ++ir)
     for(Uint etype = 0; etype < Shared::nb_gmsh_types; ++etype)
      (m_nb_gmsh_elem_in_region[ir])[etype] = 0;

  for (Uint i=0; i<m_total_nb_elements; ++i)
  {
    if (m_total_nb_elements > 100000)
    {
      if(i%(m_total_nb_elements/20)==0)
        CFinfo << 100*i/m_total_nb_elements << "% " << CFendl;
    }

    // element description
    m_file >> element_number >> gmsh_element_type;

    nb_element_nodes = Shared::m_nodes_in_gmsh_elem[gmsh_element_type];

    // get element nodes
    if (m_hash->subhash(ELEMS).owns(i))
    {
      m_file >> nb_tags;
      m_file >> phys_tag;
      for(Uint itag = 0; itag < (nb_tags-1); ++itag)
          m_file >> other_tag;

//      CFinfo << "Reading element " << element_number << " of type " << gmsh_element_type;
//      CFinfo << " in region " << phys_tag << " with " << nb_element_nodes << " nodes " << CFendl;

//      if (PE::Comm::instance().rank()==IO_rank)
//        std::cout << element_number << "   -->   ";

      cf_element.resize(nb_element_nodes);
      for (Uint j=0; j<nb_element_nodes; ++j)
      {
        cf_idx = m_nodes_gmsh_to_cf[gmsh_element_type][j];
        m_file >> gmsh_node_number;
        cf_node_number = m_node_idx_gmsh_to_cf[gmsh_node_number];
        cf_element[cf_idx] = cf_node_number;
//        if (PE::Comm::instance().rank()==IO_rank)
//          std::cout << "  " << gmsh_node_number << "("<<m_node_idx_gmsh_to_cf[gmsh_node_number]<<")";
      }
//      if (PE::Comm::instance().rank()==IO_rank)
//        std::cout << std::endl;

      elem_table_iter = conn_table_idx[phys_tag-1].find(gmsh_element_type);
      const Uint row_idx = (m_nb_gmsh_elem_in_region[phys_tag-1])[gmsh_element_type];

      Handle< Elements > elements_region = Handle<Elements>(elem_table_iter->second->handle<Component>());
      Connectivity::Row element_nodes = elements_region->geometry_space().connectivity()[row_idx];

      m_elem_idx_gmsh_to_cf[element_number] = boost::make_tuple( elements_region , row_idx);

      for(Uint node = 0; node < nb_element_nodes; ++node)
      {
         element_nodes[node] = cf_element[node];
      }

      elements_region->rank()[row_idx] = part;

      (m_nb_gmsh_elem_in_region[phys_tag-1])[gmsh_element_type]++;

    }

    // finish the line
    getline(m_file,line);
  }
  getline(m_file,line);  // ENDOFSECTION
}

////////////////////////////////////////////////////////////////////////////////

void Reader::read_element_data()
{
  //  $ElementData
  //  1              // 1 string tag:
  //  "a_string_tag" //   the name of the view
  //  1              // 1 real tag:
  //  0              //  time value == 0
  //  3              // 3 integer tags:
  //  0              //  time step == 0 (time steps always start at 0)
  //  1              //  1-component field (scalar) (only 1,3,9 are valid)
  //  6              //  data size: 6 values follow:
  //  1 0.0          //  value associated with node 1 == 0.0
  //  2 0.1          //  ...
  //  3 0.2
  //  4 0.0
  //  5 0.2
  //  6 0.4
  //  $EndElementData

  std::map<std::string,Reader::Field> fields;

  boost_foreach(Uint element_data_position, m_element_data_positions)
  {
    m_file.seekg(element_data_position,std::ios::beg);
    read_variable_header(fields);
  }

  if (fields.size())
  {
    Dictionary& dict = m_mesh->create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0",std::vector< Handle<Region> >(1,m_mesh->topology().handle<Region>()));

    foreach_container((const std::string& name) (Reader::Field& gmsh_field) , fields)
    {
      std::vector<std::string> var_types_str;
      boost_foreach(const Uint var_type, gmsh_field.var_types)
          var_types_str.push_back(var_type_gmsh_to_cf(var_type));

      mesh::Field& field = dict.create_field(gmsh_field.name);
      field.options().configure_option("var_names",gmsh_field.var_names);
      field.options().configure_option("var_types",var_types_str);

      for (Uint i=0; i<field.nb_vars(); ++i)
      {
        CFdebug << "Reading " << field.name() << "/" << field.var_name(i) <<"["<<static_cast<Uint>(field.var_length(i))<<"]" << CFendl;
        Uint var_begin = field.var_index(i);
        Uint var_end = var_begin + static_cast<Uint>(field.var_length(i));
        m_file.seekg(gmsh_field.file_data_positions[i]);


        Uint gmsh_elem_idx;
        Uint cf_idx;
        Handle< Elements > elements;
        Uint d;
        std::vector<Real> data(gmsh_field.var_types[i]);

        for (Uint e=0; e<gmsh_field.nb_entries; ++e)
        {
          m_file >> gmsh_elem_idx;
          for (d=0; d<data.size(); ++d)
            m_file >> data[d];

          std::map<Uint, boost::tuple<Handle< Elements >,Uint> >::iterator it = m_elem_idx_gmsh_to_cf.find(gmsh_elem_idx);
          if (it != m_elem_idx_gmsh_to_cf.end())
          {
            boost::tie(elements,cf_idx) = it->second;

            mesh::Field::Row field_data = field[field.space(*elements).connectivity()[cf_idx][0]] ;

            d=0;
            for(Uint v=var_begin; v<var_end; ++v)
              field_data[v] = data[d++];
          }
        }
      }
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void Reader::read_node_data()
{
  //  $NodeData
  //  1              // 1 string tag:
  //  "a_string_tag" //   the name of the view
  //  1              // 1 real tag:
  //  0              //  time value == 0
  //  3              // 3 integer tags:
  //  0              //  time step == 0 (time steps always start at 0)
  //  1              //  1-component field (scalar) (only 1,3,9 are valid)
  //  6              //  nb_nodes_in_view: 6 values follow:
  //  1 0.0          //  value associated with node 1 == 0.0
  //  2 0.1          //  ...
  //  3 0.2
  //  4 0.0
  //  5 0.2
  //  6 0.4
  //  $EndNodeData

  std::map<std::string,Field> fields;

  boost_foreach(Uint node_data_position, m_node_data_positions)
  {
    m_file.seekg(node_data_position,std::ios::beg);
    read_variable_header(fields);
  }

  foreach_container((const std::string& name) (Field& gmsh_field) , fields)
  {
    std::vector<std::string> var_types_str;
    boost_foreach(const Uint var_type, gmsh_field.var_types)
      var_types_str.push_back(var_type_gmsh_to_cf(var_type));

    mesh::Field& field = m_mesh->geometry_fields().create_field(gmsh_field.name);
    field.options().configure_option("var_names",gmsh_field.var_names);
    field.options().configure_option("var_types",var_types_str);
    field.resize(gmsh_field.nb_entries);

    for (Uint i=0; i<field.nb_vars(); ++i)
    {
      CFdebug << "Reading " << field.name() << "/" << field.var_name(i) <<"["<<static_cast<Uint>(field.var_length(i))<<"]" << CFendl;
      Uint var_begin = field.var_index(i);
      Uint var_end = var_begin + static_cast<Uint>(field.var_length(i));
      m_file.seekg(gmsh_field.file_data_positions[i]);

      Uint gmsh_node_idx;
      Uint cf_idx;
      Uint d;
      std::vector<Real> data(gmsh_field.var_types[i]);

      for (Uint e=0; e<gmsh_field.nb_entries; ++e)
      {
        m_file >> gmsh_node_idx;
        for (d=0; d<data.size(); ++d)
          m_file >> data[d];

        std::map<Uint, Uint>::iterator it = m_node_idx_gmsh_to_cf.find(gmsh_node_idx);
        if (it != m_node_idx_gmsh_to_cf.end())
        {
          cf_idx = it->second;
          mesh::Field::Row field_data = field[cf_idx];

          d=0;
          for(Uint v=var_begin; v<var_end; ++v)
            field_data[v] = data[d++];
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Reader::read_variable_header(std::map<std::string,Field>& fields)
{
  std::string line;
  std::string dummy;


  Uint nb_string_tags(0);
  std::string var_name("var");
  std::string field_name("field");
  Uint nb_real_tags(0);
  Real field_time(0.);
  Uint nb_integer_tags(0);
  Uint field_time_step(0);
  Uint var_type(0);
  Uint nb_entries(0);

  //Re-read the line that contains the keyword '$Elements':
  getline(m_file,line);

  // string tags
  m_file >> nb_string_tags;
  if (nb_string_tags > 0)
  {
    m_file >> var_name;
    var_name = var_name.substr(1,var_name.length()-2);

    field_name = var_name;
    if (nb_string_tags > 1)
    {
      m_file >> field_name;
      field_name = field_name.substr(1,field_name.length()-2);
    }
    if (nb_string_tags > 2)
    {
      for (Uint i=1; i<nb_string_tags; ++i)
        m_file >> dummy;
    }
  }

  // real tags
  m_file >> nb_real_tags;
  if (nb_real_tags > 0)
  {
    if (nb_real_tags != 1)
      throw ParsingFailed(FromHere(),"Data cannot have more than 1 real tag (time)");

    m_file >> field_time;
  }

  // integer tags
  m_file >> nb_integer_tags;
  if (nb_integer_tags > 0)
  {
    if (nb_integer_tags >= 3)
    {
      m_file >> field_time_step >> var_type >> nb_entries;
    }
    if (nb_integer_tags < 3)
      throw ParsingFailed(FromHere(),"Data must have 3 integer tags (time_step, variable_type, nb_entries)");
  }
  getline(m_file,line); // finish line

  Field& field = fields[field_name];
  field.name=field_name;
  field.var_names.push_back(var_name);
  field.var_types.push_back(var_type);
  field.time=field_time;
  field.time_step=field_time_step;
  field.nb_entries=nb_entries;
  field.file_data_positions.push_back(m_file.tellg());
}


////////////////////////////////////////////////////////////////////////////////

std::string Reader::var_type_gmsh_to_cf(const Uint& var_type_gmsh)
{
  std::string dim = to_str(m_mesh->geometry_fields().coordinates().row_size());
  switch (var_type_gmsh)
  {
    case 1:
      return "scalar";
    case 3:
      return "vector"+dim+"D";
    case 9:
      return "tensor"+dim+"D";
    default:
      throw FileFormatError(FromHere(),"Gmsh variable type should be either 1(scalar), 3(vector), 9(tensor). Found: "+to_str(var_type_gmsh));
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////

void Reader::fix_negative_volumes(Mesh& mesh)
{
  /// @note this is now only implemented for LagrangeP2.Quad2D elements!!! others are ignored
  boost_foreach(Cells& elements, find_components_recursively<Cells>(mesh.topology()))
  {
    if (elements.element_type().derived_type_name() == "cf3.mesh.LagrangeP2.Quad2D")
    {
      Real jacobian_determinant=0;
      Uint nb_nodes_per_elem = elements.element_type().nb_nodes();
      std::vector<Uint> tmp_nodes(nb_nodes_per_elem);
      for (Uint e=0; e<elements.size(); ++e)
      {
        jacobian_determinant = elements.element_type().jacobian_determinant(elements.geometry_space().shape_function().local_coordinates().row(0),elements.geometry_space().get_coordinates(e));
        if (jacobian_determinant < 0)
        {
          // reverse the connectivity nodes order
          for (Uint n=0;n<nb_nodes_per_elem; ++n)
            tmp_nodes[n] = elements.geometry_space().connectivity()[e][n];
          if (elements.element_type().derived_type_name() == "cf3.mesh.LagrangeP2.Quad2D")
          {
            elements.geometry_space().connectivity()[e][0] = tmp_nodes[0];
            elements.geometry_space().connectivity()[e][1] = tmp_nodes[3];
            elements.geometry_space().connectivity()[e][2] = tmp_nodes[2];
            elements.geometry_space().connectivity()[e][3] = tmp_nodes[1];
            elements.geometry_space().connectivity()[e][4] = tmp_nodes[7];
            elements.geometry_space().connectivity()[e][5] = tmp_nodes[6];
            elements.geometry_space().connectivity()[e][6] = tmp_nodes[5];
            elements.geometry_space().connectivity()[e][7] = tmp_nodes[4];
            elements.geometry_space().connectivity()[e][8] = tmp_nodes[8];
          }
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // gmsh
} // mesh
} // cf3
