// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/regex.hpp>
#include <boost/mpi/collectives.hpp>
// #include <mpi.h>
// #include <ptscotch.h>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/OptionT.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ConnectivityData.hpp"

#include "Mesh/Neu/CReader.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {

  using namespace Common;
	using namespace Common::String;
	
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
	m_property_list["Repartition"].as_option().attach_trigger ( boost::bind ( &CReader::config_repartition,   this ) );
	
	m_property_list["brief"] = std::string("Gambit Neutral file mesh reader component");
	
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

void CReader::defineConfigProperties ( CF::Common::PropertyList& options )
{
	options.add_option<OptionT <bool> >("Serial Merge","New mesh will be merged with existing if mesh-names match",true);
	options.add_option<OptionT <bool> >("Unified Zones","Reads Neu Groups and splits the mesh in these subgroups",false);
	options.add_option<OptionT <Uint> >("Part","Number of the part of the mesh to read. (e.g. rank of processor)",PE::instance().is_init()?PE::instance().rank():0);
	options.add_option<OptionT <Uint> >("Number of Parts","Total number of parts. (e.g. number of processors)",PE::instance().is_init()?PE::instance().size():1);
	
	
	options.add_option<OptionT <bool> >("Repartition","setting this to true, puts global indexes, for repartitioning later",false);
	options.add_option<OptionT <Uint> >("OutputRank","shows output for the specified rank",0);
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
	
	CFinfo << "nodes to read = " << m_nodes_to_read.size() << CFendl;
	CFinfo << "elements to read = " << m_elements_to_read.size() << CFendl;

	
	read_coordinates();
	
	read_connectivity();

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
	m_mesh->properties()["nb_nodes"] = m_mesh->property("nb_nodes").value<Uint>()	+ m_headerData.NUMNP;

	// close the file
  m_file.close();
	
	
	
	
	
	
	// This is just for testing purpose

	if (m_repartition)
	{
		//if (property("Part").value<Uint>() == property("OutputRank").value<Uint>())
		{
			//set_pt_scotch_data();
		}
	}
	
	
}


//////////////////////////////////////////////////////////////////////

