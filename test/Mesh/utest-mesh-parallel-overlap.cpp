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

#include "Math/Consts.hpp"

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
using namespace CF::Math::Consts;

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
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send.buffer(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.buffer(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::instance().communicator()));
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
//  Core::instance().environment().configure_option("log_level",(Uint)DEBUG);

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
//  glb_numbering->configure_option("debug",true);
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
  CFinfo << "Partitioning... done" << CFendl;

  CFinfo << "Migration..." << CFendl;
  p.migrate();
  CFinfo << "Migration... done" << CFendl;

  // -----------------------------------------------------------------------------
  // RENUMBER NODES AND ELEMENTS SEPARATELY

  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_elem_numbering")->transform(mesh);

  // -----------------------------------------------------------------------------
  // MESH IS NOW COMPLETELY LOAD BALANCED WITHOUT OVERLAP
  // -----------------------------------------------------------------------------


  // -----------------------------------------------------------------------------
  // -----------------------------------------------------------------------------
  // -----------------------------------------------------------------------------
  //                                GROW OVERLAP
  // -----------------------------------------------------------------------------
  // -----------------------------------------------------------------------------
  // -----------------------------------------------------------------------------


  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBuildFaces","build_faces")->transform(mesh);

  CFinfo << mesh.tree() << CFendl;

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

