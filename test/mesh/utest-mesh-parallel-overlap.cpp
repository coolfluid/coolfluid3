// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for parallel fields"

#include <iomanip>
#include <set>

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "common/Foreach.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

#include "common/PE/CommPattern.hpp"
#include "common/PE/CommWrapperMArray.hpp"
#include "common/PE/Buffer.hpp"
#include "common/PE/debug.hpp"

#include "math/Consts.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/MeshPartitioner.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/CellFaces.hpp"
#include "mesh/Space.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::math::Consts;

template <typename T>
std::ostream& operator<< (std::ostream& out , const std::vector<T>& v)
{
  for (Uint i=0; i<v.size()-1; ++i)
    out << v[i] << " ";
  if (v.size())
    out << v.back();
  return out;
}

void my_all_to_all(const std::vector<PE::Buffer>& send, PE::Buffer& recv)
{
  std::vector<int> send_strides(send.size());
  std::vector<int> send_displs(send.size());
  for (Uint i=0; i<send.size(); ++i)
    send_strides[i] = send[i].size();

  if (send.size()) send_displs[0] = 0;
  for (Uint i=1; i<send.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  PE::Buffer send_linear;

  send_linear.reserve(send_displs.back()+send_strides.back());
  for (Uint i=0; i<send.size(); ++i)
    send_linear.pack(send[i].begin(),send[i].size());

  std::vector<int> recv_strides(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  if (recv_displs.size()) recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];
  recv.reset();
  recv.resize(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send_linear.begin(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.begin(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::Comm::instance().communicator()));
}

////////////////////////////////////////////////////////////////////////////////

void my_all_to_all(const PE::Buffer& send, std::vector<int>& send_strides, PE::Buffer& recv, std::vector<int>& recv_strides)
{
  std::vector<int> send_displs(send_strides.size());
  if (send_strides.size()) send_displs[0] = 0;
  for (Uint i=1; i<send_strides.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  recv_strides.resize(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  if (recv_displs.size()) recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];
  recv.reset();
  recv.resize(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send.begin(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.begin(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::Comm::instance().communicator()));
}


bool check_nodes_sanity(Dictionary& nodes)
{
  bool sane = true;
  std::map<Uint,Uint> glb_node_2_loc_node;
  std::map<Uint,Uint>::iterator glb_node_not_found = glb_node_2_loc_node.end();
  for (Uint n=0; n<nodes.size(); ++n)
  {
    if ( glb_node_2_loc_node.find(nodes.glb_idx()[n]) == glb_node_not_found )
    {
      glb_node_2_loc_node[nodes.glb_idx()[n]] = n;
    }
    else
    {
      std::cout << PERank << "glb idx " << nodes.glb_idx()[n] << " already exists...  ("<<n<< "<-->"<<glb_node_2_loc_node[nodes.glb_idx()[n]] << ")" << std::endl;
      sane = false;
    }
  }
  return sane;
}

bool check_element_nodes_sanity(Mesh& mesh)
{
  bool sane = true;

  boost_foreach( Entities& entities, mesh.topology().elements_range())
  {
    Uint max_node_idx = entities.geometry_fields().size();

    for (Uint e=0; e<entities.size(); ++e)
    {
      boost_foreach(Uint node, entities.geometry_space().connectivity()[e])
      {
        if (node >=max_node_idx)
        {
          std::cout << PERank << "element " << e << " has node out of range : " << node << " >= " << max_node_idx << std::endl;
          sane = false;
        }
      }
    }
  }

  return sane;
}


bool check_elements_sanity(Entities& entities)
{
  bool sane = true;
  std::map<Uint,Uint> glb_elem_2_loc_elem;
  std::map<Uint,Uint>::iterator glb_elem_not_found = glb_elem_2_loc_elem.end();
  for (Uint e=0; e<entities.size(); ++e)
  {
    if ( glb_elem_2_loc_elem.find(entities.glb_idx()[e]) == glb_elem_not_found )
    {
      glb_elem_2_loc_elem[entities.glb_idx()[e]] = e;
    }
    else
    {
      std::cout << PERank << "glb elem idx " << entities.glb_idx()[e] << " already exists...  ("<<e<< "<-->"<<glb_elem_2_loc_elem[entities.glb_idx()[e]] << ")" << std::endl;
      sane = false;
    }
  }
  return sane;
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
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( parallelize_and_synchronize )
{
  CFinfo << "ParallelOverlap_test" << CFendl;
  Core::instance().environment().options().configure_option("log_level",(Uint)DEBUG);


  // Create or read the mesh

#define GEN

#ifdef GEN
  boost::shared_ptr< MeshGenerator > meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");
  meshgenerator->options().configure_option("mesh",URI("//rect"));
  std::vector<Uint> nb_cells(2);
  std::vector<Real> lengths(2);
  nb_cells[0] = 50;
  nb_cells[1] = 50;
  lengths[0]  = nb_cells[0];
  lengths[1]  = nb_cells[1];
  meshgenerator->options().configure_option("nb_cells",nb_cells);
  meshgenerator->options().configure_option("lengths",lengths);
  meshgenerator->options().configure_option("bdry",true);
  Mesh& mesh = meshgenerator->generate();
#endif

#ifdef NEU
  boost::shared_ptr< MeshReader > meshreader =
      build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
//  meshreader->options().configure_option("read_boundaries",false);
  boost::shared_ptr< Mesh > mesh_ptr = meshreader->create_mesh_from("rotation-tg-p1.neu");
//  Handle< Mesh > mesh_ptr = meshreader->create_mesh_from("../../resources/quadtriag.neu");
  Mesh& mesh = *mesh_ptr;
  Core::instance().root().add_component(mesh_ptr);
#endif

#ifdef GMSH
  boost::shared_ptr< MeshReader > meshreader =
      build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");
  boost::shared_ptr< Mesh > mesh_ptr = meshreader->create_mesh_from("../../resources/sinusbump-tg-p1.msh");
//  Handle< Mesh > mesh_ptr = meshreader->create_mesh_from("../../resources/quadtriag.msh");
//  Handle< Mesh > mesh_ptr = meshreader->create_mesh_from("../../resources/rectangle-tg-p1.msh");
  Mesh& mesh = *mesh_ptr;
  Core::instance().root().add_component(mesh_ptr);
#endif

  Dictionary& nodes = mesh.geometry_fields();

  boost::shared_ptr< MeshWriter > tec_writer =
      build_component_abstract_type<MeshWriter>("cf3.mesh.tecplot.Writer","tec_writer");

  boost::shared_ptr< MeshWriter > gmsh_writer =
      build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","gmsh_writer");


  tec_writer->write_from_to(mesh,"parallel_overlap_before"+tec_writer->get_extensions()[0]);
  CFinfo << "parallel_overlap_before_P*"+tec_writer->get_extensions()[0]+" written" << CFendl;

  gmsh_writer->write_from_to(mesh,"parallel_overlap_before"+gmsh_writer->get_extensions()[0]);
  CFinfo << "parallel_overlap_before_P*"+gmsh_writer->get_extensions()[0]+" written" << CFendl;

  CFinfo << "Global Numbering..." << CFendl;
//  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balancer")->transform(mesh);
  boost::shared_ptr< MeshTransformer > glb_numbering = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering");
//  glb_numbering->options().configure_option("debug",true);
  glb_numbering->transform(mesh);
  CFinfo << "Global Numbering... done" << CFendl;

  CFinfo << "Global Connectivity..." << CFendl;
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalConnectivity","glb_node_elem_connectivity")->transform(mesh);
  CFinfo << "Global Connectivity... done" << CFendl;

  CFinfo << "Partitioning..." << CFendl;
  boost::shared_ptr< MeshPartitioner > partitioner_ptr = boost::dynamic_pointer_cast<MeshPartitioner>(build_component_abstract_type<MeshTransformer>("cf3.mesh.zoltan.Partitioner","partitioner"));
  MeshPartitioner& p = *partitioner_ptr;
  p.options().configure_option("graph_package", std::string("PHG"));
  p.initialize(mesh);
  p.partition_graph();
//  p.show_changes();
  CFinfo << "Partitioning... done" << CFendl;


  Field& glb_node = mesh.geometry_fields().create_field("glb_node");
  for (Uint node=0; node<mesh.geometry_fields().size(); ++node)
    glb_node[node][0] = mesh.geometry_fields().glb_idx()[node];

  // Create a field with glb element numbers
  Dictionary& elems_P0 = mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");
  Field& glb_elem  = elems_P0.create_field("glb_elem");
  Field& elem_rank = elems_P0.create_field("elem_rank");

  BOOST_CHECK_EQUAL(mesh.dictionaries().size(),2u);
  BOOST_CHECK_EQUAL(mesh.dictionaries()[0]->name() , mesh::Tags::geometry());

  BOOST_CHECK_EQUAL(mesh.elements().size(),5u);

  boost_foreach(const Handle<Space>& space, elems_P0.spaces())
  {
    for (Uint elem=0; elem<space->size(); ++elem)
    {
      const Uint field_idx = space->connectivity()[elem][0];
      glb_elem[field_idx][0] = space->support().glb_idx()[elem]+1;
      elem_rank[field_idx][0] = space->support().rank()[elem];
    }
  }

  CFinfo << glb_node.size() << CFendl;
  CFinfo << glb_elem.size() << CFendl;
  CFinfo << mesh.topology().recursive_elements_count(true) << CFendl;

  // Create a field with glb element numbers
  Dictionary& continuous_P2 = mesh.create_discontinuous_space("continuous_P2","cf3.mesh.LagrangeP2");
  const Field& P2coords = continuous_P2.coordinates();

  // NOTE that in this case migration happens AFTER fields and other spaces have been
  // created
  CFinfo << "Migration..." << CFendl;
  p.migrate();
  CFinfo << "Migration... done" << CFendl;

  MeshAdaptor mesh_adaptor(mesh);
  mesh_adaptor.prepare();
  mesh_adaptor.grow_overlap();
  mesh_adaptor.finish();

  CFinfo << glb_node.size() << CFendl;
  CFinfo << glb_elem.size() << CFendl;
  CFinfo << mesh.topology().recursive_elements_count(true) << CFendl;


BOOST_CHECK(true);

  std::vector<URI> fields_to_output;
  fields_to_output.push_back(glb_node.uri());
  fields_to_output.push_back(glb_elem.uri());
  fields_to_output.push_back(elem_rank.uri());
  fields_to_output.push_back(P2coords.uri());
BOOST_CHECK(true);
  tec_writer->options().configure_option("fields",fields_to_output);
  tec_writer->options().configure_option("enable_overlap",true);
  tec_writer->options().configure_option("mesh",mesh.handle<Mesh>());
  tec_writer->options().configure_option("file",URI("parallel_overlap"+tec_writer->get_extensions()[0]));
  tec_writer->execute();
  CFinfo << "parallel_overlap_P*"+tec_writer->get_extensions()[0]+" written" << CFendl;
BOOST_CHECK(true);
  gmsh_writer->options().configure_option("fields",fields_to_output);
  gmsh_writer->options().configure_option("enable_overlap",true);
  gmsh_writer->options().configure_option("mesh",mesh.handle<Mesh>());
  gmsh_writer->options().configure_option("file",URI("parallel_overlap"+gmsh_writer->get_extensions()[0]));
  gmsh_writer->execute();
  CFinfo << "parallel_overlap_P*"+gmsh_writer->get_extensions()[0]+" written" << CFendl;
}

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

