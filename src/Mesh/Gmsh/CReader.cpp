// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/OptionT.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/StringConversion.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ConnectivityData.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CMixedHash.hpp"
#include "Mesh/CHash.hpp"

#include "Mesh/Gmsh/CReader.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < Gmsh::CReader, CMeshReader, LibGmsh> aGmshReader_Builder;

//////////////////////////////////////////////////////////////////////////////

CReader::CReader( const std::string& name )
: CMeshReader(name),
  Shared()
{

  // options

  m_properties.add_option<OptionT <Uint> >("part","Part","Number of the part of the mesh to read. (e.g. rank of processor)",mpi::PE::instance().is_init()?mpi::PE::instance().rank():0);
  m_properties.add_option<OptionT <Uint> >("nb_partitions","Number of Parts","Total number of parts. (e.g. number of processors)",mpi::PE::instance().is_init()?mpi::PE::instance().size():1);



  // properties

  m_properties["brief"] = std::string("Gmsh file reader component");

  std::string desc;
  desc += "This component can read in parallel.\n";
  desc += "It can also read multiple files in serial, combining them in one large mesh.\n";
  desc += "Available coolfluid-element types are:\n";
  BOOST_FOREACH(const std::string& supported_type, m_supported_types)
  desc += "  - " + supported_type + "\n";
  m_properties["description"] = desc;
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

  //Create a hash
  m_hash = create_component<CMixedHash>("hash");
  std::vector<Uint> num_obj(2);
  num_obj[0] = m_total_nb_nodes;
  num_obj[1] = m_total_nb_elements;
  m_hash->configure_property("Number of Objects",num_obj);

  // Create a region component inside the mesh with a generic mesh name
  // NOTE: since gmsh contains several 'physical entities' in one mesh, we create one region per physical entity
  m_region = m_mesh->topology().create_region("main").as_ptr<CRegion>();


  find_ghost_nodes();

//  CFinfo << m_mesh->tree() << CFendl;
//  CFinfo << "nodes to read = " << m_nodes_to_read.size() << CFendl;
  read_coordinates();

  read_connectivity();

//  // clean-up

//  // update the number of cells and nodes in the mesh
  m_mesh->properties()["nb_cells"] = m_mesh->property("nb_cells").value<Uint>() + m_total_nb_elements;
  m_mesh->properties()["nb_nodes"] = m_mesh->property("nb_nodes").value<Uint>() + m_total_nb_nodes;

  // close the file
  m_file.close();

}

//////////////////////////////////////////////////////////////////////////////

void CReader::get_file_positions()
{

  std::string region_names("$PhysicalNames");
  std::string nodes("$Nodes");
  std::string elements("$Elements");

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

      std::string tempstr;
      m_mesh_dimension = DIM_1D;
      for(Uint ir = 0; ir < m_nb_regions; ++ir)
      {
          m_file >> m_region_list[ir].dim;
          m_mesh_dimension = std::max(m_region_list[ir].dim,m_mesh_dimension);
          m_file >> m_region_list[ir].index;
          //The original name of the region in the mesh file has quotes, we want to strip them off
          m_file >> tempstr;
          m_region_list[ir].name = tempstr.substr(1,tempstr.length()-2);
      }
    }
    else if (line.find(nodes)!=std::string::npos) {
      m_coordinates_position=p;
      m_file >> m_total_nb_nodes;
//      CFinfo << "The total number of nodes is " << m_total_nb_nodes << CFendl;
    }
    else if (line.find(elements)!=std::string::npos)
    {
      m_elements_position = p;
      m_file >> m_total_nb_elements;
//      CFinfo << "The total number of elements is " << m_total_nb_elements << CFendl;

      Uint elem_idx, elem_type, nb_tags, phys_tag;

      //Let's count how many elements of each type are present
      for(Uint ie = 0; ie < m_total_nb_elements; ++ie)
      {
          m_file >> elem_idx;
          m_file >> elem_type;
          m_file >> nb_tags;
          m_file >> phys_tag;
          getline(m_file,line);
          (m_nb_gmsh_elem_in_region[phys_tag-1])[elem_type]++;
      }
    }

  }
  m_file.clear();

//  CFinfo << "Mesh dimension: " << m_mesh_dimension << CFendl;
//  CFinfo << "The number of regions is: " << m_nb_regions << CFendl;
//  CFinfo << "Region list" << CFendl;
//  for(Uint ir = 0; ir < m_nb_regions; ++ir)
//  {
//    CFinfo << m_region_list[ir].dim << " " << m_region_list[ir].index << " " << m_region_list[ir].name;
//    CFinfo << CFendl;
//  }

