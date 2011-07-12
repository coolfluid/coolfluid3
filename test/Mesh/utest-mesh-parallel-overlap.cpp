// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for parallel fields"

#include <iomanip>
#include <set>

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"

#include "Common/Foreach.hpp"
#include "Common/OSystem.hpp"
#include "Common/OSystemLayer.hpp"

#include "Common/MPI/PECommPattern.hpp"
#include "Common/MPI/PEObjectWrapperMultiArray.hpp"
#include "Common/MPI/Buffer.hpp"
#include "Common/MPI/debug.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshElements.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshGenerator.hpp"
#include "Mesh/CMeshPartitioner.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Manipulations.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Common::mpi;
using namespace CF::Math::MathConsts;

template <typename T>
std::ostream& operator<< (std::ostream& out , const std::vector<T>& v)
{
  for (Uint i=0; i<v.size()-1; ++i)
    out << v[i] << " ";
  if (v.size())
    out << v.back();
  return out;
}

template <typename T>
void my_all_gather(const std::vector<T>& send, std::vector<std::vector<T> >& recv)
{
  std::vector<int> strides;
  PE::instance().all_gather((int)send.size(),strides);
  std::vector<int> displs(strides.size());
  if (strides.size())
  {
    int sum_strides = strides[0];
    displs[0] = 0;
    for (Uint i=1; i<strides.size(); ++i)
    {
      displs[i] = displs[i-1] + strides[i-1];
      sum_strides += strides[i];
    }
    std::vector<Uint> recv_linear(sum_strides);
    MPI_CHECK_RESULT(MPI_Allgatherv, ((void*)&send[0], (int)send.size(), get_mpi_datatype<T>(), &recv_linear[0], &strides[0], &displs[0], get_mpi_datatype<T>(), PE::instance().communicator()));
    recv.resize(strides.size());
    for (Uint i=0; i<strides.size(); ++i)
    {
      recv[i].resize(strides[i]);
      for (Uint j=0; j<strides[i]; ++j)
      {
        recv[i][j]=recv_linear[displs[i]+j];
      }
    }
  }
  else
  {
    recv.resize(0);
  }
}

