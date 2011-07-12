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

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Common::mpi;

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
  recv.packed_size()=recv.size();
}

////////////////////////////////////////////////////////////////////////////////

struct PackedNode : public mpi::PackedObject
{
  PackedNode(CNodes& nodes, const Uint idx) :
    mpi::PackedObject(),
    glb_idx(nodes.glb_idx()[idx]),
    rank(nodes.rank()[idx]),
    coords(nodes.coordinates()[idx]),
    connected_elems(nodes.glb_elem_connectivity()[idx])
  {
  }

  virtual void pack(mpi::Buffer& buffer) const
  {
    buffer << glb_idx << rank << coords << connected_elems;
  }

  virtual void unpack(mpi::Buffer& buffer)
  {
    buffer >> glb_idx >> rank >> coords >> connected_elems;
  }

  Uint& glb_idx;
  Uint& rank;
  CTable<Real>::Row coords;
  CDynTable<Uint>::Row connected_elems;
};

struct UnpackAndAddNodes : PackedObject
{
  UnpackAndAddNodes(CNodes& nodes) : PackedObject(),
    is_ghost (nodes.is_ghost().create_buffer()),
    glb_idx (nodes.glb_idx().create_buffer()),
    rank (nodes.rank().create_buffer()),
    coordinates (nodes.coordinates().create_buffer()),
    connected_elements (nodes.glb_elem_connectivity().create_buffer())
  {}

  virtual void pack(Buffer& buf) const {}
  virtual void unpack(Buffer& buf)
  {
    Uint glb_idx_data;
    Uint rank_data;
    std::vector<Real> coordinates_data;
    std::vector<Uint> connected_elems_data;

    buf >> glb_idx_data >> rank_data >> coordinates_data >> connected_elems_data;

    Uint idx, idx_check;
    idx = glb_idx.add_row(glb_idx_data);
    idx_check = rank.add_row(rank_data);
    cf_assert(idx_check == idx);
    idx_check = coordinates.add_row(coordinates_data);
    cf_assert(idx_check == idx);
    idx_check = connected_elements.add_row(connected_elems_data);
    cf_assert(idx_check == idx);
    idx_check = is_ghost.add_row(rank_data != PE::instance().rank());
    cf_assert(idx_check == idx);

    std::cout << PERank << "added node    glb_idx = " << glb_idx_data << "\t    rank = " << rank_data << "\t    coords = " << coordinates_data << "\t    connected_elem = " << connected_elems_data << std::endl;
  }

  void flush()
  {
    is_ghost.flush();
    glb_idx.flush();
    rank.flush();
    coordinates.flush();
    connected_elements.flush();
  }

  CList<bool>::Buffer       is_ghost;
  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Real>::Buffer      coordinates;
  CDynTable<Uint>::Buffer   connected_elements;
};


struct PackedElement : public mpi::PackedObject
{
  PackedElement(CElements& elem_comp, const Uint idx) :
    mpi::PackedObject(),
    elements(elem_comp),
    glb_idx(elements.glb_idx()[idx]),
    rank(elements.rank()[idx]),
    connected_nodes(elements.node_connectivity()[idx])
  {
  }

  virtual void pack(mpi::Buffer& buffer) const
  {
    std::cout << PERank << "packed element " << glb_idx << " with glb nodes  ";

    buffer << glb_idx << rank;
    boost_foreach(const Uint n, connected_nodes)
    {
      buffer << n;
      std::cout << n << "  ";
    }
    std::cout << std::endl;
  }

  virtual void unpack(mpi::Buffer& buffer)
  {
//    buffer >> glb_idx  >> nodes;
  }

  CElements& elements;
  Uint& glb_idx;
  Uint& rank;
  CTable<Uint>::Row connected_nodes;
};

struct UnpackAndAddElements : PackedObject
{
  UnpackAndAddElements(CElements& elements) : PackedObject(),
    glb_idx (elements.glb_idx().create_buffer()),
    rank (elements.rank().create_buffer()),
    connected_nodes (elements.node_connectivity().create_buffer()),
    row_size(elements.node_connectivity().row_size())
  {}

