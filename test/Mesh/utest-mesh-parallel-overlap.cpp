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

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"

#include "Common/Foreach.hpp"
#include "Common/OSystem.hpp"
#include "Common/OSystemLayer.hpp"

#include "Common/PE/CommPattern.hpp"
#include "Common/PE/CommWrapperMArray.hpp"
#include "Common/PE/Buffer.hpp"
#include "Common/PE/debug.hpp"

#include "Math/Consts.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshElements.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshGenerator.hpp"
#include "Mesh/CMeshPartitioner.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Manipulations.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CSpace.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Common::PE;
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
  PE::Comm::instance().all_gather((int)send.size(),strides);
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
    MPI_CHECK_RESULT(MPI_Allgatherv, ((void*)&send[0], (int)send.size(), get_mpi_datatype<T>(), &recv_linear[0], &strides[0], &displs[0], get_mpi_datatype<T>(), PE::Comm::instance().communicator()));
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

  std::vector<int> recv_strides(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];

  std::vector<Uint> recv_linear(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, (&send_linear[0], &send_strides[0], &send_displs[0], PE::get_mpi_datatype<Uint>(), &recv_linear[0], &recv_strides[0], &recv_displs[0], get_mpi_datatype<Uint>(), PE::Comm::instance().communicator()));

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

void my_all_to_all(const std::vector<PE::Buffer>& send, PE::Buffer& recv)
{
  std::vector<int> send_strides(send.size());
  std::vector<int> send_displs(send.size());
  for (Uint i=0; i<send.size(); ++i)
    send_strides[i] = send[i].packed_size();

  if (send.size()) send_displs[0] = 0;
  for (Uint i=1; i<send.size(); ++i)
    send_displs[i] = send_displs[i-1] + send_strides[i-1];

  PE::Buffer send_linear;

  send_linear.resize(send_displs.back()+send_strides.back());
  for (Uint i=0; i<send.size(); ++i)
    send_linear.pack(send[i].buffer(),send[i].packed_size());

  std::vector<int> recv_strides(PE::Comm::instance().size());
  std::vector<int> recv_displs(PE::Comm::instance().size());
  PE::Comm::instance().all_to_all(send_strides,recv_strides);
  if (recv_displs.size()) recv_displs[0] = 0;
  for (Uint i=1; i<PE::Comm::instance().size(); ++i)
    recv_displs[i] = recv_displs[i-1] + recv_strides[i-1];
  recv.reset();
  recv.resize(recv_displs.back()+recv_strides.back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send_linear.buffer(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.buffer(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::Comm::instance().communicator()));
  recv.packed_size()=recv_displs.back()+recv_strides.back();
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
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)send.buffer(), &send_strides[0], &send_displs[0], MPI_PACKED, (void*)recv.buffer(), &recv_strides[0], &recv_displs[0], MPI_PACKED, PE::Comm::instance().communicator()));
  recv.packed_size()=recv_displs.back()+recv_strides.back();
}


bool check_nodes_sanity(Geometry& nodes)
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

