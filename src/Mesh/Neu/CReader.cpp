// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/regex.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/OptionT.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ConnectivityData.hpp"

#include "Mesh/Neu/CReader.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {

  using namespace Common;
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ObjectProvider < Mesh::Neu::CReader,
                             Mesh::CMeshReader,
                             Mesh::Neu::LibNeu,
                             1 >
aNeuReader_Provider ( "Neu" );

//////////////////////////////////////////////////////////////////////////////

CReader::CReader( const CName& name )
: CMeshReader(name),
  Shared(),
	m_repartition(false)
{
  BUILD_COMPONENT;
	m_property_list["Partition"].as_option().attach_trigger ( boost::bind ( &CReader::check_Partition_valid,   this ) );
	m_property_list["Repartition"].as_option().attach_trigger ( boost::bind ( &CReader::config_repartition,   this ) );
}
	
void CReader::check_Partition_valid()
{
	std::string opt; property("Partition").put_value(opt);
	if (!boost::regex_match(opt,boost::regex("nodes")) && !boost::regex_match(opt,boost::regex("elements")))
		throw ParsingFailed (FromHere(), "option \"Partition\" should be one of [ nodes , elements ]");
}

void CReader::config_repartition()
{
	property("Repartition").put_value(m_repartition);
}
	
//////////////////////////////////////////////////////////////////////////////

void CReader::defineConfigProperties ( CF::Common::PropertyList& options )
{
	options.add_option<OptionT <std::string> >("Partition","Parallel read based on elements or nodes [nodes,elements]","nodes");
	options.add_option<OptionT <Uint> >("rank","rank of this processor, temporary option",(Uint) 0);
	options.add_option<OptionT <Uint> >("number_of_processors","number_of_processors, temporary option",(Uint) 1);
	options.add_option<OptionT <bool> >("Repartition","setting this to true, puts global indexes, for repartitioning later",false);
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
     throw boost::filesystem::filesystem_error( fp.string() + " does not exist",
                                                boost::system::error_code() );
  }

  m_file_basename = boost::filesystem::basename(fp);
  
  // set the internal mesh pointer
  m_mesh = mesh;

  // create a region component inside the mesh
  CRegion& domain = m_mesh->create_domain("Base");
  m_region = domain.create_region(m_file_basename).get_type<CRegion>();
	
	
  // must be in correct order!
  read_headerData();
	
	boost::regex e_nodal_coordinates("[[:space:]]+NODAL[[:space:]]COORDINATES.+");
	boost::regex e_elements_cells("[[:space:]]+ELEMENTS/CELLS.+");
	boost::regex e_element_group("[[:space:]]+ELEMENT[[:space:]]GROUP.+");
	boost::regex e_boundary_condition("[[:space:]]+BOUNDARY[[:space:]]CONDITIONS.+");

	m_nodal_coordinates_positions.resize(0);
	m_elements_cells_positions.resize(0);
	m_element_group_positions.resize(0);
	m_boundary_condition_positions.resize(0);
	
	int p;
	std::string line;
	while (!m_file.eof())
	{
		p = m_file.tellg();
		getline(m_file,line);
		if (boost::regex_match(line,e_nodal_coordinates))
			m_nodal_coordinates_positions.push_back(p);
		else if (boost::regex_match(line,e_elements_cells))
			m_elements_cells_positions.push_back(p);
		else if (boost::regex_match(line,e_element_group))
			m_element_group_positions.push_back(p);
		else if (boost::regex_match(line,e_boundary_condition))
			m_boundary_condition_positions.push_back(p);
	}
	m_file.clear();

	Uint rank=property("rank").value<Uint>();
	Uint np=property("number_of_processors").value<Uint>();
	if (property("Partition").value<std::string>() == "nodes")
	{
		std::pair<Uint,Uint> range = std::make_pair(rank*m_headerData.NUMNP/np,(rank+1)*m_headerData.NUMNP/np-1);	
		partition_nodes(range);
	}
	else if (property("Partition").value<std::string>() == "elements")
	{
		std::pair<Uint,Uint> range = std::make_pair(rank*m_headerData.NELEM/np,(rank+1)*m_headerData.NELEM/np-1);	
		partition_elements(range);		
	}
	
	// Read nodes in parallel
	// collect elements of which all node numbers are in the given range
	
	CFinfo << "nodes to read = " << m_nodes_to_read.size() << CFendl;
	CFinfo << "elements to read = " << m_elements_to_read.size() << CFendl;

	
	read_coordinates();
	
	read_connectivity();
	
	cf_assert(m_element_group_positions.size() == m_headerData.NGRPS)
	read_groups();
  read_boundaries();