template <typename T>
void my_all_to_all(const std::vector<std::vector<T> >& send, std::vector<std::vector<T> >& recv)
{
  std::vector<int> send_strides(send.size());
  std::vector<int> send_displs(send.size());
  for (Uint i=0; i<send.size(); ++i)
    send_strides[i] = send[i].size();

  send_displs[0] = 0;
  for (Uint i=1; i<send.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  std::vector<T> send_linear(send_displs.back()+send_strides.back());
  for (Uint i=0; i<send.size(); ++i)
    for (Uint j=0; j<send[i].size(); ++j)
      send_linear[send_displs[i]+j] = send[i][j];

  std::vector<int> recv_strides(PE::instance().size());
  std::vector<int> recv_displs(PE::instance().size());
  PE::instance().all_to_all(send_strides,recv_strides);
  recv_displs[0] = 0;
  for (Uint i=1; i<PE::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];

  std::vector<Uint> recv_linear(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, (&send_linear[0], &send_strides[0], &send_displs[0], get_mpi_datatype<Uint>(), &recv_linear[0], &recv_strides[0], &recv_displs[0], get_mpi_datatype<Uint>(), PE::instance().communicator()));

  recv.resize(recv_strides.size());
  for (Uint i=0; i<recv_strides.size(); ++i)
  {
    recv[i].resize(recv_strides[i]);
    for (Uint j=0; j<recv_strides[i]; ++j)
    {
      recv[i][j]=recv_linear[recv_displs[i]+j];
    }
  }
}

void my_all_to_all(const std::vector<mpi::Buffer>& send, mpi::Buffer& recv)
{
  std::vector<int> send_strides(send.size());
  std::vector<int> send_displs(send.size());
  for (Uint i=0; i<send.size(); ++i)
    send_strides[i] = send[i].packed_size();

  if (send.size()) send_displs[0] = 0;
  for (Uint i=1; i<send.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  mpi::Buffer send_linear;

  send_linear.resize(send_displs.back()+send_strides.back());
  for (Uint i=0; i<send.size(); ++i)
    send_linear.pack(send[i].buffer(),send[i].packed_size());

  std::vector<int> recv_strides(PE::instance().size());
  std::vector<int> recv_displs(PE::instance().size());
  PE::instance().all_to_all(send_strides,recv_strides);
  if (recv_displs.size()) recv_displs[0] = 0;
  for (Uint i=1; i<PE::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];
  recv.reset();
  recv.resize(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send_linear.buffer(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.buffer(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::instance().communicator()));
  recv.packed_size()=recv_displs.back()+recv_strides.back();
}

////////////////////////////////////////////////////////////////////////////////

void my_all_to_all(const mpi::Buffer& send, std::vector<int>& send_strides, mpi::Buffer& recv, std::vector<int>& recv_strides)
{
//  std::vector<int> send_strides(send_displs.size());
//  for (Uint i=0; i<send_strides.size()-1; ++i)
//    send_strides[i] = send_displs[i+1] - send_displs[i];
//  if (send_strides.size())
//    send_strides.back() = send.packed_size() - send_displs.back();

  std::vector<int> send_displs(send_strides.size());
  if (send_strides.size()) send_displs[0] = 0;
  for (Uint i=1; i<send_strides.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  recv_strides.resize(PE::instance().size());
  std::vector<int> recv_displs(PE::instance().size());
  PE::instance().all_to_all(send_strides,recv_strides);
  //std::cout << PERank << "recv_strides = " << recv_strides << std::endl;
  if (recv_displs.size()) recv_displs[0] = 0;
  for (Uint i=1; i<PE::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];
  recv.reset();
  recv.resize(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send.buffer(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.buffer(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::instance()));
  recv.packed_size()=recv_displs.back()+recv_strides.back();
}

////////////////////////////////////////////////////////////////////////////////

struct ParallelOverlapTests_Fixture
{
  /// common setup for each test case
  ParallelOverlapTests_Fixture()
  {
    // uncomment if you want to use arguments to the test executable
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

  }

  /// common tear-down for each test case
  ~ParallelOverlapTests_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ParallelOverlapTests_TestSuite, ParallelOverlapTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Core::instance().initiate(m_argc,m_argv);
  mpi::PE::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( test_buffer_MPINode )
{
  CFinfo << "ParallelOverlap_test" << CFendl;
  Core::instance().environment().configure_option("log_level",(Uint)INFO);

  // Create or read the mesh
  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");
  meshgenerator->configure_option("parent",URI("//Root"));
  meshgenerator->configure_option("name",std::string("test_mpinode_mesh"));
  std::vector<Uint> nb_cells(2);
  std::vector<Real> lengths(2);
  nb_cells[0] = 3;
  nb_cells[1] = 2;
  lengths[0]  = nb_cells[0];
  lengths[1]  = nb_cells[1];
  meshgenerator->configure_option("nb_cells",nb_cells);
  meshgenerator->configure_option("lengths",lengths);
  meshgenerator->configure_option("bdry",false);
  meshgenerator->execute();
  CMesh& mesh = Core::instance().root().get_child("test_mpinode_mesh").as_type<CMesh>();

  Core::instance().root().add_component(mesh);

  //build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balancer")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_node_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_elem_node_connectivity")->transform(mesh);

  BOOST_CHECK(true);
  CNodes& nodes = mesh.nodes();

  PackUnpackNodes copy_node(nodes);
  mpi::Buffer buf;
  buf << copy_node(0);
  buf << copy_node(1);
  copy_node.flush();
  buf >> copy_node;
  copy_node.flush();


  BOOST_CHECK_EQUAL(nodes.glb_idx()[nodes.size()-1] , nodes.glb_idx()[0]);
  BOOST_CHECK_EQUAL(nodes.coordinates()[nodes.size()-1][0] , nodes.coordinates()[0][0]);
  BOOST_CHECK_EQUAL(nodes.coordinates()[nodes.size()-1][1] , nodes.coordinates()[0][1]);


}
*/
BOOST_AUTO_TEST_CASE( parallelize_and_synchronize )
{
  CFinfo << "ParallelOverlap_test" << CFendl;
  Core::instance().environment().configure_option("log_level",(Uint)DEBUG);

  // Create or read the mesh

#define NEU

#ifdef GEN
  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");
  meshgenerator->configure_option("parent",URI("//Root"));
  meshgenerator->configure_option("name",std::string("rect"));
  std::vector<Uint> nb_cells(2);
  std::vector<Real> lengths(2);
  nb_cells[0] = 200;
  nb_cells[1] = 200;
  lengths[0]  = nb_cells[0];
  lengths[1]  = nb_cells[1];
  meshgenerator->configure_option("nb_cells",nb_cells);
  meshgenerator->configure_option("lengths",lengths);
  meshgenerator->configure_option("bdry",false);
  meshgenerator->execute();
  CMesh& mesh = Core::instance().root().get_child("rect").as_type<CMesh>();
#endif

#ifdef NEU
  CMeshReader::Ptr meshreader =
      build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  meshreader->configure_option("read_boundaries",false);
  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("rotation-tg-p1.neu");
//  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("quadtriag.neu");
  CMesh& mesh = *mesh_ptr;
#endif

#ifdef GMSH
  CMeshReader::Ptr meshreader =
      build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");
  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("rectangle-tg-p1.msh");
  CMesh& mesh = *mesh_ptr;
#endif


  Core::instance().root().add_component(mesh);
  CNodes& nodes = mesh.nodes();


  CMeshWriter::Ptr msh_writer =
      build_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","msh_writer");

  msh_writer->write_from_to(mesh,"parallel_overlap_before"+msh_writer->get_extensions()[0]);

  CFinfo << "parallel_overlap_before_P*"+msh_writer->get_extensions()[0]+" written" << CFendl;

  CFinfo << "Global Numbering..." << CFendl;

//  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balancer")->transform(mesh);
  CMeshTransformer::Ptr glb_numbering = build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering");
  glb_numbering->configure_option("debug",true);
  glb_numbering->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_node_elem_connectivity")->transform(mesh);

  CFinfo << "Global Numbering... done" << CFendl;

  CFinfo << "Partitioning..." << CFendl;
  CMeshPartitioner::Ptr partitioner_ptr = build_component_abstract_type<CMeshPartitioner>("CF.Mesh.Zoltan.CPartitioner","partitioner");

  CMeshPartitioner& p = *partitioner_ptr;
  p.configure_option("graph_package", std::string("PHG"));
  p.initialize(mesh);
  p.partition_graph();
  //p.show_changes();

  BOOST_CHECK(true);

  CFinfo << "Partitioning... done" << CFendl;

  const Uint my_rank = PE::instance().rank();

  CFinfo << "Migration..." << CFendl;

  mpi::PE::instance().barrier();
  boost::this_thread::sleep(boost::posix_time::milliseconds(50));

#define NEW_MIGRATION

#ifndef NEW_MIGRATION
  p.migrate();
#else
  // ----------------------------------------------------------------------------
  // ----------------------------------------------------------------------------
  //                            MIGRATION ALGORITHM
  // ----------------------------------------------------------------------------
  // ----------------------------------------------------------------------------

  PackUnpackNodes node_manipulation(nodes);

  // -----------------------------------------------------------------------------
  // REMOVE GHOST NODES AND GHOST ELEMENTS


  for (Uint n=0; n<nodes.size(); ++n)
  {
    if (nodes.is_ghost(n))
      node_manipulation.remove(n);
  }

  // DONT FLUSH YET!!! node_manipulation.flush()

  PECheckArrivePoint(10,"ghost nodes removed");

  boost_foreach(CElements& elements, find_components_recursively<CElements>(mesh.topology()) )
  {
    PackUnpackElements element_manipulation(elements);

    if (elements.rank().size() != elements.size())
      throw ValueNotFound(FromHere(),elements.uri().string()+" --> mismatch in element sizes (rank.size() = "+to_str(elements.rank().size())+" , elements.size() = "+to_str(elements.size())+")");
    for (Uint e=0; e<elements.size(); ++e)
    {
      if (elements.rank()[e] != my_rank)
        element_manipulation.remove(e);
    }
    /// @todo mechanism not to flush element_manipulation until during real migration
  }

  PECheckArrivePoint(10,"ghost elements removed");

  // -----------------------------------------------------------------------------
  // SET NODE CONNECTIVITY TO GLOBAL NUMBERS BEFORE PARTITIONING

  const CList<Uint>& global_node_indices = mesh.nodes().glb_idx();
  boost_foreach (CEntities& elements, mesh.topology().elements_range())
  {
    boost_foreach ( CTable<Uint>::Row nodes, elements.as_type<CElements>().node_connectivity().array() )
    {
      boost_foreach ( Uint& node, nodes )
      {
        node = global_node_indices[node];
      }
    }
  }
  PECheckArrivePoint(10,"glb node connectivity renumbered");


  // -----------------------------------------------------------------------------
  // SEND ELEMENTS AND NODES FROM PARTITIONING ALGORITHM

  std::vector<Component::Ptr> mesh_element_comps = mesh.elements().components();

  mpi::Buffer send_to_proc;  std::vector<int> send_strides(PE::instance().size());
  mpi::Buffer recv_from_all; std::vector<int> recv_strides(PE::instance().size());

  // Move elements
  for(Uint i=0; i<mesh_element_comps.size(); ++i)
  {
    CElements& elements = mesh_element_comps[i]->as_type<CElements>();

    send_to_proc.reset();
    recv_from_all.reset();

    PackUnpackElements migrate_element(elements);
    std::vector<Uint> nb_elems_to_send(PE::instance().size());

    for (Uint r=0; r<PE::instance().size(); ++r)
    {
      Uint displs = send_to_proc.packed_size();
      for (Uint e=0; e<p.exported_elements()[i][r].size(); ++e)
        send_to_proc << migrate_element(p.exported_elements()[i][r][e],PackUnpackElements::MIGRATE);
      send_strides[r] = send_to_proc.packed_size() - displs;
    }

    std::cout << PERank << "elements " << elements.uri().path() << " send_strides = " << send_strides << std::endl;

    my_all_to_all(send_to_proc,send_strides,recv_from_all,recv_strides);

    std::cout << PERank << "elements " << elements.uri().path() << " recv_strides = " << recv_strides << std::endl;

    while(recv_from_all.more_to_unpack())
      recv_from_all >> migrate_element;
    migrate_element.flush();
  }

  PECheckArrivePoint(10,"elements moved");


  // Move nodes
   send_to_proc.reset();
   recv_from_all.reset();

   std::set<Uint> packed_nodes;
   for (Uint r=0; r<PE::instance().size(); ++r)
   {
     Uint displs = send_to_proc.packed_size();
     for (Uint n=0; n<p.exported_nodes()[r].size(); ++n)
     {
       send_to_proc << node_manipulation(p.exported_nodes()[r][n],PackUnpackNodes::MIGRATE);
     }
     send_strides[r] = send_to_proc.packed_size() - displs;
   }

  // STILL DONT FLUSH!!! node_manipulation.flush();

   PECheckArrivePoint(10,"nodes packed and removed");

   std::cout << PERank << "nodes send_strides = " << send_strides << std::endl;
   my_all_to_all(send_to_proc,send_strides,recv_from_all,recv_strides);
   std::cout << PERank << "nodes recv_strides = " << recv_strides << std::endl;

   PECheckArrivePoint(10,"nodes communicated");

   while (recv_from_all.more_to_unpack())
    recv_from_all >> node_manipulation;

   // FINALLY FLUSH NODES
   node_manipulation.flush();

   PECheckArrivePoint(10,"nodes moved");

   // -----------------------------------------------------------------------------
   // MARK EVERYTHING AS OWNED

   for (Uint n=0; n<nodes.size(); ++n)
     nodes.rank()[n] = my_rank;

   boost_foreach(CEntities& elements, mesh.topology().elements_range())
   {
     for (Uint e=0; e<elements.size(); ++e)
       elements.rank()[e] = my_rank;
   }

   PECheckArrivePoint(10,"everything marked as owned");


  // -----------------------------------------------------------------------------
  // ELEMENTS AND NODES HAVE BEEN MOVED
  // -----------------------------------------------------------------------------

  boost::this_thread::sleep(boost::posix_time::milliseconds(20));

  // -----------------------------------------------------------------------------
  // COLLECT GHOST-NODES TO LOOK FOR ON OTHER PROCESSORS

  std::set<Uint> owned_nodes;
  for (Uint n=0; n<nodes.size(); ++n)
    owned_nodes.insert(nodes.glb_idx()[n]);

  std::set<Uint> ghost_nodes;
  boost_foreach(const CElements& elements, find_components_recursively<CElements>(mesh.topology()))
  {
    boost_foreach(CConnectivity::ConstRow connected_nodes, elements.node_connectivity().array())
    {
      boost_foreach(const Uint node, connected_nodes)
      {
        if (owned_nodes.find(node) == owned_nodes.end())
          ghost_nodes.insert(node);
      }
    }
  }

  std::vector<Uint> request_nodes;  request_nodes.reserve(ghost_nodes.size());
  boost_foreach(const Uint node, ghost_nodes)
    request_nodes.push_back(node);

//  PEProcessSortedExecute(-1,
//  std::cout << PERank << "look for = " << request_nodes << std::endl;
//  boost::this_thread::sleep(boost::posix_time::milliseconds(10));
//  )
  BOOST_CHECK(true);

  // -----------------------------------------------------------------------------
  // COMMUNICATE NODES TO LOOK FOR

  std::vector<std::vector<Uint> > recv_request_nodes;
  my_all_gather(request_nodes,recv_request_nodes);

  BOOST_CHECK(true);

//  if (PE::instance().rank() == 0)
//  {
//    std::cout << "[*] everybody is looking for = ";
//    for (Uint i=0; i<recv_request_nodes.size(); ++i)
//      std::cout << recv_request_nodes[i] << "     ";
//    std::cout << std::endl;
//  }
  BOOST_CHECK(true);

  // -----------------------------------------------------------------------------
  // SEARCH FOR REQUESTED NODES

  PackUnpackNodes& copy_node = node_manipulation;
  std::vector<std::vector<Uint> > found_nodes(PE::instance().size());
  std::vector<mpi::Buffer> nodes_to_send(PE::instance().size());
  std::vector<Uint> nb_nodes_to_send(PE::instance().size(),0);
  for (Uint proc=0; proc<PE::instance().size(); ++proc)
  {
    if (proc != PE::instance().rank())
    {

      for (Uint n=0; n<recv_request_nodes[proc].size(); ++n)
      {
        Uint find_glb_idx = recv_request_nodes[proc][n];

        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
        /// @todo THIS ALGORITHM HAS TO BE IMPROVED (BRUTE FORCE)
        Uint loc_idx=0;
        bool found=false;
        boost_foreach(const Uint glb_idx, nodes.glb_idx().array())
        {

          cf_assert(loc_idx < nodes.size());
          if (glb_idx == find_glb_idx)
          {
            //          found_nodes[proc].push_back(loc_idx);
            found_nodes[proc].push_back(glb_idx);
            //std::cout << PERank << "copying node " << glb_idx << " from loc " << loc_idx << std::flush;
            nodes_to_send[proc] << copy_node(loc_idx,PackUnpackNodes::COPY);

            ++nb_nodes_to_send[proc];
            break;
          }
          ++loc_idx;
        }
        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      }
    }
  }
//  PEProcessSortedExecute(-1,
//  std::cout << PERank << "found_nodes = ";
//  for (Uint i=0; i<found_nodes.size(); ++i)
//    std::cout << found_nodes[i] << "     ";
//  std::cout << std::endl;
//  BOOST_CHECK(true);
//  )

  PECheckArrivePoint(10,"ready to send ghost nodes");

  // -----------------------------------------------------------------------------
  // COMMUNICATE FOUND NODES BACK TO RANK THAT REQUESTED IT

  std::vector<std::vector<Uint> > received_nodes;
  my_all_to_all(found_nodes,received_nodes);

//  std::cout << PERank << "received_nodes = ";
//  for (Uint i=0; i<received_nodes.size(); ++i)
//    std::cout << received_nodes[i] << "     ";
//  std::cout << std::endl;

  BOOST_CHECK(true);

  std::cout << PERank << "nb_nodes_to_send = " << nb_nodes_to_send << std::endl;
  std::vector<Uint> nb_nodes_to_recv(PE::instance().size(),0);
  PE::instance().all_to_all(nb_nodes_to_send,nb_nodes_to_recv);
  std::cout << PERank << "nb_nodes_to_recv = " << nb_nodes_to_recv << std::endl;

  mpi::Buffer received_nodes_buffer;
  my_all_to_all(nodes_to_send,received_nodes_buffer);

  PackUnpackNodes add_node(nodes);
  for (Uint p=0; p<PE::instance().size(); ++p)
  {
    std::cout << PERank << "unpacking " << nb_nodes_to_recv[p] << " nodes from proc " << p << std::endl;
    for (Uint n=0; n<nb_nodes_to_recv[p]; ++n)
    {
      received_nodes_buffer >> copy_node;
    }
  }
  copy_node.flush();

  PECheckArrivePoint(10,"ghost nodes added");

  // -----------------------------------------------------------------------------
  // REQUESTED GHOST-NODES HAVE NOW BEEN ADDED
  // -----------------------------------------------------------------------------

  // -----------------------------------------------------------------------------
  // FIX NODE CONNECTIVITY
  std::map<Uint,Uint> glb_to_loc;
  std::map<Uint,Uint>::iterator it;
  bool inserted;
  for (Uint n=0; n<nodes.size(); ++n)
  {
    boost::tie(it,inserted) = glb_to_loc.insert(std::make_pair(nodes.glb_idx()[n],n));
    if (! inserted)
      throw ValueExists(FromHere(), std::string(nodes.is_ghost(n)? "ghost " : "" ) + "node["+to_str(n)+"] with glb_idx "+to_str(nodes.glb_idx()[n])+" already exists as "+to_str(glb_to_loc[n]));
  }
  boost_foreach (CEntities& elements, mesh.topology().elements_range())
  {
    boost_foreach ( CTable<Uint>::Row nodes, elements.as_type<CElements>().node_connectivity().array() )
    {
      boost_foreach ( Uint& node, nodes )
      {
        node = glb_to_loc[node];
      }
    }
  }

  PECheckArrivePoint(10,"renumbered node connectivity back to normal");

  // -----------------------------------------------------------------------------
  // RENUMBER NODES AND ELEMENTS SEPARATELY

  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_elem_numbering")->transform(mesh);

  // -----------------------------------------------------------------------------
  // MESH IS NOW COMPLETELY LOAD BALANCED WITHOUT OVERLAP
  // -----------------------------------------------------------------------------

#endif


  // Create a field with glb element numbers
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CreateSpaceP0","create_space_P0")->transform(mesh);
  CField& elem_rank = mesh.create_field("elem_rank",CField::Basis::ELEMENT_BASED,"P0");
  CFieldView& field_view = elem_rank.create_component<CFieldView>("field_view");
  field_view.set_field(elem_rank);
  boost_foreach(const CEntities& elements, elem_rank.field_elements())
  {
    field_view.set_elements(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      field_view[e][0] = PE::instance().rank();
    }
  }

    msh_writer->set_fields(std::vector<CField::Ptr>(1,elem_rank.as_ptr<CField>()));
    msh_writer->write_from_to(mesh,"parallel_overlap"+msh_writer->get_extensions()[0]);

    CFinfo << "parallel_overlap_P*"+msh_writer->get_extensions()[0]+" written" << CFendl;

}

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  mpi::PE::instance().finalize();
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