bool check_element_nodes_sanity(CMesh& mesh)
{
  bool sane = true;

  boost_foreach( CEntities& entities, mesh.topology().elements_range())
  {
    Uint max_node_idx = entities.geometry().size();

    for (Uint e=0; e<entities.size(); ++e)
    {
      boost_foreach(Uint node, entities.get_nodes(e))
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


bool check_elements_sanity(CEntities& entities)
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
  Geometry& nodes = mesh.geometry();

  PackUnpackNodes copy_node(nodes);
  PE::Buffer buf;
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

#define GEN

#ifdef GEN
  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");
  meshgenerator->configure_option("mesh",URI("//Root/rect"));
  std::vector<Uint> nb_cells(2);
  std::vector<Real> lengths(2);
  nb_cells[0] = 100;
  nb_cells[1] = 100;
  lengths[0]  = nb_cells[0];
  lengths[1]  = nb_cells[1];
  meshgenerator->configure_option("nb_cells",nb_cells);
  meshgenerator->configure_option("lengths",lengths);
  meshgenerator->configure_option("bdry",true);
  CMesh& mesh = meshgenerator->generate();
#endif

#ifdef NEU
  CMeshReader::Ptr meshreader =
      build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
//  meshreader->configure_option("read_boundaries",false);
  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("rotation-tg-p1.neu");
//  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("quadtriag.neu");
  CMesh& mesh = *mesh_ptr;
#endif

#ifdef GMSH
  CMeshReader::Ptr meshreader =
      build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");
//  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("sinusbump-tg-p1.msh");
  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("quadtriag.msh");
//  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("rectangle-tg-p1.msh");
  CMesh& mesh = *mesh_ptr;
#endif


  Core::instance().root().add_component(mesh);
  Geometry& nodes = mesh.geometry();

  CMeshWriter::Ptr tec_writer =
      build_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","tec_writer");

  CMeshWriter::Ptr gmsh_writer =
      build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","gmsh_writer");


  tec_writer->write_from_to(mesh,"parallel_overlap_before"+tec_writer->get_extensions()[0]);
  CFinfo << "parallel_overlap_before_P*"+tec_writer->get_extensions()[0]+" written" << CFendl;

  gmsh_writer->write_from_to(mesh,"parallel_overlap_before"+gmsh_writer->get_extensions()[0]);
  CFinfo << "parallel_overlap_before_P*"+gmsh_writer->get_extensions()[0]+" written" << CFendl;

  CFinfo << "Global Numbering..." << CFendl;
//  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balancer")->transform(mesh);
  CMeshTransformer::Ptr glb_numbering = build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering");
//  glb_numbering->configure_option("debug",true);
  glb_numbering->transform(mesh);
  CFinfo << "Global Numbering... done" << CFendl;

  CFinfo << "Global Connectivity..." << CFendl;
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_node_elem_connectivity")->transform(mesh);
  CFinfo << "Global Connectivity... done" << CFendl;

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

 // build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);
 // build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_elem_numbering")->transform(mesh);

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

//  CFinfo << "Global Numbering..." << CFendl;
//  glb_numbering->transform(mesh);
//  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering")->transform(mesh);
//  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);
//  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_elem_numbering")->transform(mesh);
//  CFinfo << "Global Numbering... done" << CFendl;
//  CFinfo << "Global Connectivity..." << CFendl;
//  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_node_elem_connectivity")->transform(mesh);
//  CFinfo << "Global Connectivity... done" << CFendl;

  std::set<Uint> debug_nodes;


  /// @todo debug
  const std::vector<Component::Ptr>& mesh_elements = mesh.elements().components();

  std::vector<std::set<Uint> > debug_elems(mesh_elements.size());


  Uint nb_overlap=1;
  for (Uint o=0; o<nb_overlap; ++o)
  {
    CFinfo << "Growing overlap..." << CFendl;
    build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.GrowOverlap","grow_overlap")->transform(mesh);
    CFinfo << "Growing overlap... done" << CFendl;
  }

#if 0
  Uint nb_overlap=4;
  // Grow overlap nb_overlap times
  for (Uint o=0; o<nb_overlap; ++o)
  {
  CFinfo << "Growing Overlap... " << CFendl;

  debug_nodes.clear();
  boost_foreach( std::set<Uint>& elems, debug_elems)
      elems.clear();


  CFaceCellConnectivity& face2cell = mesh.create_component<CFaceCellConnectivity>("face2cell");
  face2cell.setup(mesh.topology());

//  std::cout << PERank << "nb_faces = " << face2cell.size() << std::endl;

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
      std::cout << PERank << "node glb idx " << nodes.glb_idx()[n] << " already exists..." << std::endl;
    }
  }

  std::map<Uint,Uint> glb_elem_2_loc_elem;
  std::map<Uint,Uint>::iterator glb_elem_not_found = glb_elem_2_loc_elem.end();
  for (Uint e=0; e<mesh.elements().size(); ++e)
  {
    Component::Ptr comp;
    Uint idx;

    boost::tie(comp,idx) = mesh.elements().location(e);
    if ( CElements::Ptr elements = comp->as_ptr<CElements>() )
    {
      if ( glb_elem_2_loc_elem.find(elements->glb_idx()[idx]) == glb_elem_not_found )
      {
        glb_elem_2_loc_elem[elements->glb_idx()[idx]] = e;
      }
      else
      {
        std::cout << PERank << "elem glb idx " << elements->glb_idx()[idx] << " already exists..." << std::endl;
      }
    }
  }

  std::set<Uint> bdry_nodes;
  if ( find_components_recursively_with_tag<CCellFaces>(mesh.topology(),"outer_faces").size() > 0 )
  {
    boost_foreach(CCellFaces& bdry_faces, find_components_recursively_with_tag<CCellFaces>(mesh.topology(),"outer_faces"))
    {
      for (Uint e=0; e<bdry_faces.size(); ++e)
      {
        BOOST_CHECK( bdry_faces.is_bdry(e) );

        boost_foreach(const Uint node, bdry_faces.get_nodes(e))
            bdry_nodes.insert(nodes.glb_idx()[node]);
      }
    }
  }
  else
  {
    for (Uint f=0; f<face2cell.size(); ++f)
    {
      cf_assert(f < face2cell.is_bdry_face().size());
      if (face2cell.is_bdry_face()[f])
      {
        boost_foreach(const Uint node, face2cell.face_nodes(f))
          bdry_nodes.insert(nodes.glb_idx()[node]);
      }
    }
    boost_foreach (CFaces& faces, find_components_recursively<CFaces>(mesh.topology()))
    {
      boost_foreach (CConnectivity::Row face_nodes, faces.node_connectivity().array())
      {
        boost_foreach(const Uint node, face_nodes)
        {
          bdry_nodes.insert(nodes.glb_idx()[node]);
        }
      }
    }
    mesh.remove_component(face2cell);
  }
//  PEProcessSortedExecute(-1,
//  std::cout << PERank << "bdry_nodes (" << bdry_nodes.size() << ") = ";
//  boost_foreach(const Uint n, bdry_nodes)
//      std::cout << n << " ";
//  std::cout << std::endl;
//  )

  // -----------------------------------------------------------------------------
  // SEARCH FOR CONNECTED ELEMENTS
  // in  : nodes                            std::vector<Uint>
  // out : buffer with packed elements      PE::Buffer(nodes)

  // COMMUNICATE NODES TO LOOK FOR


  std::vector<Uint> send_nodes; send_nodes.reserve(bdry_nodes.size());
  boost_foreach(const Uint n, bdry_nodes)
    send_nodes.push_back(n);
  std::vector<std::vector<Uint> > recv_nodes;
  my_all_gather(send_nodes,recv_nodes);



  // elem_idx_to_send[from_comp][to_proc][elem_idx]
  std::vector< std::vector < std::set<Uint> > > elem_ids_to_send(mesh_elements.size());
  for (Uint comp_idx=0; comp_idx<elem_ids_to_send.size(); ++comp_idx)
    elem_ids_to_send[comp_idx].resize(PE::Comm::instance().size());


  // storage for nodes that will need to be fetched after elements have been received
  std::set<Uint> new_ghost_nodes;

  for (Uint proc=0; proc<Comm::instance().size(); ++proc)
  {
    if (proc != Comm::instance().rank())
    {

      for (Uint n=0; n<recv_nodes[proc].size(); ++n)
      {
        Uint find_glb_node_idx = recv_nodes[proc][n];

        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
        /// @todo THIS ALGORITHM HAS TO BE IMPROVED (BRUTE FORCE)
        if ( glb_node_2_loc_node.find(find_glb_node_idx) != glb_node_not_found)
        {
          Uint loc_idx = glb_node_2_loc_node[find_glb_node_idx];
          CDynTable<Uint>::ConstRow connected_elements = nodes.glb_elem_connectivity()[loc_idx];
          boost_foreach ( const Uint glb_elem_idx, nodes.glb_elem_connectivity()[loc_idx] )
          {

            if ( glb_elem_2_loc_elem.find(glb_elem_idx) != glb_elem_not_found)
            {
              Uint unif_elem_idx = glb_elem_2_loc_elem[glb_elem_idx];

              Uint elem_comp_idx;
              Uint elem_idx;
              boost::tie(elem_comp_idx,elem_idx) = mesh.elements().location_idx(unif_elem_idx);

              if (mesh_elements[elem_comp_idx]->as_type<CElements>().is_ghost(elem_idx) == false)
              {
                elem_ids_to_send[elem_comp_idx][proc].insert(elem_idx);
                debug_elems[elem_comp_idx].insert(elem_idx);
              }



            }

          }
          debug_nodes.insert(loc_idx);
        }
        // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      }
    }
  }
#if 1

  BOOST_CHECK(check_nodes_sanity(nodes));


  std::vector<Uint> old_elem_size(mesh_elements.size());
  std::vector<Uint> new_elem_size(mesh_elements.size());

  for (Uint comp_idx=0; comp_idx<mesh_elements.size(); ++comp_idx)
  {
    if (CElements::Ptr elements_ptr = mesh_elements[comp_idx]->as_ptr<CElements>())
    {
      CElements& elements = *elements_ptr;
      PackUnpackElements copy(elements);

      std::vector<PE::Buffer> elements_to_send(Comm::instance().size());
      PE::Buffer elements_to_recv;

      // Pack
      for (Uint to_proc = 0; to_proc<PE::Comm::instance().size(); ++to_proc)
      {
        boost_foreach(const Uint elem_idx, elem_ids_to_send[comp_idx][to_proc])
        {

          elements_to_send[to_proc] << elements.glb_idx()[elem_idx]
                                    << elements.rank()[elem_idx];

          boost_foreach(const Uint connected_node, elements.node_connectivity()[elem_idx])
              elements_to_send[to_proc] << nodes.glb_idx()[connected_node];

        }
      }

      // Communicate
      my_all_to_all(elements_to_send,elements_to_recv);

      // Save old_size
      old_elem_size[comp_idx] = elements.size();

      // Unpack
      while (elements_to_recv.more_to_unpack())
      {
        elements_to_recv >> copy;
      }

      copy.flush();

      new_elem_size[comp_idx] = elements.size();

      RemoveElements remove(elements);
      for (Uint e=old_elem_size[comp_idx]; e<new_elem_size[comp_idx]; ++e)
      {
        if ( glb_elem_2_loc_elem.find(elements.glb_idx()[e]) != glb_elem_not_found )
        {
          remove(e);
        }
      }
      remove.flush();
      new_elem_size[comp_idx] = elements.size();

      BOOST_CHECK(check_elements_sanity(elements));
      //      std::cout << PERank << elements.uri().path() << " grew from size " << old_elem_size[comp_idx] << " to " << new_elem_size[comp_idx] << std::endl;

      std::set<Uint>::iterator found_bdry_node;
      std::set<Uint>::iterator not_found = bdry_nodes.end();
      for (Uint e=old_elem_size[comp_idx]; e<new_elem_size[comp_idx]; ++e)
      {
        boost_foreach(const Uint connected_glb_node, elements.node_connectivity()[e])
        {
          if ( glb_node_2_loc_node.find(connected_glb_node) == glb_node_not_found)
            new_ghost_nodes.insert(connected_glb_node);
        }
      }


    }
    else
    {
      /// @todo case of non-CElements
    }
  }



//  PEProcessSortedExecute(-1,
//  std::cout << PERank << "new_ghost_nodes (" << new_ghost_nodes.size() << ") = ";
//  boost_foreach(const Uint n, new_ghost_nodes)
//      std::cout << n << " ";
//  std::cout << std::endl;
//  )

  BOOST_CHECK(check_nodes_sanity(nodes));

  // -----------------------------------------------------------------------------
  // SEARCH FOR REQUESTED NODES
  // in  : requested nodes                std::vector<Uint>
  // out : buffer with packed nodes       PE::Buffer(nodes)
  {

    // COMMUNICATE NODES TO LOOK FOR

    std::vector<Uint> request_nodes; request_nodes.reserve(new_ghost_nodes.size());
    boost_foreach(const Uint n, new_ghost_nodes)
      request_nodes.push_back(n);

    std::vector<std::vector<Uint> > recv_request_nodes;
    my_all_gather(request_nodes,recv_request_nodes);


    PackUnpackNodes copy_node(nodes);
    std::vector<PE::Buffer> nodes_to_send(Comm::instance().size());
    for (Uint proc=0; proc<Comm::instance().size(); ++proc)
    {
      if (proc != Comm::instance().rank())
      {

        for (Uint n=0; n<recv_request_nodes[proc].size(); ++n)
        {
          Uint find_glb_idx = recv_request_nodes[proc][n];

          // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
          /// @todo THIS ALGORITHM HAS TO BE IMPROVED (BRUTE FORCE)
          if ( glb_node_2_loc_node.find(find_glb_idx) != glb_node_not_found)
          {
            Uint loc_idx = glb_node_2_loc_node[find_glb_idx];
              //std::cout << PERank << "copying node " << glb_idx << " from loc " << loc_idx << std::flush;
            if (nodes.is_ghost(loc_idx) == false)
              nodes_to_send[proc] << copy_node(loc_idx,PackUnpackNodes::COPY);
          }
          // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
        }
      }
    }
    BOOST_CHECK(check_nodes_sanity(nodes));
    // COMMUNICATE FOUND NODES BACK TO RANK THAT REQUESTED IT

    PE::Buffer received_nodes_buffer;
    my_all_to_all(nodes_to_send,received_nodes_buffer);

    // out: buffer containing requested nodes
    // -----------------------------------------------------------------------------

    // ADD GHOST NODES
    BOOST_CHECK(check_nodes_sanity(nodes));


    Uint old_nodes_size = nodes.size();
    PackUnpackNodes add_node(nodes);
    while (received_nodes_buffer.more_to_unpack())
      received_nodes_buffer >> add_node;
    add_node.flush();
    Uint new_nodes_size = nodes.size();

    //    std::cout << PERank << "nodes grew from size " << old_nodes_size << " to " << new_nodes_size << std::endl;
    // -----------------------------------------------------------------------------
    // REQUESTED GHOST-NODES HAVE NOW BEEN ADDED
    // -----------------------------------------------------------------------------
    BOOST_CHECK(check_nodes_sanity(nodes));

    // -----------------------------------------------------------------------------
    // FIX NODE CONNECTIVITY
    std::map<Uint,Uint> glb_to_loc;
    std::map<Uint,Uint>::iterator it;
    bool inserted;
    for (Uint n=0; n<nodes.size(); ++n)
    {
      boost::tie(it,inserted) = glb_to_loc.insert(std::make_pair(nodes.glb_idx()[n],n));
//      if (! inserted)
//        throw ValueExists(FromHere(), std::string(nodes.is_ghost(n)? "ghost " : "" ) + "node["+to_str(n)+"] with glb_idx "+to_str(nodes.glb_idx()[n])+" already exists as "+to_str(glb_to_loc[n]));
    }
    BOOST_CHECK(true);
    for (Uint comp_idx=0; comp_idx<mesh_elements.size(); ++comp_idx)
    {
      if (CElements::Ptr elements_ptr = mesh_elements[comp_idx]->as_ptr<CElements>())
      {
        CElements& elements = *elements_ptr;

        for (Uint e=old_elem_size[comp_idx]; e < new_elem_size[comp_idx]; ++e)
        {
          CConnectivity::Row connected_nodes = elements.node_connectivity()[e];

          boost_foreach ( Uint& node, connected_nodes )
          {
            node = glb_to_loc[node];
          }

        }

      }

    }

    // debug
    boost_foreach(const Uint node, new_ghost_nodes)
    {
      debug_nodes.insert(glb_to_loc[node]);
    }

    BOOST_CHECK( check_element_nodes_sanity(mesh) );

  }
#endif

mesh.elements().reset();
mesh.elements().update();
mesh.update_statistics();
CFinfo << "Growing Overlap... done" << CFendl;
  }

#endif


#if 1

//  CFinfo << mesh.tree() << CFendl;

BOOST_CHECK(true);

  Field& glb_node = mesh.geometry().create_field("glb_node");
  boost_foreach(const Uint node, debug_nodes)
      glb_node[node][0] = 1.;

  // Create a field with glb element numbers
  FieldGroup& elems_P0 = mesh.create_space_and_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED,"CF.Mesh.LagrangeP0");
  Field& glb_elem  = elems_P0.create_field("glb_elem");
  Field& elem_rank = elems_P0.create_field("elem_rank");

  for(Uint comp_idx=0; comp_idx < mesh_elements.size(); ++comp_idx)
  {
    CEntities& elements = mesh_elements[comp_idx]->as_type<CEntities>();
    CSpace& space = glb_elem.space(elements);
    boost_foreach (const Uint elem, debug_elems[comp_idx])
    {
      Uint field_idx = space.indexes_for_element(elem)[0];
      glb_elem[field_idx][0] = 1.;
      elem_rank[field_idx][0] = elements.rank()[elem];
    }
  }
BOOST_CHECK(true);

  std::vector<Field::Ptr> fields_to_output;
//  fields_to_output.push_back(glb_node.as_ptr<Field>());
//  fields_to_output.push_back(glb_elem.as_ptr<Field>());
  fields_to_output.push_back(elem_rank.as_ptr<Field>());
BOOST_CHECK(true);
  tec_writer->set_fields(fields_to_output);
  tec_writer->write_from_to(mesh,"parallel_overlap"+tec_writer->get_extensions()[0]);
  CFinfo << "parallel_overlap_P*"+tec_writer->get_extensions()[0]+" written" << CFendl;
BOOST_CHECK(true);
  gmsh_writer->set_fields(fields_to_output);
  gmsh_writer->write_from_to(mesh,"parallel_overlap"+gmsh_writer->get_extensions()[0]);
  CFinfo << "parallel_overlap_P*"+gmsh_writer->get_extensions()[0]+" written" << CFendl;
#endif

}

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