  virtual void pack(Buffer& buf) const {}
  virtual void unpack(Buffer& buf)
  {
    Uint glb_idx_data;
    Uint rank_data;
    std::vector<Uint> connected_nodes_data(row_size);

    buf >> glb_idx_data >> rank_data;

    for (Uint n=0; n<connected_nodes_data.size(); ++n)
      buf >> connected_nodes_data[n];

    Uint idx, idx_check;
    idx = glb_idx.add_row(glb_idx_data);
    idx_check = rank.add_row(rank_data);
    cf_assert(idx_check == idx);
    idx_check = connected_nodes.add_row(connected_nodes_data);
    cf_assert(idx_check == idx);

    std::cout << PERank << "added elem    glb_idx = " << glb_idx_data << "\t    rank = " << rank_data << "\t    connected_nodes = " << connected_nodes_data << std::endl;
  }

  void flush()
  {
    glb_idx.flush();
    connected_nodes.flush();
    rank.flush();
  }

  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Uint>::Buffer      connected_nodes;
  Uint row_size;
};

struct RemoveNodes
{
  RemoveNodes(CNodes& nodes) :
    is_ghost (nodes.is_ghost().create_buffer()),
    glb_idx (nodes.glb_idx().create_buffer()),
    rank (nodes.rank().create_buffer()),
    coordinates (nodes.coordinates().create_buffer()),
    connected_elements (nodes.glb_elem_connectivity().create_buffer())
  {}

  void operator() (const Uint idx)
  {
    Uint val = glb_idx.get_row(idx);

    is_ghost.rm_row(idx);
    glb_idx.rm_row(idx);
    rank.rm_row(idx);
    coordinates.rm_row(idx);
    connected_elements.rm_row(idx);

    std::cout << PERank << "removed node  " << val << std::endl;
  }

  void flush()
  {
    is_ghost.flush();
    glb_idx.flush();
    rank.flush();
    coordinates.flush();
    connected_elements.flush();
  }

  CList<bool>::Buffer       is_ghost;
  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Real>::Buffer      coordinates;
  CDynTable<Uint>::Buffer   connected_elements;
};

struct RemoveElements
{
  RemoveElements(CElements& elements) :
    glb_idx (elements.glb_idx().create_buffer()),
    rank (elements.rank().create_buffer()),
    connected_nodes (elements.node_connectivity().create_buffer())
  {}

  void operator() (const Uint idx)
  {
    Uint val = glb_idx.get_row(idx);

    glb_idx.rm_row(idx);
    rank.rm_row(idx);
    connected_nodes.rm_row(idx);

    std::cout << PERank << "removed element  " << val << std::endl;

  }

  void flush()
  {
    glb_idx.flush();
    connected_nodes.flush();
    rank.flush();
  }

  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Uint>::Buffer      connected_nodes;
};

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

  BOOST_CHECK(true);
  CNodes& nodes = mesh.nodes();


  mpi::Buffer buf;
  buf << PackedNode(nodes,0);
  buf << PackedNode(nodes,1);
  buf << PackedNode(nodes,2);

  PackedNode node3(nodes,3);
  buf >> node3;

  BOOST_CHECK_EQUAL(nodes.glb_idx()[3] , nodes.glb_idx()[0]);
  BOOST_CHECK_EQUAL(nodes.coordinates()[3][0] , nodes.coordinates()[0][0]);
  BOOST_CHECK_EQUAL(nodes.coordinates()[3][1] , nodes.coordinates()[0][1]);


}

