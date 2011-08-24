// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::BlockMesh::BlockMeshMPI"

#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/MPI/PE.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

struct BockMesh3DFixture :
  public Tools::Testing::TimedTestFixture
{
  BockMesh3DFixture()
  {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;
    
    cf_assert(argc == 4);
    x_segs = boost::lexical_cast<Uint>(argv[1]);
    y_segs = boost::lexical_cast<Uint>(argv[2]);
    z_segs = boost::lexical_cast<Uint>(argv[3]);
    
    if(!Comm::PE::instance().is_active())
      Comm::PE::instance().init(argc, argv);
    
    CRoot& root = Core::instance().root();
    if(!root.get_child_ptr("domain"))
    {
      m_domain = root.create_component_ptr<CDomain>("domain");
    }
    else
    {
      m_domain = root.get_child("domain").as_ptr<CDomain>();
    }
    
    if(!domain().get_child_ptr("writer"))
    {
      m_writer = domain().add_component(build_component_abstract_type<CMeshWriter>("CF.Mesh.VTKXML.CWriter", "writer")).as_ptr<CMeshWriter>();
    }
    else
    {
      m_writer = domain().get_child("writer").as_ptr<CMeshWriter>();
    }
    
    if(!domain().get_child_ptr("block_mesh"))
    {
      m_block_mesh = domain().create_component_ptr<CMesh>("block_mesh");
    }
    else
    {
      m_block_mesh = domain().get_child("block_mesh").as_ptr<CMesh>();
    }
    
    if(!domain().get_child_ptr("mesh"))
    {
      m_mesh = domain().create_component_ptr<CMesh>("mesh");
    }
    else
    {
      m_mesh = domain().get_child("mesh").as_ptr<CMesh>();
    }
  }
  
  CDomain& domain()
  {
    return *m_domain.lock();
  }
  
  CMeshWriter& writer()
  {
    return *m_writer.lock();
  }
  
  CMesh& block_mesh()
  {
    return *m_block_mesh.lock();
  }
  
  CMesh& mesh()
  {
    return *m_mesh.lock();
  }
  
  Uint x_segs, y_segs, z_segs;
  
  boost::weak_ptr<CDomain> m_domain;
  boost::weak_ptr<CMesh> m_block_mesh; // Mesh containing the blocks (helper for parallelization)
  boost::weak_ptr<CMesh> m_mesh; // Actual generated mesh
  boost::weak_ptr<CMeshWriter> m_writer;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BlockMesh3D, BockMesh3DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( GenerateMesh )
{
  const Uint nb_procs = Comm::PE::instance().size();
  const Uint rank = Comm::PE::instance().rank();
  
  const Real length = 12.;
  const Real half_height = 0.5;
  const Real width = 6.;
  const Real ratio = 0.1;

  BlockMesh::BlockData serial_blocks, parallel_blocks;
  
  // Create blocks for a 3D channel
  Tools::MeshGeneration::create_channel_3d(serial_blocks, length, half_height, width, x_segs, y_segs/2, z_segs, ratio);

  // partition blocks
  BlockMesh::partition_blocks(serial_blocks, block_mesh(), nb_procs, XX, parallel_blocks);
  
  // Gnerate the actual mesh
  BlockMesh::build_mesh(parallel_blocks, mesh());
}

BOOST_AUTO_TEST_CASE( WriteMesh )
{
  writer().write_from_to(mesh(), "utest-blockmesh-3d-mpi_output.pvtu");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