void CReader::set_pt_scotch_data()
{
	// Partitioner data
	CF_DEBUG_POINT;

	baseval=0;
	vertglbnbr = m_mesh->property("nb_nodes").value<Uint>();

	//edgeglbnbr = ; // total number of connections in the total mesh
	procglbnbr = property("Number of Parts").value<Uint>();
	proccnttab.resize(procglbnbr);
	procvrttab.resize(procglbnbr+1);
	
	vertlocnbr = m_nodes_to_read.size() - m_ghost_nodes.size();
	vertgstnbr = m_nodes_to_read.size();
	
	const CList<Uint>& global_node_idx = get_tagged_component_typed<CList<Uint> >(*m_coordinates,"global_node_indices");
	const CList<bool>& is_ghost = *m_coordinates->get_child_type<CList<bool> >("is_ghost");

	if (property("Part").value<Uint>() == property("OutputRank").value<Uint>())
	{
		CFLogVar(m_coordinates->size());
		CFLogVar(global_node_idx.size());
		CFLogVar(m_mesh->properties()["nb_nodes"].value<Uint>());
		
	}
	
  CNodeConnectivity::Ptr node_connectivity = create_component_type<CNodeConnectivity>("node_connectivity");
	node_connectivity->initialize(recursive_range_typed<CElements>(*m_mesh));
	
	
	// renumber global indices
	/*Uint nb_nodes = 0;
	BOOST_FOREACH(bool is_ghost_node, is_ghost.array())
	{
		if (!is_ghost_node)
			nb_nodes++;
	}
	if (PE::instance().is_init())
	boost::mpi::all_gather(PE::instance(), nb_nodes, proccnttab);
	
	Uint cnt=0;
	for (Uint p=0; p<proccnttab.size(); ++p)
	{
		procvrttab[p] = cnt;
		cnt += proccnttab[p];
	}
	procvrttab[procglbnbr] = cnt;
	
	if (property("Part").value<Uint>() == property("OutputRank").value<Uint>())
	for (Uint proc = 0; proc < PE::instance().size(); ++proc)
		CFinfo << "Process #" << proc << " has " << proccnttab[proc] << " nodes" << CFendl;
		
	if (property("Part").value<Uint>() == property("OutputRank").value<Uint>())
	{
		print_vector(CFinfo,procvrttab); CFinfo << CFendl;		
	}


	cf_assert(m_coordinates->size()==global_node_idx.size());
	Uint rank = PE::instance().rank();
	std::set<Uint> nodes_to_renumber_set;
	for (Uint iNode=0; iNode<m_coordinates->size(); ++iNode)
	{
		if (!is_ghost[iNode])
		{
			if (global_node_idx[iNode] < procvrttab[rank] || global_node_idx[iNode] >= procvrttab[rank+1])
			{
				nodes_to_renumber_set.insert(global_node_idx[iNode]);
			}
		}
	}
	
	std::vector<Uint> nodes_to_renumber;
	nodes_to_renumber.reserve(nodes_to_renumber_set.size());
	BOOST_FOREACH(Uint node, nodes_to_renumber_set)
		nodes_to_renumber.push_back(node);
	
	if (PE::instance().is_init())
	PE::instance().barrier();

	//if (property("Part").value<Uint>() == property("OutputRank").value<Uint>())
	{
		CFinfo << "nodes to renumber on proc " << rank << " = ";
		BOOST_FOREACH(Uint node, nodes_to_renumber)
		CFinfo << node << " " ;
		CFinfo << CFendl;
	}
	
	if (PE::instance().is_init())
	PE::instance().barrier();
	
	
	CFinfo << "before renumbering: global nodes on proc " << rank << " = ";
	BOOST_FOREACH(Uint node, global_node_idx.array())
	CFinfo << node << " " ;
	CFinfo << CFendl;
	PE::instance().barrier();
	
	
	for (Uint proc=0; proc<PE::instance().size(); ++proc)
	{
		// renumber everywhere the ones that processor "proc" wants
		std::vector<Uint> nodes_to_renumber_from_proc;
		if (proc == PE::instance().rank())
		{
			nodes_to_renumber_from_proc = nodes_to_renumber;
		}
		
		boost::mpi::broadcast(PE::instance(),nodes_to_renumber_from_proc,proc);
		
		Uint cnt = procvrttab[proc+1] - nodes_to_renumber_from_proc.size();
		
		CFinfo << "proc #" << proc << " : " ;
		print_vector(CFinfo,nodes_to_renumber_from_proc); CFinfo << CFendl;		
		PE::instance().barrier();

		
		BOOST_FOREACH(Uint node, nodes_to_renumber_from_proc)
		{
			//CFinfo << "proc " << rank << " dealing with " << node << CFendl;
			Uint found = false;
			BOOST_FOREACH(CList<Uint>& glb_node_list, recursive_filtered_range_typed< CList<Uint> >(*m_region,IsComponentTag("global_node_indices")))
			{
				if (!found)
				{
					BOOST_FOREACH(Uint& glb_node, glb_node_list.array())
					{
						if (glb_node == node && (glb_node < procvrttab[rank] || glb_node >= procvrttab[rank+1]))					
						{
							//CFinfo << "proc " << rank << " will renumber glb_node " << glb_node << " to " << cnt << CFendl;
							glb_node = cnt;
							found = true;
							break;
						}
					}					
				}
			}
			PE::instance().barrier();
			cnt++;
		}

	}
	*/
	/*
	for (Uint node=0; node<procvrttab[procglbnbr+1]; ++node)
	{
		BOOST_FOREACH(CList<Uint>& glb_node_list, recursive_filtered_range_typed< CList<Uint> >(*m_region,IsComponentTag("global_node_indices")))
		{
			BOOST_FOREACH(Uint& glb_node, glb_node_list.array())
			{
				if (glb_node == node && (glb_node < procvrttab[rank] || glb_node >= procvrttab[rank+1]))					
				{
					CFinfo << "proc " << rank << " will renumber glb_node " << glb_node << " to " << cnt << CFendl;
					glb_node = cnt;
				}
			}
		}
		if (PE::instance().is_init())
		PE::instance().barrier();
	}*/
	
	PE::instance().barrier();

	CFinfo << "global nodes on proc " << PE::instance().rank() << " = ";
	BOOST_FOREACH(Uint node, global_node_idx.array())
	CFinfo << node << " " ;
	CFinfo << CFendl;
	PE::instance().barrier();

//	
//	for (Uint iNode=0; iNode<m_coordinates->size(); ++iNode)
//	{
//		if (iNode == 0 || iNode == 16)
//		{
//			CFinfo << iNode << " --> \n";
//			BOOST_FOREACH(const Uint elm_idx, node_connectivity->node_element_range(iNode))
//			{
//				const CNodeConnectivity::ElementReferenceT& elm = node_connectivity->element(elm_idx);
//				
//				CFinfo << "   "<<elm_idx << "("<<elm.first->full_path().string()<<","<<elm.second<<")\n";
//				CTable::ConstRow nodes = elm.first->connectivity_table()[elm.second];
//				CFinfo << "          ";
//				BOOST_FOREACH(const Uint node, nodes)
//				{
//					CFinfo << node << " ";
//				}
//				CFinfo << CFendl;
//			}
//			CFinfo << CFendl;					
//		}
//	}
//	
	
	
	if (property("Part").value<Uint>() == property("OutputRank").value<Uint>())
	{

	vertloctab.resize(0);
	vendloctab.resize(0);
	edgeloctab.resize(0);
	edgegsttab.resize(0);
	Uint start;
	Uint end = 0;
	for (Uint iNode=0; iNode<m_coordinates->size(); ++iNode)
	{
		// if the node is not a ghost node
		if (! is_ghost[iNode] )
		{
			start = end;
			std::set<Uint> nodes_vec;
			
			// Find all the elements this node is contained in
			BOOST_FOREACH(const Uint elm_idx, node_connectivity->node_element_range(iNode))
			{
				const CNodeConnectivity::ElementReferenceT& elm = node_connectivity->element(elm_idx);
				CTable::ConstRow nodes = elm.first->connectivity_table()[elm.second];
				
				BOOST_FOREACH(const Uint node, nodes)
				{
					if (node != iNode)
					{
						if (nodes_vec.find(node)==nodes_vec.end())
						{
							nodes_vec.insert(node);
							edgeloctab.push_back(node);
							edgegsttab.push_back(global_node_idx[node]);
							end++;							
						}
					}
				}			
			}
			
			vertloctab.push_back(start);
			vendloctab.push_back(end);
			
			
			CFinfo << "node " << global_node_idx[iNode] << " is connected to nodes " << CFflush;
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
	remove_component(node_connectivity->name());
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
	
	m_file >> m_headerData.mesh_name; 	getline(m_file,line);
	
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
	
	//m_mesh->properties()["nb_cells"] = m_mesh->properties()["nb_cells"].value<Uint>() + NELEM;
	//m_mesh->properties()["nb_nodes"] = m_mesh->properties()["nb_nodes"].value<Uint>()		+ NUMNP;
  //m_headerData.print();
  
  getline(m_file,line);
}

//////////////////////////////////////////////////////////////////////////////
	
void CReader::get_file_positions()
{	
	boost::regex e_nodal_coordinates("[[:space:]]+NODAL[[:space:]]COORDINATES.+");
	boost::regex e_elements_cells("[[:space:]]+ELEMENTS/CELLS.+");
	boost::regex e_element_group("[[:space:]]+ELEMENT[[:space:]]GROUP.+");
	boost::regex e_boundary_condition("[[:space:]]+BOUNDARY[[:space:]]CONDITIONS.+");
	
	m_element_group_positions.resize(0);
	m_boundary_condition_positions.resize(0);
	
	int p;
	std::string line;
	while (!m_file.eof())
	{
		p = m_file.tellg();
		getline(m_file,line);
		if (boost::regex_match(line,e_nodal_coordinates))
			m_nodal_coordinates_position=p;
		else if (boost::regex_match(line,e_elements_cells))
			m_elements_cells_position=p;
		else if (boost::regex_match(line,e_element_group))
			m_element_group_positions.push_back(p);
		else if (boost::regex_match(line,e_boundary_condition))
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
	std::pair<Uint,Uint> range = std::make_pair(m_headerData.NUMNP/np * p ,(p == np-1 ? m_headerData.NUMNP : m_headerData.NUMNP/np*(p+1)));	
	
	m_nodes_to_read.clear();
	m_elements_to_read.clear();
	m_ghost_nodes.clear();
	
	m_file.seekg(m_elements_cells_position,std::ios::beg);
	// skip next line
	std::string line;
	getline(m_file,line);
	
	// read every line and store the connectivity in the correct region through the buffer
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
			if (neu_element_nodes[j]>=range.first+1 && neu_element_nodes[j]<range.second+1)
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
				if (node_number<range.first+1 || node_number>=range.second+1)
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
		
//////////////////////////////////////////////////////////////////////////////
	
void CReader::read_connectivity()
{
	m_tmp = m_region->create_region("main").get_type<CRegion>();

	m_global_to_tmp.clear();
	m_file.seekg(m_elements_cells_position,std::ios::beg);
		
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
		
		etype_CF = element_type(elementType,nbElementNodes);
		
		it = m_elements_to_read.find(elementNumber);
		if (it != m_elements_to_read.end())
		{
			
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
	
	m_node_to_coord_idx.clear();
}

//////////////////////////////////////////////////////////////////////////////

std::string CReader::element_type(const Uint neu_type, const Uint nb_nodes)
{
	std::string cf_type;
	std::string dim = to_str<int>(m_headerData.NDFCD);
	if      (neu_type==LINE  && nb_nodes==2) cf_type = "Line"  + dim + "DLagrangeP1";  // line
	else if (neu_type==QUAD  && nb_nodes==4) cf_type = "Quad"  + dim + "DLagrangeP1";  // quadrilateral
	else if (neu_type==TRIAG && nb_nodes==3) cf_type = "Triag" + dim + "DLagrangeP1";  // triangle
	else if (neu_type==HEXA  && nb_nodes==8) cf_type = "Hexa"  + dim + "DLagrangeP1";  // hexahedron
	else if (neu_type==TETRA && nb_nodes==4) cf_type = "Tetra" + dim + "DLagrangeP1";  // tetrahedron
	/// @todo to be implemented
	else if (neu_type==5 && nb_nodes==6) // wedge (prism)
		throw Common::NotImplemented(FromHere(),"wedge or prism element not able to convert to COOLFluiD yet.");
	else if (neu_type==7 && nb_nodes==5) // pyramid
		throw Common::NotImplemented(FromHere(),"pyramid element not able to convert to COOLFluiD yet.");
	else {
		throw Common::NotSupported(FromHere(),"no support for element type/nodes "
															 + to_str<int>(neu_type) + "/" + to_str<int>(nb_nodes) +
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
		
		CFinfo << "region " << region.full_path().string() << " created" << CFendl;
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

	m_region->remove_component(m_tmp->name());
	m_tmp.reset();
	
}
	
//////////////////////////////////////////////////////////////////////////////

void CReader::read_boundaries()
{
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
				Uint faceIdx = m_faces_neu_to_cf[ETYPE][FACE];

				const ElementType& etype = tmp_region->element_type();
				const ElementType::FaceConnectivity& face_connectivity = etype.face_connectivity();
				
				// make a row of nodes
				const CTable::Row& elem_nodes = tmp_region->connectivity_table()[local_element];
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