//
//
//  // Remove tmp region from component
//  //if (m_headerData.NGRPS != 1)
//  //{
	remove_component("tmp");
  m_tmp.reset();
//  //}
//  
  // Remove regions with empty connectivity tables
  remove_empty_element_regions(get_component_typed<CRegion>(*m_mesh));
	
  // truely deallocate this vector
//  std::vector<Region_TableIndex_pair>().swap (m_global_to_tmp);

  m_file.close();
	
	if (m_repartition)
	{
		set_pt_scotch_data();
	}
	
}

//////////////////////////////////////////////////////////////////////

void CReader::set_pt_scotch_data()
{
	// Partitioner data
	
	baseval=0;
	vertglbnbr = m_headerData.NUMNP;
	//edgeglbnbr = ; // total number of connections in the total mesh
	procglbnbr = property("number_of_processors").value<Uint>();
	proccnttab.resize(procglbnbr);
	procvrttab.resize(procglbnbr);
	// communicate this
	
	vertlocnbr = m_nodes_to_read.size() - m_ghost_nodes.size();
	vertgstnbr = m_nodes_to_read.size();
	
	CArray::Ptr global_node_idx = m_coordinates->get_child_type<CArray>("global_idx");
	
	
	CFinfo << " m_coordinates->size() = " << m_coordinates->size() << CFendl;
	CFinfo << " global_node_idx->size() = " << global_node_idx->size() << CFendl;
  CNodeConnectivity::Ptr node_connectivity = create_component_type<CNodeConnectivity>("node_connectivity");
	node_connectivity->initialize(recursive_range_typed<CElements>(*m_mesh));
	
	Uint start;
	Uint end = 0;
	for (Uint iNode=0; iNode<m_coordinates->size(); ++iNode)
	{
		if (m_ghost_nodes.find((*global_node_idx)[iNode][0]) == m_ghost_nodes.end())
		{
			start = end;
			std::set<Uint> nodes_vec;
			BOOST_FOREACH(const Uint elm_idx, node_connectivity->node_element_range(iNode))
			{
				const CNodeConnectivity::ElementReferenceT& elm = node_connectivity->element(elm_idx);
				CTable::ConstRow nodes = elm.first->connectivity_table()[elm.second];
				
				for (Uint n=0; n<nodes.size(); ++n)
				{
					if (nodes[n] != iNode)
					{
						if (nodes_vec.find(nodes[n])==nodes_vec.end())
						{
							nodes_vec.insert(nodes[n]);
							edgeloctab.push_back(nodes[n]);
							edgegsttab.push_back((*global_node_idx)[nodes[n]][0]);
							end++;							
						}
					}
				}			
			}
			
			vertloctab.push_back(start);
			vendloctab.push_back(end);
			
			
			CFinfo << "node " << (*global_node_idx)[iNode][0] << " is connected to nodes " << CFflush;
			for (Uint j=start; j<end; ++j)
			{
				CFinfo << edgegsttab[j] << " " << CFflush;
			}
			CFinfo << CFendl;
			
		}
		
	}
	
	print_vector(CFinfo, vertloctab, " ", "vertloctab = ","\n");
	print_vector(CFinfo, edgeloctab, " ", "edgeloctab = ","\n");
	print_vector(CFinfo, edgegsttab, " ", "edgegsttab = ","\n");
	CFinfo << CFendl;	
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_headerData()
{
  Uint NUMNP, NELEM, NGRPS, NBSETS, NDFCD, NDFVL;
  std::string line;

  // skip 6 initial lines
  for (Uint i=0; i<6; ++i)
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
	
  //m_headerData.print();
  
  getline(m_file,line);
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_coordinates()
{
	m_node_to_coord_idx.clear();
	BOOST_FOREACH(Uint file_pos, m_nodal_coordinates_positions)
	{
		m_file.seekg(file_pos,std::ios::beg);
		
		// Create the coordinates array
		m_coordinates = m_region->create_coordinates(m_headerData.NDFCD).get_type<CArray>();
		CArray::ArrayT& coordinates = m_coordinates->array();
		Uint coord_start_idx = coordinates.size();
		coordinates.resize(boost::extents[coordinates.size()+m_nodes_to_read.size()][m_headerData.NDFCD]);

		CArray::Ptr global_node_idx = m_coordinates->get_child_type<CArray>("global_idx");
		if (!global_node_idx)
		{
			CFinfo << "before" << CFendl;
			global_node_idx = m_coordinates->create_component_type<CArray>("global_idx");
			global_node_idx->initialize(1);
			CFinfo << "after" << CFendl;
		}
		global_node_idx->array().resize(boost::extents[global_node_idx->size()+m_nodes_to_read.size()][1]);
		
		std::string line;
		// skip one line
		getline(m_file,line);

		// declare and allocate one coordinate row
		std::vector<Real> rowVector(m_headerData.NDFCD);

		std::set<Uint>::const_iterator it;

		Uint coord_idx=coord_start_idx;
		for (Uint i=1; i<=m_headerData.NUMNP; ++i) 
		{
			getline(m_file,line);
			it = m_nodes_to_read.find(i);
			if (it != m_nodes_to_read.end())
			{
				// add global node index
				(*global_node_idx)[coord_idx][0] = i; // -1 because base zero
				
				std::stringstream ss(line);
				Uint nodeNumber;
				ss >> nodeNumber;
				m_node_to_coord_idx[nodeNumber]= coord_idx;
				for (Uint dim=0; dim<m_headerData.NDFCD; ++dim)
					ss >> coordinates[coord_idx][dim];
				coord_idx++;
			}
		}
		
		getline(m_file,line);
		
	}
}

//////////////////////////////////////////////////////////////////////////////

void CReader::partition_nodes(const std::pair<Uint,Uint>& range)
{
	m_nodes_to_read.clear();
	m_elements_to_read.clear();
	
	BOOST_FOREACH(Uint file_pos, m_elements_cells_positions)
	{
		m_file.seekg(file_pos,std::ios::beg);
		// skip next line
		std::string line;
		getline(m_file,line);
		
		// read every line and store the connectivity in the correct region through the buffer
		std::string etype_CF;
		for (Uint i=0; i<m_headerData.NELEM; ++i) 
		{
			// element description
			Uint elementNumber, elementType, nbElementNodes;
			m_file >> elementNumber >> elementType >> nbElementNodes;
			
			// get element nodes
			std::vector<Uint> neu_element_nodes(nbElementNodes);
			bool element_contains_nodes_in_range = false;
			for (Uint j=0; j<nbElementNodes; ++j)
			{
				m_file >> neu_element_nodes[j];
				if (neu_element_nodes[j]>=range.first && neu_element_nodes[j]<=range.second)
				{
					element_contains_nodes_in_range = true;
				}
			}
			if (element_contains_nodes_in_range)
			{
				bool is_ghost_elem = false;
				BOOST_FOREACH(const Uint node_number, neu_element_nodes)
				{
					m_nodes_to_read.insert(node_number);
					if (node_number<range.first || node_number>range.second)
					{
						m_ghost_nodes.insert(node_number);
						is_ghost_elem = true;
					}
				}
				m_ghost_elems.insert(elementNumber);
				m_elements_to_read.insert(elementNumber);
			}
			
			
			// finish the line
			getline(m_file,line);
		}
		getline(m_file,line);  // ENDOFSECTION		
	}
}

//////////////////////////////////////////////////////////////////////////////

void CReader::partition_elements(const std::pair<Uint,Uint>& range)
{
	m_nodes_to_read.clear();
	m_elements_to_read.clear();
	
	BOOST_FOREACH(Uint file_pos, m_elements_cells_positions)
	{
		m_file.seekg(file_pos,std::ios::beg);
		// skip next line
		std::string line;
		getline(m_file,line);
		
		// read every line and store the connectivity in the correct region through the buffer
		std::string etype_CF;
		for (Uint i=0; i<m_headerData.NELEM; ++i) 
		{
			// element description
			Uint elementNumber, elementType, nbElementNodes;
			m_file >> elementNumber >> elementType >> nbElementNodes;
			
			// get element nodes
			std::vector<Uint> neu_element_nodes(nbElementNodes);
			for (Uint j=0; j<nbElementNodes; ++j)
			{
				m_file >> neu_element_nodes[j];
			}
			if (i>=range.first && i<=range.second)
			{
				BOOST_FOREACH(const Uint node_number, neu_element_nodes)
					m_nodes_to_read.insert(node_number);
				m_elements_to_read.insert(elementNumber);
			}
			
			
			// finish the line
			getline(m_file,line);
		}
		getline(m_file,line);  // ENDOFSECTION		
	}
}
		
//////////////////////////////////////////////////////////////////////////////
	
void CReader::read_connectivity()
{
	m_global_to_tmp.clear();
	BOOST_FOREACH(Uint file_pos, m_elements_cells_positions)
	{
		m_file.seekg(file_pos,std::ios::beg);
			
		// make temporary regions for each element type possible
		m_tmp = create_component_type<CRegion>("tmp");

		CArray& coordinates = *m_coordinates;
		
		std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
				create_element_regions_with_buffermap(*m_tmp,coordinates,m_supported_types);

		// skip next line
		std::string line;
		getline(m_file,line);

		// read every line and store the connectivity in the correct region through the buffer
		std::string etype_CF;
		std::set<Uint>::const_iterator it;
		for (Uint i=0; i<m_headerData.NELEM; ++i) 
		{
			// element description
			Uint elementNumber, elementType, nbElementNodes;
			m_file >> elementNumber >> elementType >> nbElementNodes;
			it = m_elements_to_read.find(elementNumber);
			if (it != m_elements_to_read.end())
			{
				// find the element type
				if      (elementType==LINE  && nbElementNodes==2) etype_CF = "Line";  // quadrilateral
				else if (elementType==QUAD  && nbElementNodes==4) etype_CF = "Quad";  // quadrilateral
				else if (elementType==TRIAG && nbElementNodes==3) etype_CF = "Triag"; // triangle
				else if (elementType==HEXA  && nbElementNodes==8) etype_CF = "Hexa";  // brick
				else if (elementType==TETRA && nbElementNodes==4) etype_CF = "Tetra";
				/// @todo to be implemented
				else if (elementType==5 && nbElementNodes==6) // wedge (prism)
					throw Common::NotImplemented(FromHere(),"wedge or prism element not able to convert to COOLFluiD yet.");
				else if (elementType==7 && nbElementNodes==5) // pyramid
					throw Common::NotImplemented(FromHere(),"pyramid element not able to convert to COOLFluiD yet.");
				else {
					throw Common::NotSupported(FromHere(),"no support for element type/nodes "
																		 + String::to_str<int>(elementType) + "/" + String::to_str<int>(nbElementNodes) +
																		 " in Gambit Neutral format");
				}
				// append dimension to the element type (1D, 2D, 3D)
				etype_CF += String::to_str<int>(m_headerData.NDFCD)+"DLagrangeP1";
				
				// get element nodes
				std::vector<Uint> cf_element(nbElementNodes);
				
				for (Uint j=0; j<nbElementNodes; ++j)
				{
					Uint neu_node_number;
					Uint cf_idx = m_nodes_neu_to_cf[elementType][j];
					m_file >> neu_node_number;
					cf_element[cf_idx] = m_node_to_coord_idx[neu_node_number];
				}
				Uint table_idx = buffer[etype_CF]->add_row(cf_element);
				CElements::Ptr tmp_elements = get_named_component_typed_ptr<CElements>(*m_tmp, "elements_" + etype_CF);
				cf_assert(tmp_elements);
				m_global_to_tmp[elementNumber] = std::make_pair(tmp_elements,table_idx);
			}
			else
			{
				Uint neu_node_number;
				for (Uint j=0; j<nbElementNodes; ++j)
				{
					m_file >> neu_node_number;
				}
				
			}
			// finish the line
			getline(m_file,line);
		}
		getline(m_file,line);  // ENDOFSECTION
	}
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_groups()
{
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

//  // 2 cases:
//  // 1) there is only one group --> The tmp region can just be renamed
//  //    and put in the filesystem as subcomponent of "mesh/regions"
//  if (m_headerData.NGRPS == 1)
//  {
//    m_tmp->rename(groups[0].ELMMAT);
//    m_tmp->move_component(m_region);
//  }
//  // 2) there are multiple groups --> New regions have to be created
//  //    and the elements from the tmp region have to be distributed among
//  //    these new regions.
//  else
//  {
	// Create Region for each group
	BOOST_FOREACH(GroupData& group, groups)
	{

		CRegion& region = m_region->create_region(group.ELMMAT);

		// Create regions for each element type in each group-region
		std::map<std::string,boost::shared_ptr<CTable::Buffer> > buffer =
				create_element_regions_with_buffermap(region,coordinates,m_supported_types);

		// Copy elements from tmp_region in the correct region
		BOOST_FOREACH(Uint global_element, group.ELEM)
		{
			CElements::Ptr tmp_region = m_global_to_tmp[global_element].first;
			Uint local_element = m_global_to_tmp[global_element].second;
			boost::shared_ptr<CTable::Buffer> buf = buffer[tmp_region->element_type().getElementTypeName()];
			cf_assert(buf);
			Uint idx = buf->add_row(tmp_region->connectivity_table().array()[local_element]);
			
			std::string new_region_name = "elements_" + tmp_region->element_type().getElementTypeName();
			m_global_to_tmp[global_element] = std::make_pair(region.get_child_type<CElements>(new_region_name),idx);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

void CReader::read_boundaries()
{
  CArray& coordinates = *m_coordinates;
  
  std::string line;
  for (Uint t=0; t<m_headerData.NBSETS; ++t) {

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

    // boundary connectivity here
    //vector< GElement > e2n(NENTRY);

    CRegion& bc_region = m_region->create_region(NAME);

    // create all kind of element type regions
    BufferMap buffer = create_element_regions_with_buffermap (bc_region,coordinates,m_supported_types);

    // read boundary elements connectivity
    for (int i=0; i<NENTRY; ++i) {
      int ELEM, ETYPE, FACE;
      m_file >> ELEM >> ETYPE >> FACE;

      Uint global_element = ELEM;
			
			std::map<Uint,Region_TableIndex_pair>::iterator it = m_global_to_tmp.find(global_element);
			if (it != m_global_to_tmp.end())
			{
				CElements::Ptr tmp_region = it->second.first;
				Uint local_element = it->second.second;

				//Uint elementType = ETYPE;
				Uint faceIdx = m_faces_neu_to_cf[ETYPE][FACE];

				const ElementType& etype = tmp_region->element_type();
				const ElementType::FaceConnectivity& face_connectivity = etype.face_connectivity();
				
				// make a row of nodes
				const CTable::Row& elem_nodes = tmp_region->connectivity_table().array()[local_element];
				std::vector<Uint> row;
				row.reserve(face_connectivity.face_node_counts[faceIdx]);
				BOOST_FOREACH(const Uint& node, face_connectivity.face_node_range(faceIdx))
					row.push_back(elem_nodes[node]);

				// add the row to the buffer of the face region
				buffer[etype.face_type(faceIdx).getElementTypeName()]->add_row(row);
			}
      getline(m_file,line);  // finish the line (read new line)
    }
    getline(m_file,line);  // ENDOFSECTION

  }
}


//////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