//  CFinfo << "Element types present:" << CFendl;
//  for(Uint ir = 0; ir < m_nb_regions; ++ir)
//  {
//    CFinfo << "Region: " << m_region_list[ir].name << CFendl;
//    for(Uint etype = 0; etype < Shared::nb_gmsh_types; ++etype)
//    {
//      if ((m_nb_gmsh_elem_in_region[ir])[etype] > 0)
//      {
//        CFinfo << "\ttype: " << etype << ", number of elements: " << (m_nb_gmsh_elem_in_region[ir])[etype] << CFendl;
//      }

//    }
//  }


}

//////////////////////////////////////////////////////////////////////////////

void CReader::find_ghost_nodes()
{
  m_ghost_nodes.clear();

  // Only find ghost nodes if the domain is split up
  if (property("nb_partitions").value<Uint>() > 1)
  {
    m_file.seekg(m_elements_position,std::ios::beg);
    // skip next line
    std::string line;
    getline(m_file,line);

    //Skip the line containing the actual number of elements
    getline(m_file,line);


    // read every line and store the connectivity in the correct region through the buffer
    Uint elementNumber, elementType, nbElementNodes;
    for (Uint i=0; i<m_total_nb_elements; ++i)
    {
      if (m_total_nb_elements > 100000)
      {
        if(i%(m_total_nb_elements/20)==0)
          CFinfo << 100*i/m_total_nb_elements << "% " << CFendl;
      }

      if (m_hash->subhash(ELEMS)->owns(i))
      {
        // element description
        m_file >> elementNumber >> elementType;
        nbElementNodes = Shared::m_nodes_in_gmsh_elem[elementType];

        // check if element nodes are ghost
        std::vector<Uint> gmsh_element_nodes(nbElementNodes);
        for (Uint j=0; j<nbElementNodes; ++j)
        {
          m_file >> gmsh_element_nodes[j];
          if (!m_hash->subhash(NODES)->owns(gmsh_element_nodes[j]-1))
          {
            m_ghost_nodes.insert(gmsh_element_nodes[j]);
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

void CReader::read_coordinates()
{

  Uint global_start_idx = m_mesh->properties()["nb_nodes"].value<Uint>();

  m_file.seekg(m_coordinates_position,std::ios::beg);

  // Find the region which has the highest dimensionality present in the mesh:
  Uint master_region = 0;
  while((m_region_list[master_region].dim != m_mesh_dimension) && (master_region < m_nb_regions))
  {
     master_region++;
  }

  // Create the coordinates array
  m_nodes = m_region->create_nodes(m_mesh_dimension).as_ptr<CNodes>();


  Uint nodes_start_idx = m_nodes->size();
  m_nodes->resize(nodes_start_idx + m_hash->subhash(NODES)->nb_objects_in_part(mpi::PE::instance().rank()) + m_ghost_nodes.size());

  std::string line;
  //Skip the line with keyword '$Nodes':
  getline(m_file,line);
  // skip one line, which says how many (total) nodes are  present in the mesh
  getline(m_file,line);


  // declare and allocate one coordinate row
//  std::vector<Real> rowVector(m_mesh_dimension);

  std::set<Uint>::const_iterator it;

  Uint coord_idx=nodes_start_idx;

  for (Uint node_idx=1; node_idx<=m_total_nb_nodes; ++node_idx)
  {
    if (m_total_nb_nodes > 100000)
    {
      if(node_idx%(m_total_nb_nodes/20)==0)
        CFinfo << 100*node_idx/m_total_nb_nodes << "% " << CFendl;
    }
    getline(m_file,line);

    if (m_hash->subhash(NODES)->owns(node_idx-1))
    {
      m_nodes->is_ghost()[coord_idx] = false;
      m_node_to_coord_idx[node_idx]=coord_idx;
      std::stringstream ss(line);
      Uint nodeNumber;
      ss >> nodeNumber;
      for (Uint dim=0; dim<m_mesh_dimension; ++dim)
        ss >> m_nodes->coordinates()[coord_idx][dim];
      if(m_mesh_dimension < DIM_3D) getline(ss,line); //Gmsh always stores 3 coordinates, even for 2D meshes
      coord_idx++;
    }
    else
    {
      it = m_ghost_nodes.find(node_idx);
      if (it != m_ghost_nodes.end())
      {
        // add global node index
        m_nodes->is_ghost()[coord_idx] = true;
        m_node_to_coord_idx[node_idx]=coord_idx;
        std::stringstream ss(line);
        Uint nodeNumber;
        ss >> nodeNumber;
        for (Uint dim=0; dim<m_mesh_dimension; ++dim)
        ss >> m_nodes->coordinates()[coord_idx][dim];
        if(m_mesh_dimension < DIM_3D) getline(ss, line);

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
            CFinfo << m_nodes->coordinates()[i][dim] << " ";
        }
      CFinfo << CFendl;
  }
  */

  getline(m_file,line);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_connectivity()
{

 //Each entry of this vector holds a map (gmsh_type_idx, pointer to connectivity table of this gmsh type).
 //Each row corresponds to one region of the mesh
 std::vector<std::map<Uint, CEntities* > > conn_table_idx;
 conn_table_idx.resize(m_nb_regions);
 for(Uint ir = 0; ir < m_nb_regions; ++ir)
 {
    conn_table_idx[ir].clear();
 }

 std::map<Uint, CEntities*>::iterator elem_table_iter;

 //Loop over all regions and allocate a connectivity table of proper size for each element type that
 //is present in each region. Counting of elements was done during the first pass in the function
 //get_file_positions
 for(Uint ir = 0; ir < m_nb_regions; ++ir)
 {
   // create new region
   CRegion::Ptr new_region = m_region->create_region(m_region_list[ir].name).as_ptr<CRegion>();

   // Take the gmsh element types present in this region and generate new names of elements which correspond
   // to coolfuid naming:
   for(Uint etype = 0; etype < Shared::nb_gmsh_types; ++etype)
     if((m_nb_gmsh_elem_in_region[ir])[etype] > 0)
     {
       const std::string cf_elem_name = Shared::gmsh_name_to_cf_name(m_mesh_dimension,etype);

       ElementType::Ptr allocated_type = create_component_abstract_type<ElementType>(cf_elem_name,"tmp");
       CEntities::Ptr elements;
       if (allocated_type->dimensionality() == allocated_type->dimension()-1)
         elements = create_component_abstract_type<CEntities>("CF.Mesh.CFaces",allocated_type->shape_name());
       else
         elements = create_component_abstract_type<CEntities>("CF.Mesh.CCells",allocated_type->shape_name());
      new_region->add_component(elements);
      elements->initialize(cf_elem_name,*m_nodes);

      // Celements& elements = new_region->create_component<CElements>(cf_elem_name);
      // elements.initialize(cf_elem_name,*m_nodes);

       CTable<Uint>& elem_table = elements->as_ptr<CElements>()->connectivity_table();
       elem_table.set_row_size(Shared::m_nodes_in_gmsh_elem[etype]);
       elem_table.resize((m_nb_gmsh_elem_in_region[ir])[etype]);

       CList<Uint>& global_node_idx = elements->glb_idx();
       global_node_idx.resize((m_nb_gmsh_elem_in_region[ir])[etype]);

       conn_table_idx[ir].insert(std::pair<Uint,CEntities*>(etype,elements.get()));
     }


 }

   std::string etype_CF;
   std::set<Uint>::const_iterator it;
   std::vector<Uint> cf_element;
   Uint gmsh_node_number, nb_tags, phys_tag, other_tag;
   Uint cf_node_number;
   Uint cf_idx;
   Uint table_idx;

   m_node_to_glb_elements.resize(m_nodes->size());
   m_global_to_tmp.clear();
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
    Uint element_number, gmsh_element_type, nb_element_nodes;
    m_file >> element_number >> gmsh_element_type;

    nb_element_nodes = Shared::m_nodes_in_gmsh_elem[gmsh_element_type];

    // get element nodes
    if (m_hash->subhash(ELEMS)->owns(i))
    {
      m_file >> nb_tags;
      m_file >> phys_tag;
      for(Uint itag = 0; itag < (nb_tags-1); ++itag)
          m_file >> other_tag;

//      CFinfo << "Reading element " << element_number << " of type " << gmsh_element_type;
//      CFinfo << " in region " << phys_tag << " with " << nb_element_nodes << " nodes " << CFendl;

      cf_element.resize(nb_element_nodes);
      for (Uint j=0; j<nb_element_nodes; ++j)
      {
        cf_idx = m_nodes_gmsh_to_cf[gmsh_element_type][j];
        m_file >> gmsh_node_number;
        cf_node_number = m_node_to_coord_idx[gmsh_node_number];
        cf_element[cf_idx] = cf_node_number;
      }
      elem_table_iter = conn_table_idx[phys_tag-1].find(gmsh_element_type);
      const Uint row_idx = (m_nb_gmsh_elem_in_region[phys_tag-1])[gmsh_element_type];
      CTable<Uint>::Row element_nodes = (*(elem_table_iter->second)).as_ptr<CElements>()->connectivity_table()[row_idx];

      for(Uint node = 0; node < nb_element_nodes; ++node)
      {
         element_nodes[node] = cf_element[node];
      }

      //Set the global index of this element
      (*(elem_table_iter->second)).glb_idx()[row_idx] = element_number;

      (m_nb_gmsh_elem_in_region[phys_tag-1])[gmsh_element_type]++;

    }

    else
    {
      for (Uint j=0; j<nb_element_nodes; ++j)
      {
        m_file >> gmsh_node_number;
      }
    }

    // finish the line
    getline(m_file,line);
  }
  getline(m_file,line);  // ENDOFSECTION


  m_node_to_coord_idx.clear();

}

//////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF
