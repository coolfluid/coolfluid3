// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::CField"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/mpi/collectives.hpp>

#include <mpi.h>
#include <ptscotch.h>

#include "Common/ConfigObject.hpp"
#include "Common/Log.hpp"
#include "Common/StreamHelpers.hpp"

#include "Common/MPI/PEInterface.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ConnectivityData.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct PTScotchTests_Fixture
{
  /// common setup for each test case
  PTScotchTests_Fixture()
  {
		// uncomment if you want to use arguments to the test executable
		m_argc = boost::unit_test::framework::master_test_suite().argc;
		m_argv = boost::unit_test::framework::master_test_suite().argv;

		
		baseval=0;
		
  }

  /// common tear-down for each test case
  ~PTScotchTests_Fixture()
  {

  }

  /// possibly common functions used on the tests below

	SCOTCH_Num* to_ptr(std::vector<SCOTCH_Num>& vec)
	{
		if (vec.empty())
			return NULL;
		else
			return &vec[0];
	}
	

  /// common values accessed by all tests goes here
	SCOTCH_Dgraph graph;

	SCOTCH_Num baseval;								  // first index of an array starts with 0 for c++
	SCOTCH_Num vertglbnbr;						  // number of vertices in the total mesh
	SCOTCH_Num edgeglbnbr;						  // number of connections in the total mesh
	SCOTCH_Num procglbnbr;						  // number of processors

	SCOTCH_Num vertlocnbr;							// number of vertices on this processor
	SCOTCH_Num vertgstnbr;							// number of vertices on this processor, including ghost vertices
	SCOTCH_Num edgelocnbr;							// number of connections to other vertices starting from each local vertex
	
	SCOTCH_Num vertlocmax;
	SCOTCH_Num edgelocsiz;
	std::vector<SCOTCH_Num> vertloctab;
	std::vector<SCOTCH_Num> edgeloctab;
	std::vector<SCOTCH_Num> edgegsttab;
	std::vector<SCOTCH_Num> partloctab;
	std::vector<SCOTCH_Num> proccnttab;// number of vertices per processor
	std::vector<SCOTCH_Num> procvrttab;// start_idx of the vertex for each processor + one extra index greater than vertglbnbr
	
	
	void gather_global_data()
	{
		SCOTCH_dgraphSize(&graph, 
											&vertglbnbr, 
											&vertlocnbr,
											&edgeglbnbr,
											&edgelocnbr);
		
		proccnttab.resize(PEInterface::instance().size());
	  procvrttab.resize(PEInterface::instance().size()+1);
		boost::mpi::all_gather(PEInterface::instance(), vertlocnbr, proccnttab);
		Uint cnt=0;
		for (Uint p=0; p<proccnttab.size(); ++p)
		{
			procvrttab[p] = cnt;
			cnt += proccnttab[p];
		}
		procvrttab[PEInterface::instance().size()] = cnt;
		
		
		if (SCOTCH_dgraphGhst(&graph))
			throw BadValue(FromHere(),"ptscotch error");

		
		SCOTCH_dgraphData(&graph,
											NULL,
											NULL, 
											NULL,
											NULL,
										  &vertgstnbr,
											NULL,
											NULL,
											NULL,//&veloloctab,
											NULL,//&vlblocltab,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,//&edloloctab,
											NULL); // &comm
		

	}
	
	void build_graph()
	{
		SCOTCH_dgraphFree(&graph);
		
		if (SCOTCH_dgraphInit(&graph, PEInterface::instance()))
			throw BadValue(FromHere(),"ptscotch error");

		if (SCOTCH_dgraphBuild(&graph, 
													 baseval, 
													 vertlocnbr,      // number of local vertices (for creation of proccnttab)
													 vertlocmax,          // max number of local vertices to be created (for creation of procvrttab)
													 to_ptr(vertloctab),  // local adjacency index array (size = vertlocnbr+1 if vendloctab matches or is null)
													 to_ptr(vertloctab)+1,  //   (optional) local adjacency end index array 
													 NULL, //veloloctab,  //   (optional) local vertex load array
													 NULL,  //vlblocltab,  //   (optional) local vertex label array (size = vertlocnbr+1)
													 edgelocnbr,      // total number of arcs (twice number of edges)
													 edgelocsiz,      // minimum size of the edge array required to encompass all used adjacency values (at least equal to the max of vendloctab entries)
													 to_ptr(edgeloctab),  // edgeloctab,  local adjacency array which stores global indices
													 to_ptr(edgegsttab),  // edgegsttab,  //   (optional) if passed it is assumed an empty array that will be filled by SCOTHC_dgraphGhst if required
													 NULL))   //edloloctab)) //   (optional) arc load array of size edgelocsiz
			throw BadValue(FromHere(),"ptscotch error");
		
		
		gather_global_data();

	}
	
	
	void output_graph_info()
	{
		if (PEInterface::instance().rank() == 0)
		{
			CFinfo << "\n" << CFendl;
			CFinfo << "global graph info" << CFendl;
			CFinfo << "-----------------" << CFendl;
			CFLogVar(vertglbnbr);
			CFLogVar(edgeglbnbr);
			CFinfo << "proccnttab = [ ";
			for (Uint i=0; i<PEInterface::instance().size(); ++i)
				CFinfo << proccnttab[i] << " ";
			CFinfo << "]" << CFendl;
			CFinfo << "procvrttab = [ ";
			for (Uint i=0; i<PEInterface::instance().size()+1; ++i)
				CFinfo << procvrttab[i] << " ";
			CFinfo << "]" << CFendl;
			
			CFinfo << CFendl << CFendl;
		}
		
		bool original_filter = CFinfo.getFilterRankZero(LogStream::SCREEN);
		CFinfo.setFilterRankZero(LogStream::SCREEN,false);
		for (Uint proc=0; proc<PEInterface::instance().size(); ++proc)
		{
			if (PEInterface::instance().rank() == proc)
			{
				CFinfo << "proc #"<<proc << CFendl;
				CFinfo << "-------" << CFendl;
				CFLogVar(vertlocnbr);
				CFLogVar(vertlocmax);
				CFLogVar(vertgstnbr);
				
				CFinfo << "vertloctab = [ ";
				for (int i=0; i<vertlocnbr; ++i)
					CFinfo << vertloctab[i] << " ";
				CFinfo << "]" << CFendl;
				
				CFLogVar(edgelocnbr);
				CFinfo << "edgeloctab = [ ";
				for (int i=0; i<edgelocnbr; ++i)
					CFinfo << edgeloctab[i] << " ";
				CFinfo << "]" << CFendl;
				
				CFinfo << "edgegsttab = [ ";
				for (int i=0; i<edgelocnbr; ++i)
					CFinfo << edgegsttab[i] << " ";
				CFinfo << "]" << CFendl;
				
				
				CFinfo << CFendl;					
			}
			PEInterface::instance().barrier();
		}
		PEInterface::instance().barrier();
		CFinfo.setFilterRankZero(LogStream::SCREEN,original_filter);
	}
	
	void partition_graph(const Uint nb_parts)
	{
		SCOTCH_Strat stradat;
		if(SCOTCH_stratInit(&stradat))
			throw BadValue (FromHere(), "pt-scotch error");
		
		partloctab.resize(vertlocmax);
		
		if (SCOTCH_dgraphPart(&graph,
													nb_parts,
													&stradat,
													to_ptr(partloctab)))
			throw BadValue (FromHere(), "pt-scotch error");
		
		SCOTCH_stratExit(&stradat);
	}
	
	void output_graph_partitions()
	{
		bool original_filter = CFinfo.getFilterRankZero(LogStream::SCREEN);
		CFinfo.setFilterRankZero(LogStream::SCREEN,false);
		for (Uint proc=0; proc<PEInterface::instance().size(); ++proc)
		{
			if (PEInterface::instance().rank() == proc)
			{
				CFinfo << "proc #"<<proc << CFendl;
				CFinfo << "-------" << CFendl;
				CFLogVar(vertlocnbr);
				CFLogVar(vertlocmax);
				CFLogVar(vertgstnbr);
				
				for (int i=0; i<vertlocnbr; ++i)
					CFinfo << procvrttab[proc]+i << " goes to part " << partloctab[i] << CFendl;
				
				CFinfo << CFendl;					
			}
			PEInterface::instance().barrier();
		}
		PEInterface::instance().barrier();
		CFinfo.setFilterRankZero(LogStream::SCREEN,original_filter);
		CFinfo << CFendl<< CFendl;		
	}
	
	int m_argc;
	char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PTScotchTests_TestSuite, PTScotchTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
	PEInterface::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( PTSCOTCH_tutorial_construction )
{
	
	if (PEInterface::instance().size() == 3)
	{
		
		CFinfo << "+++++++++++++++++++ building graph ++++++++++++++++++++ \n" << CFendl;
		
		switch (PEInterface::instance().rank())
		{
			case 0:
			{
				vertloctab = list_of(0)(2)(6)(9) (0);
				vertlocnbr=vertloctab.size()-2;
				vertlocmax=vertlocnbr; // no expected growth
				
				edgeloctab = list_of(2)(1)(2)(4)(3)(0)(3)(1)(0) (0)(0);
				edgelocnbr = edgeloctab.size()-2;
				edgelocsiz = edgeloctab.size();
				edgegsttab.resize(edgeloctab.size());
				break;
			}
			case 1:
			{
				vertloctab = list_of(0)(5)(8) (0);
				vertlocnbr=vertloctab.size()-2;
				vertlocmax=vertlocnbr; // no expected growth

				edgeloctab = list_of(2)(5)(1)(7)(4)(7)(1)(3) (0)(0);
				edgelocnbr = edgeloctab.size()-2;
				edgelocsiz = edgeloctab.size();
				edgegsttab.resize(edgeloctab.size());
				break;
			}
			case 2:
			{
				vertloctab = list_of(0)(3)(5)(9) (0);
				vertlocnbr=vertloctab.size()-2;
				vertlocmax=vertlocnbr; // no expected growth

				edgeloctab = list_of(3)(6)(7)(5)(7)(3)(5)(6)(4) (0)(0);
				edgelocnbr = edgeloctab.size()-2;
				edgelocsiz = edgeloctab.size();
				edgegsttab.resize(edgeloctab.size());
				break;
			}
		}		
		
		build_graph();
		
		BOOST_CHECK_EQUAL(SCOTCH_dgraphCheck(&graph),0);
		
		CFinfo << "\n\n+++++++++++++++++++ graph info ++++++++++++++++++++ " << CFendl;
		
		
		output_graph_info();

		BOOST_CHECK(true);
		
		CFinfo << "\n\n+++++++++++++++++++ partitioning ++++++++++++++++++++ \n" << CFendl;
		
		partition_graph(3);								 
																			 
		output_graph_partitions();
	}
	else
	{
		CFinfo << "must run this testcase with 3 processors" << CFendl;
	}

	
}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( PTSCOTCH_mesh_test )
{
	CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
	//meshreader->configure_property("Number of Parts",world.size());
	//meshreader->configure_property("Part",world.rank());
	// the file to read from
	boost::filesystem::path fp_in ("quadtriag.neu");
	// the mesh to store in
	
	
	CMesh::Ptr mesh = meshreader->create_mesh_from(fp_in);

		
	BOOST_FOREACH(const CArray& coords, recursive_filtered_range_typed<CArray>(*mesh,IsComponentTag("coordinates")))
	{
		
		const CList<Uint>& global_node_idx = get_tagged_component_typed<CList<Uint> >(coords,"global_node_indices");
		const CList<bool>& is_ghost = *coords.get_child_type<CList<bool> >("is_ghost");

		CNodeConnectivity::Ptr node_connectivity = mesh->create_component_type<CNodeConnectivity>("node_connectivity");
		node_connectivity->initialize(recursive_range_typed<CElements>(*coords.get_parent()));

		
		
  	vertloctab.resize(0);
  	edgeloctab.resize(0);
  	Uint start;
  	Uint end = 0;
		vertloctab.push_back(end);
  	for (Uint iNode=0; iNode<coords.size(); ++iNode)
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
  							edgeloctab.push_back(global_node_idx[node]);
  							end++;							
  						}
  					}
  				}			
  			}
				
  			vertloctab.push_back(end);
								
  			CFinfo << "node " << global_node_idx[iNode] << " is connected to nodes " << CFflush;
  			for (Uint j=start; j<end; ++j)
  			{
  				CFinfo << edgeloctab[j] << " " << CFflush;
  			}
  			CFinfo << CFendl;
				
  		}
			
  	}
		
		mesh->remove_component(node_connectivity->name());
		
		
		vertlocnbr=vertloctab.size()-1;
		vertlocmax=vertlocnbr; // no expected growth
		
		edgelocnbr = edgeloctab.size();
		edgelocsiz = edgeloctab.size();
		edgegsttab.resize(edgeloctab.size());
		
		build_graph();
		output_graph_info();
		partition_graph(2);
		output_graph_partitions();

	}
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
	PEInterface::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

