// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/algorithm/string/erase.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/OptionT.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/Foreach.hpp"
#include "Common/StringConversion.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CMixedHash.hpp"
#include "Mesh/CHash.hpp"
#include "Mesh/CElements.hpp"

#include "Mesh/Neu/CReader.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < Neu::CReader, CMeshReader, LibNeu > aNeuReader_Builder;

//////////////////////////////////////////////////////////////////////////////

CReader::CReader( const std::string& name )
: CMeshReader(name),
  Shared()
{
  // options
  m_properties.add_option<OptionT <bool> >("read_groups","Unified Zones","Reads Neu Groups and splits the mesh in these subgroups",true);
  m_properties.add_option<OptionT <Uint> >("part","Part","Number of the part of the mesh to read. (e.g. rank of processor)",mpi::PE::instance().rank());
  m_properties.add_option<OptionT <Uint> >("nb_partitions","Total nb_partitions. (e.g. number of processors)",mpi::PE::instance().size());
  m_properties.add_option<OptionT <bool> >("read_boundaries","Read Boundaries","Read the surface elements for the boundary",true);


  m_properties["brief"] = std::string("Neutral file mesh reader component");

  std::string desc;
  desc += "This component can read in parallel.\n";
  desc += "Available coolfluid-element types are:\n";
  boost_foreach(const std::string& supported_type, m_supported_types)
  desc += "  - " + supported_type + "\n";
  m_properties["description"] = desc;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CReader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".neu");
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

  // set the internal mesh pointer
  m_mesh = mesh;

  // Read file once and store positions
  get_file_positions();

  // Read mesh information
  read_headerData();

  // Create a hash
	m_hash = create_component<CMixedHash>("hash");
	std::vector<Uint> num_obj(2);
	num_obj[0] = m_headerData.NUMNP;
	num_obj[1] = m_headerData.NELEM;
	m_hash->configure_property("Number of Objects",num_obj);

  // Create a region component inside the mesh with the name mesh_name
  //if (property("new_api").value<bool>())
    m_region = m_mesh->topology().create_region(m_headerData.mesh_name).as_ptr<CRegion>();
  //else
  //  m_region = m_mesh->create_region(m_headerData.mesh_name,!property("Serial Merge").value<bool>()).as_ptr<CRegion>();

  find_ghost_nodes();
  read_coordinates();
  read_connectivity();
  if (property("read_boundaries").value<bool>())
    read_boundaries();

  if (property("read_groups").value<bool>())
    read_groups();

  // clean-up
  // --------
  // Remove regions with empty connectivity tables
  remove_empty_element_regions(m_mesh->topology());

  // update the number of cells and nodes in the mesh
//  m_mesh->properties()["nb_cells"] = m_headerData.NELEM;
//  m_mesh->properties()["nb_nodes"] = m_headerData.NUMNP;

  // close the file
  m_file.close();
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