BOOST_AUTO_TEST_CASE( parallelize_and_synchronize )
{
  CFinfo << "ParallelOverlap_test" << CFendl;
  Core::instance().environment().configure_option("log_level",(Uint)DEBUG);

  // Create or read the mesh

#define GEN

#ifdef GEN
  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");
  meshgenerator->configure_option("parent",URI("//Root"));
  meshgenerator->configure_option("name",std::string("rect"));
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
  CMesh& mesh = Core::instance().root().get_child("rect").as_type<CMesh>();
#endif

#ifdef NEU
  CMeshReader::Ptr meshreader =
      build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("rotation-tg-p1.neu");
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

//  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balancer")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_node_elem_connectivity")->transform(mesh);


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

  // -----------------------------------------------------------------------------
  // REMOVE GHOST NODES AND GHOST ELEMENTS

  const Uint my_rank = PE::instance().rank();
  RemoveNodes remove_node(nodes);
  for (Uint n=0; n<nodes.size(); ++n)
  {
    if (nodes.rank()[n] != my_rank)
      remove_node(n);
  }
  remove_node.flush();
  boost_foreach(CElements& elements, find_components_recursively<CElements>(mesh.topology()) )
  {
    RemoveElements remove_element(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      if (elements.rank()[e] != my_rank)
        remove_element(e);
    }
    remove_element.flush();
  }


  CMeshPartitioner::Ptr partitioner_ptr = build_component_abstract_type<CMeshPartitioner>("CF.Mesh.Zoltan.CPartitioner","partitioner");

  CMeshPartitioner& p = *partitioner_ptr;
  p.configure_option("graph_package", std::string("PHG"));
  p.initialize(mesh);
  p.partition_graph();
  //p.show_changes();

  BOOST_CHECK(true);

  boost::this_thread::sleep(boost::posix_time::milliseconds(20));

  // -----------------------------------------------------------------------------
  // SEND ELEMENTS AND NODES FROM PARTITIONING ALGORITHM

  std::vector<Component::Ptr> mesh_element_comps = mesh.elements().components();

  std::vector<std::vector<Uint> > export_nodes(PE::instance().size());
  std::vector<std::vector<std::vector<Uint> > > export_elems(mesh_element_comps.size());
  for (Uint i=0; i<export_elems.size(); ++i)
    export_elems[i].resize(PE::instance().size());
  Uint component_idx;
  Uint loc_idx;
  bool found;
  foreach_container((const Uint glb_obj) (const Uint part), p.changes())
  {
    boost::tie(component_idx,loc_idx,found) = p.to_local_indices_from_glb_obj(glb_obj);

    if (component_idx==0)
    {
      std::cout << "export node " << loc_idx << std::endl;
      export_nodes[part].push_back(loc_idx);
    }
    else
    {
      std::cout << "export elem " << mesh_element_comps[component_idx-1]->uri().path() << "["<<loc_idx<<"]" << std::endl;
      export_elems[component_idx-1][part].push_back(loc_idx);
    }
    std::cout << "  to part " << part << std::endl;
  }

  std::vector<mpi::Buffer> send_to_proc(PE::instance().size());
  mpi::Buffer recv_from_all;

  // Move elements
  for(Uint i=0; i<mesh_element_comps.size(); ++i)
  {
    CElements& elements = mesh_element_comps[i]->as_type<CElements>();
    for (Uint p=0; p<PE::instance().size(); ++p)
      send_to_proc[p].reset();
    recv_from_all.reset();

    RemoveElements remove_element(elements);
    std::vector<Uint> nb_elems_to_send(PE::instance().size());
    for (Uint p=0; p<PE::instance().size(); ++p)
    {
      nb_elems_to_send[p]=export_elems[i][p].size();
      for (Uint e=0; e<export_elems[i][p].size(); ++e)
      {
        send_to_proc[p] << PackedElement(elements,export_elems[i][p][e]);
        remove_element(export_elems[i][p][e]);
      }
    }
    remove_element.flush();

    std::vector<Uint> nb_elems_to_recv(PE::instance().size());
    PE::instance().all_to_all(nb_elems_to_send,nb_elems_to_recv);
    my_all_to_all(send_to_proc,recv_from_all);

    UnpackAndAddElements add_element(elements);
    for (Uint p=0; p<nb_elems_to_recv.size(); ++p)
    {
      for (Uint e=0; e<nb_elems_to_recv[p]; ++e)
      {
        recv_from_all >> add_element;
      }
    }
    add_element.flush();
  }

  // Move nodes
   for (Uint p=0; p<PE::instance().size(); ++p)
    send_to_proc[p].reset();
   recv_from_all.reset();

   //RemoveNodes remove_node(nodes);
   std::vector<Uint> nb_nodes_to_send_to_proc(PE::instance().size());
   for (Uint p=0; p<PE::instance().size(); ++p)
   {
     nb_nodes_to_send_to_proc[p]=export_nodes[p].size();
     for (Uint e=0; e<export_nodes[p].size(); ++e)
     {
       send_to_proc[p] << PackedNode(nodes,export_nodes[p][e]);
       remove_node(export_nodes[p][e]);
     }
   }
   remove_node.flush();

   std::vector<Uint> nb_nodes_to_recv_from_proc(PE::instance().size());
   PE::instance().all_to_all(nb_nodes_to_send_to_proc,nb_nodes_to_recv_from_proc);
   my_all_to_all(send_to_proc,recv_from_all);

   UnpackAndAddNodes add_node(nodes);
   for (Uint p=0; p<nb_nodes_to_recv_from_proc.size(); ++p)
   {
     for (Uint e=0; e<nb_nodes_to_recv_from_proc[p]; ++e)
     {
       recv_from_all >> add_node;
     }
   }
   add_node.flush();
  // -----------------------------------------------------------------------------
  // ELEMENTS AND NODES HAVE BEEN MOVED
  // -----------------------------------------------------------------------------

  boost::this_thread::sleep(boost::posix_time::milliseconds(20));

  // -----------------------------------------------------------------------------
  // COLLECT NODES TO LOOK FOR ON OTHER PROCESSORS

  std::set<Uint> owned_nodes;
  std::set<Uint> ghost_nodes;
  for (Uint n=0; n<nodes.size(); ++n)
  {
    nodes.rank()[n] = my_rank;
    owned_nodes.insert(nodes.glb_idx()[n]);
  }
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

  PEProcessSortedExecute(-1,
                         std::cout << PERank << "owns nodes" << nodes.glb_idx() << std::endl;
  std::cout << PERank << "look for = " << request_nodes << std::endl;
  boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  )
  BOOST_CHECK(true);

  // -----------------------------------------------------------------------------
  // COMMUNICATE NODES TO LOOK FOR

  std::vector<std::vector<Uint> > recv_request_nodes;
  my_all_gather(request_nodes,recv_request_nodes);

  BOOST_CHECK(true);

  if (PE::instance().rank() == 0)
  {
    std::cout << "[*] everybody is looking for = ";
    for (Uint i=0; i<recv_request_nodes.size(); ++i)
      std::cout << recv_request_nodes[i] << "     ";
    std::cout << std::endl;
  }
  BOOST_CHECK(true);

  // -----------------------------------------------------------------------------
  // SEARCH FOR REQUESTED NODES

  std::vector<std::vector<Uint> > found_nodes(PE::instance().size());
  std::vector<mpi::Buffer> nodes_to_send(PE::instance().size());
  std::vector<Uint> nb_nodes_to_send(PE::instance().size(),0);
  PEProcessSortedExecute(-1,
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
          if (glb_idx == find_glb_idx)
          {
            //          found_nodes[proc].push_back(loc_idx);
            found_nodes[proc].push_back(glb_idx);
            nodes_to_send[proc] << PackedNode(nodes,loc_idx);
            ++nb_nodes_to_send[proc];
            break;
          }
          ++loc_idx;
        }
        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      }
    }
  }
  std::cout << PERank << "found_nodes = ";
  for (Uint i=0; i<found_nodes.size(); ++i)
    std::cout << found_nodes[i] << "     ";
  std::cout << std::endl;
  BOOST_CHECK(true);
  )

  // -----------------------------------------------------------------------------
  // COMMUNICATE FOUND NODES BACK TO RANK THAT REQUESTED IT

  std::vector<std::vector<Uint> > received_nodes;
  my_all_to_all(found_nodes,received_nodes);

  std::cout << PERank << "received_nodes = ";
  for (Uint i=0; i<received_nodes.size(); ++i)
    std::cout << received_nodes[i] << "     ";
  std::cout << std::endl;

  BOOST_CHECK(true);

  std::cout << PERank << "nb_nodes_to_send = " << nb_nodes_to_send << std::endl;
  std::vector<Uint> nb_nodes_to_recv(PE::instance().size(),0);
  PE::instance().all_to_all(nb_nodes_to_send,nb_nodes_to_recv);
  std::cout << PERank << "nb_nodes_to_recv = " << nb_nodes_to_recv << std::endl;

  mpi::Buffer received_nodes_buffer;
  my_all_to_all(nodes_to_send,received_nodes_buffer);

  //UnpackAndAddNodes add_node(nodes);
  for (Uint p=0; p<PE::instance().size(); ++p)
  {
    std::cout << PERank << "unpacking " << nb_nodes_to_recv[p] << " nodes from proc " << p << std::endl;
    for (Uint n=0; n<nb_nodes_to_recv[p]; ++n)
    {
      received_nodes_buffer >> add_node;
    }
  }
  add_node.flush();

  // -----------------------------------------------------------------------------
  // REQUESTED NODES HAVE NOW BEEN ADDED
  // -----------------------------------------------------------------------------

  // -----------------------------------------------------------------------------
  // FIX NODE CONNECTIVITY
  std::map<Uint,Uint> glb_to_loc;
  for (Uint n=0; n<nodes.size(); ++n)
  {
    glb_to_loc[nodes.glb_idx()[n]]=n;
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

  // -----------------------------------------------------------------------------
  // RENUMBER NODES AND ELEMENTS SEPARATELY

  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_elem_numbering")->transform(mesh);

  // -----------------------------------------------------------------------------
  // MESH IS NOW COMPLETELY LOAD BALANCED
  // -----------------------------------------------------------------------------

    CMeshWriter::Ptr msh_writer =
        build_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","msh_writer");

    //msh_writer->set_fields(fields);
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

