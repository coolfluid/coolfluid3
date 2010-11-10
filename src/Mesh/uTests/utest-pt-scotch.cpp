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
     int    argc = boost::unit_test::framework::master_test_suite().argc;
     char** argv = boost::unit_test::framework::master_test_suite().argv;

		PEInterface::instance().init(argc,argv);

    CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");

    // the file to read from
    boost::filesystem::path fp_in ("quadtriag.neu");
    // the mesh to store in
		

    m_mesh = meshreader->create_mesh_from(fp_in);

  }

  /// common tear-down for each test case
  ~PTScotchTests_Fixture()
  {
		PEInterface::instance().finalize();

  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
	
	boost::shared_ptr<SCOTCH_Dgraph> grafptr;
	
	SCOTCH_Num baseval;								  // first index of an array starts with 0 for c++
	SCOTCH_Num vertglbnbr;						  // number of vertices in the total mesh
	SCOTCH_Num edgeglbnbr;						  // number of connections in the total mesh
	SCOTCH_Num procglbnbr;						  // number of processors
	SCOTCH_Num* proccnttab; // number of vertices per processor
  SCOTCH_Num* procvrttab; // start_idx of the vertex for each processor + one extra index greater than vertglbnbr

	SCOTCH_Num vertlocnbr;							// number of vertices on this processor
	SCOTCH_Num vertgstnbr;							// number of vertices on this processor, including ghost vertices
	SCOTCH_Num edgelocnbr;							// number of connections to other vertices starting from each local vertex
	SCOTCH_Num* vertloctab; // start_idx in edgeloctab and edgegsttab of array of connected vertices
	SCOTCH_Num* vendloctab; // end_idx in edgeloctab and edgegsttab of array of connected vertices
	SCOTCH_Num* edgeloctab; // array of all connections between vertices in global indices
	SCOTCH_Num* edgegsttab; // array of all connections between vertices in local indices

  CMesh::Ptr m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( PTScotchTests_TestSuite, PTScotchTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( PTSCOTCH_tutorial_construction )
{
	if (PEInterface::instance().size() == 3)
	{
		
		CFinfo << "+++++++++++++++++++ building graph ++++++++++++++++++++ \n" << CFendl;
		
		// http://foam.sourceforge.net/doc/Doxygen/html/scotchDecomp_8C_source.html

		grafptr = boost::shared_ptr<SCOTCH_Dgraph>( new SCOTCH_Dgraph );
			
		if (SCOTCH_dgraphInit(grafptr.get(), PEInterface::instance()))
			throw BadValue(FromHere(),"ptscotch error");
		
		
		SCOTCH_Num vertlocmax;
		SCOTCH_Num edgelocsiz;
		
		std::vector<SCOTCH_Num> vertloctab_vec;
		std::vector<SCOTCH_Num> edgeloctab_vec;
		SCOTCH_Num* vendloctab = NULL;
		SCOTCH_Num* edgegsttab = NULL;
		
		baseval=0;
		
		switch (PEInterface::instance().rank())
		{
			case 0:
			{
				vertloctab_vec = list_of(0)(2)(6)(9) (0);
				vertloctab = &vertloctab_vec[0];
				vertlocnbr=vertloctab_vec.size()-2;
				vertlocmax=vertlocnbr; // no expected growth
				vendloctab = vertloctab + 1;
				
				edgeloctab_vec = list_of(2)(1)(2)(4)(3)(0)(3)(1)(0) (0)(0);
				edgeloctab = &edgeloctab_vec[0];
				edgelocnbr = edgeloctab_vec.size()-2;
				edgelocsiz = edgeloctab_vec.size();
				break;
			}
			case 1:
			{
				vertloctab_vec = list_of(0)(5)(8) (0);
				vertloctab = &vertloctab_vec[0];
				vertlocnbr=vertloctab_vec.size()-2;
				vertlocmax=vertlocnbr; // no expected growth
				vendloctab = vertloctab + 1;

				edgeloctab_vec = list_of(2)(5)(1)(7)(4)(7)(1)(3) (0)(0);
				edgeloctab = &edgeloctab_vec[0];
				edgelocnbr = edgeloctab_vec.size()-2;
				edgelocsiz = edgeloctab_vec.size();
				break;
			}
			case 2:
			{
				vertloctab_vec = list_of(0)(3)(5)(9) (0);
				vertloctab = &vertloctab_vec[0];
				vertlocnbr=vertloctab_vec.size()-2;
				vertlocmax=vertlocnbr; // no expected growth
				vendloctab = vertloctab + 1;

				edgeloctab_vec = list_of(3)(6)(7)(5)(7)(3)(5)(6)(4) (0)(0);
				edgeloctab = &edgeloctab_vec[0];
				edgelocnbr = edgeloctab_vec.size()-2;
				edgelocsiz = edgeloctab_vec.size();
				break;
			}
		}		
		
		if (SCOTCH_dgraphBuild(grafptr.get(), 
													 baseval, 
													 vertlocnbr,      // number of local vertices (for creation of proccnttab)
													 vertlocmax,          // max number of local vertices to be created (for creation of procvrttab)
													 vertloctab,  // local adjacency index array (size = vertlocnbr+1 if vendloctab matches or is null)
													 vendloctab,  //   (optional) local adjacency end index array 
													 NULL, //veloloctab,  //   (optional) local vertex load array
													 NULL,  //vlblocltab,  //   (optional) local vertex label array (size = vertlocnbr+1)
													 edgelocnbr,      // total number of arcs (twice number of edges)
													 edgelocsiz,      // minimum size of the edge array required to encompass all used adjacency values (at least equal to the max of vendloctab entries)
													 edgeloctab,  // edgeloctab,  local adjacency array which stores global indices
													 edgegsttab,  // edgegsttab,  //   (optional) if passed it is assumed an empty array that will be filled by SCOTHC_dgraphGhst if required
													 NULL))   //edloloctab)) //   (optional) arc load array of size edgelocsiz
			throw BadValue(FromHere(),"ptscotch error");
		
		CFLogVar(SCOTCH_dgraphCheck(grafptr.get()));

		
		BOOST_CHECK(true);
		
		CFinfo << "\n\n+++++++++++++++++++ graph info ++++++++++++++++++++ " << CFendl;
		
		
		SCOTCH_Num vertglbnbr;
		SCOTCH_Num vertlocnbr;
		SCOTCH_Num edgeglbnbr;
		SCOTCH_Num edgelocnbr;
		
		
		SCOTCH_dgraphSize(grafptr.get(), 
											&vertglbnbr, 
											&vertlocnbr,
											&edgeglbnbr,
											&edgelocnbr);
		
		if (SCOTCH_dgraphGhst(grafptr.get()))
			throw BadValue(FromHere(),"ptscotch error");

		
		SCOTCH_dgraphData(grafptr.get(),
											&baseval,
											&vertglbnbr, 
											&vertlocnbr,
											&vertlocmax,
										  &vertgstnbr,
											&vertloctab,
											&vendloctab,
											NULL,//&veloloctab,
											NULL,//&vlblocltab,
											&edgeglbnbr,
											&edgelocnbr,
											&edgelocsiz,
											&edgeloctab,
											&edgegsttab,
											NULL,//&edloloctab,
											NULL); // &comm
		
		
		if (PEInterface::instance().rank() == 0)
		{
			CFinfo << "\n" << CFendl;
			CFinfo << "global graph info" << CFendl;
			CFinfo << "-----------------" << CFendl;
			CFLogVar(vertglbnbr);
			CFLogVar(edgeglbnbr);
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
				
		
		BOOST_CHECK(true);
		
		CFinfo << "\n\n+++++++++++++++++++ partitioning ++++++++++++++++++++ \n" << CFendl;
		SCOTCH_Strat stradat;
		if(SCOTCH_stratInit(&stradat))
			throw BadValue (FromHere(), "pt-scotch error");
		
		
		SCOTCH_Num* partloctab = new SCOTCH_Num [vertlocmax];
		if (SCOTCH_dgraphPart(grafptr.get(),
													3,
													&stradat,
													partloctab))
			throw BadValue (FromHere(), "pt-scotch error");

		
		
		
		
		std::vector<SCOTCH_Num> proccnttab(PEInterface::instance().size());
	  std::vector<SCOTCH_Num> procvrttab(PEInterface::instance().size()+1);
		boost::mpi::all_gather(PEInterface::instance(), vertlocnbr, proccnttab);
		Uint cnt=0;
		for (Uint p=0; p<proccnttab.size(); ++p)
		{
			procvrttab[p] = cnt;
			cnt += proccnttab[p];
		}
		procvrttab[PEInterface::instance().size()] = cnt;
		
		CFinfo << "procvrttab = [ ";
		for (Uint i=0; i<PEInterface::instance().size()+1; ++i)
			CFinfo << procvrttab[i] << " ";
		CFinfo << "]" << CFendl;
		
																			 
																			 
		original_filter = CFinfo.getFilterRankZero(LogStream::SCREEN);
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
					CFinfo << procvrttab[proc]+i+1 << " goes to part " << partloctab[i] << CFendl;

				CFinfo << CFendl;					
			}
			PEInterface::instance().barrier();
		}
		PEInterface::instance().barrier();
		CFinfo.setFilterRankZero(LogStream::SCREEN,original_filter);
		CFinfo << CFendl<< CFendl;
	}
	else
	{
		CFinfo << "must run this testcase with 3 processors" << CFendl;
	}

	
}

//////////////////////////////////////////////////////////////////////

/*
BOOST_AUTO_TEST_CASE( PTSCOTCH_mesh_test )
{
  CMesh& mesh = *m_mesh;

	
	baseval=0;
	vertglbnbr = mesh.property("nb_nodes").value<Uint>();
	
	procglbnbr = PEInterface::instance().size();
	proccnttab.resize(procglbnbr);
	procvrttab.resize(procglbnbr+1);
		
	BOOST_FOREACH(const CArray& coords, recursive_filtered_range_typed<CArray>(mesh,IsComponentTag("coordinates")))
	{
		
		const CList<Uint>& global_node_idx = get_tagged_component_typed<CList<Uint> >(coords,"global_node_indices");
		const CList<bool>& is_ghost = *coords.get_child_type<CList<bool> >("is_ghost");

		CNodeConnectivity::Ptr node_connectivity = mesh.create_component_type<CNodeConnectivity>("node_connectivity");
		node_connectivity->initialize(recursive_range_typed<CElements>(*coords.get_parent()));

		
		
  	vertloctab.resize(0);
  	vendloctab.resize(0);
  	edgeloctab.resize(0);
  	edgegsttab.resize(0);
  	Uint start;
  	Uint end = 0;
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
		
				
		
		mesh.remove_component(node_connectivity->name());
		
	
		edgeglbnbr = boost::mpi::all_reduce(PEInterface::instance(), edgeloctab.size(), std::plus<Uint>());

		
		SCOTCH_Num nb_nodes = 0;
		BOOST_FOREACH(bool is_ghost_node, is_ghost.array())
		{
			if (!is_ghost_node)
				nb_nodes++;
		}
		
		boost::mpi::all_gather(PEInterface::instance(), nb_nodes, proccnttab);
		
		Uint cnt=0;
		for (Uint p=0; p<proccnttab.size(); ++p)
		{
			procvrttab[p] = cnt;
			cnt += proccnttab[p];
		}
		procvrttab[procglbnbr] = cnt;
		
		
		
		vertlocnbr = vertloctab.size();
		vertgstnbr = coords.size();
		edgelocnbr = edgeloctab.size();
		
		
		
		if (PEInterface::instance().rank() == 0)
		{
			CFinfo << "\n\n\n" << CFendl;
			CFinfo << "global graph info" << CFendl;
			CFinfo << "-----------------" << CFendl;
			CFLogVar(baseval);
			CFLogVar(vertglbnbr);
			CFLogVar(edgeglbnbr);
			CFLogVar(procglbnbr);
			print_vector(CFinfo, proccnttab, " ", "proccnttab = ","\n");
			print_vector(CFinfo, procvrttab, " ", "procvrttab = ","\n");
			CFinfo << CFendl << CFendl;
		}

		bool original_filter = CFinfo.getFilterRankZero(LogStream::SCREEN);
		CFinfo.setFilterRankZero(LogStream::SCREEN,false);
		for (Uint proc=0; proc<procglbnbr; ++proc)
		{
			if (PEInterface::instance().rank() == proc)
			{
				CFinfo << "proc #"<<proc << CFendl;
				CFinfo << "-------" << CFendl;
				CFLogVar(vertlocnbr);
				CFLogVar(vertgstnbr);
				CFLogVar(edgelocnbr);
				print_vector(CFinfo, vertloctab, " ", "vertloctab = ","\n");
				print_vector(CFinfo, edgeloctab, " ", "edgeloctab = ","\n");
				print_vector(CFinfo, edgegsttab, " ", "edgegsttab = ","\n");
				print_vector(CFinfo, vendloctab, " ", "edgegsttab = ","\n");
				CFinfo << CFendl;					
			}
			PEInterface::instance().barrier();
		}
		PEInterface::instance().barrier();
		CFinfo.setFilterRankZero(LogStream::SCREEN,original_filter);
		CFinfo << CFendl<< CFendl<< CFendl<< CFendl;					

		boost::shared_ptr<SCOTCH_Dgraph> grafptr ( new SCOTCH_Dgraph );

		if (SCOTCH_dgraphInit(grafptr.get(), PEInterface::instance()))
			throw BadValue(FromHere(),"ptscotch error");
		
		
		SCOTCH_Num vertlocmax = vertlocnbr;
		std::vector<SCOTCH_Num> veloloctab(1);
		std::vector<SCOTCH_Num> vlblocltab(1);
		std::vector<SCOTCH_Num> edloloctab(1);
		SCOTCH_Num edgelocsiz = edgeloctab.size();
		
		if (
		SCOTCH_dgraphBuild(grafptr.get(), 
											 baseval, 
											 vertlocnbr,      // number of local vertices (for creation of proccnttab)
											 vertlocmax,      // max number of local vertices to be created (for creation of procvrttab)
											 &vertloctab[0],  // local adjacency index array (size = vertlocnbr+1 if vendloctab matches or is null)
											 &vendloctab[0],  //   (optional) local adjacency end index array 
											 &veloloctab[0],  //   (optional) local vertex load array
											 &vlblocltab[0],  //   (optional) local vertex label array (size = vertlocnbr+1)
											 edgelocnbr,      // total number of arcs (twice number of edges)
											 edgelocsiz,      // minimum size of the edge array required to encompass all used adjacency values (at least equal to the max of vendloctab entries)
											 &edgeloctab[0],  // local adjacency array which stores global indices
											 &edgegsttab[0],  //   (optional) if passed it is assumed an empty array that will be filled by SCOTHC_dgraphGhst if required
											 &edloloctab[0])) //   (optional) arc load array of size edgelocsiz
			throw BadValue(FromHere(),"ptscotch error");
	}
		
}
*/
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