void CReader::find_ghost_nodes()
{
  m_ghost_nodes.clear();

  // Only find ghost nodes if the domain is split up
  if (property("nb_partitions").value<Uint>() > 1)
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

      if (m_hash->subhash(ELEMS)->owns(i))
      {
        // element description
        m_file >> elementNumber >> elementType >> nbElementNodes;

        // check if element nodes are ghost
        std::vector<Uint> neu_element_nodes(nbElementNodes);
        for (Uint j=0; j<nbElementNodes; ++j)
        {
          m_file >> neu_element_nodes[j];
          if (!m_hash->subhash(NODES)->owns(neu_element_nodes[j]-1))
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

void CReader::read_coordinates()
{
  m_file.seekg(m_nodal_coordinates_position,std::ios::beg);

  // Create the nodes
  m_nodes = m_region->create_nodes(m_headerData.NDFCD).as_ptr<CNodes>();

  m_nodes->resize(m_hash->subhash(NODES)->nb_objects_in_part(mpi::PE::instance().rank()) + m_ghost_nodes.size());
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
    if (m_hash->subhash(NODES)->owns(node_idx-1))
    {
      m_nodes->is_ghost()[coord_idx] = false;
      m_node_to_coord_idx[node_idx]=coord_idx;
      std::stringstream ss(line);
      Uint nodeNumber;
      ss >> nodeNumber;
      for (Uint dim=0; dim<m_headerData.NDFCD; ++dim)
        ss >> m_nodes->coordinates()[coord_idx][dim];
      coord_idx++;
    }
    else
    {
      if (m_ghost_nodes.find(node_idx) != not_found)
      {
        // add global node index
        m_nodes->is_ghost()[coord_idx] = true;
        m_node_to_coord_idx[node_idx]=coord_idx;
        std::stringstream ss(line);
        Uint nodeNumber;
        ss >> nodeNumber;
        for (Uint dim=0; dim<m_headerData.NDFCD; ++dim)
          ss >> m_nodes->coordinates()[coord_idx][dim];
        coord_idx++;
      }
    }
  }
  getline(m_file,line);
}


//////////////////////////////////////////////////////////////////////////////

void CReader::read_connectivity()
{
  m_tmp = m_region->create_region("main").as_ptr<CRegion>();

  m_global_to_tmp.clear();
  m_file.seekg(m_elements_cells_position,std::ios::beg);

  std::map<std::string,CElements::Ptr> elements = create_cells_in_region(*m_tmp,*m_nodes,m_supported_types);
  std::map<std::string,CTable<Uint>::Buffer::Ptr> buffer = create_connectivity_buffermap(elements);

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
    if (m_hash->subhash(ELEMS)->owns(i))
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

void CReader::read_groups()
{

  cf_assert(m_element_group_positions.size() == m_headerData.NGRPS)

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
			if (m_hash->subhash(ELEMS)->owns(I-1))
				nb_elems_in_group++;
		}
		// now allocate and read again
		groups[g].ELEM.reserve(nb_elems_in_group);
		m_file.seekg(p,std::ios::beg);
		for (Uint i=0; i<NELGP; ++i)
		{
			m_file >> I;
			if (m_hash->subhash(ELEMS)->owns(I-1))
				groups[g].ELEM.push_back(I);     // set element index
		}

    getline(m_file,line);  // finish the line (read new line)
    getline(m_file,line);  // ENDOFSECTION
  }

  // Create Region for each group
  boost_foreach(GroupData& group, groups)
  {

    CRegion& region = m_region->create_region(group.ELMMAT);

    //CFinfo << "region " << region.full_path().string() << " created" << CFendl;
    // Create regions for each element type in each group-region
    std::map<std::string,CElements::Ptr> elements = create_cells_in_region(region,*m_nodes,m_supported_types);
    std::map<std::string,CTable<Uint>::Buffer::Ptr> buffer = create_connectivity_buffermap(elements);

    // Copy elements from tmp_region in the correct region
    boost_foreach(Uint global_element, group.ELEM)
    {
      CElements::Ptr tmp_elems = m_global_to_tmp[global_element].first;
      Uint local_element = m_global_to_tmp[global_element].second;
      std::string etype = tmp_elems->element_type().builder_name();

      Uint idx = buffer[etype]->add_row(tmp_elems->connectivity_table().array()[local_element]);
      std::string new_elems_name = tmp_elems->name();
      m_global_to_tmp[global_element] = std::make_pair(region.get_child_ptr(new_elems_name)->as_ptr<CElements>(),idx);
    }
  }

  m_region->remove_component(m_tmp->name());
  m_tmp.reset();

}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_boundaries()
{
  cf_assert(m_boundary_condition_positions.size() == m_headerData.NBSETS)

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
    std::map<std::string,CElements::Ptr> elements = create_faces_in_region (bc_region,*m_nodes,m_supported_types);
    std::map<std::string,CTable<Uint>::Buffer::Ptr> buffer = create_connectivity_buffermap (elements);

    // read boundary elements connectivity
    for (int i=0; i<NENTRY; ++i)
    {
      int ELEM, ETYPE, FACE;
      m_file >> ELEM >> ETYPE >> FACE;

      Uint global_element = ELEM;

      std::map<Uint,Region_TableIndex_pair>::iterator it = m_global_to_tmp.find(global_element);
      if (it != m_global_to_tmp.end())
      {
        CElements::Ptr tmp_elements = it->second.first;
        Uint local_element = it->second.second;

        //Uint elementType = ETYPE;
        Uint faceIdx = m_faces_neu_to_cf[ETYPE][FACE];

        const ElementType& etype = tmp_elements->element_type();
        const ElementType::FaceConnectivity& face_connectivity = etype.face_connectivity();

        // make a row of nodes
        const CTable<Uint>::Row& elem_nodes = tmp_elements->connectivity_table()[local_element];
        std::vector<Uint> row;
        row.reserve(face_connectivity.face_node_counts[faceIdx]);
        boost_foreach(const Uint& node, face_connectivity.face_node_range(faceIdx))
          row.push_back(elem_nodes[node]);

        // add the row to the buffer of the face region
        std::string face_type = etype.face_type(faceIdx).builder_name();
        buffer[face_type]->add_row(row);

      }
      getline(m_file,line);  // finish the line (read new line)
    }
    getline(m_file,line);  // ENDOFSECTION

  }
}

//////////////////////////////////////////////////////////////////////////////

std::string CReader::element_type(const Uint neu_type, const Uint nb_nodes)
{
  std::string cf_type;
  std::string dim = to_str<int>(m_headerData.NDFCD);
  if      (neu_type==LINE  && nb_nodes==2) cf_type = "CF.Mesh.SF.Line"  + dim + "DLagrangeP1";  // line
  else if (neu_type==QUAD  && nb_nodes==4) cf_type = "CF.Mesh.SF.Quad"  + dim + "DLagrangeP1";  // quadrilateral
  else if (neu_type==TRIAG && nb_nodes==3) cf_type = "CF.Mesh.SF.Triag" + dim + "DLagrangeP1";  // triangle
  else if (neu_type==HEXA  && nb_nodes==8) cf_type = "CF.Mesh.SF.Hexa"  + dim + "DLagrangeP1";  // hexahedron
  else if (neu_type==TETRA && nb_nodes==4) cf_type = "CF.Mesh.SF.Tetra" + dim + "DLagrangeP1";  // tetrahedron
  /// @todo to be implemented
  else if (neu_type==5 && nb_nodes==6) // wedge (prism)
    throw Common::NotImplemented(FromHere(),"wedge or prism element not able to convert to COOLFluiD yet.");
  else if (neu_type==7 && nb_nodes==5) // pyramid
    throw Common::NotImplemented(FromHere(),"pyramid element not able to convert to COOLFluiD yet.");
  else {
    throw Common::NotSupported(FromHere(),"no support for element type/nodes "
                               + to_str<int>(neu_type) + "/" + to_str<int>(nb_nodes) +
                               " in Neutral format");
  }

  return cf_type;
}

//////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
