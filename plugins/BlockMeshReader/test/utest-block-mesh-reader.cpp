// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for BlockMeshReader"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"
#include "Mesh/BlockMesh/WriteDict.hpp"

#include "Tools/MeshDiff/MeshDiff.hpp"
#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "BlockMeshReader/BlockMeshReader.hpp"
#include "BlockMeshReader/Parser.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::BlockMesh;
using namespace CF::BlockMeshReader;

//////////////////////////////////////////////////////////////////////////////

struct BlockMeshReaderFixture
{
  BlockMeshReaderFixture() : root(Core::instance().root())
  {
    int    argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;
    if(argc < 2)
      throw ValueNotFound(FromHere(), "Path to base directory was not found");
    base_dir = URI(argv[1], CF::Common::URI::Scheme::FILE);
  }
  URI base_dir;
  Common::CRoot& root;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BlockMeshReader, BlockMeshReaderFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Channel3D )
{
  BOOST_CHECK(true);

  // files should be in current working directory
  URI dict_path = base_dir / URI("channel3d.dict");
  CMeshReader::Ptr dict_reader = root.create_component("meshreader", "CF.BlockMeshReader.BlockMeshReader").as_ptr<CMeshReader>();

  // Read the dict mesh
  CMesh& dict_mesh = root.create_component< CMesh >("dict_mesh");
  dict_reader->read_mesh_into(dict_path, dict_mesh);

  // Read the reference mesh
  URI ref_path = base_dir / URI("uTestBlockMeshReader-Channel3D-reference.neu");
  CMeshReader::Ptr ref_reader = root.create_component("meshreader", "CF.Mesh.Neu.CReader").as_ptr<CMeshReader>();
  CMesh& ref_mesh = root.create_component< CMesh >("ref_mesh");
  ref_reader->read_mesh_into(ref_path, ref_mesh);

  // Write output
  CMeshWriter& writer = root.create_component("meshwriter", "CF.Mesh.Gmsh.CWriter").as_type<CMeshWriter>();
  writer.write_from_to(dict_mesh, URI("channel3d-output.msh"));

  // Check if they are equal
  //BOOST_CHECK(CF::Tools::MeshDiff::diff(dict_mesh, ref_mesh, 25000));
}
/*
BOOST_AUTO_TEST_CASE( Cavity2D )
{
  BOOST_CHECK(true);

  // files should be in current working directory
  URI dict_path = base_dir / URI("cavity2d.dict");
  CMeshReader::Ptr dict_reader = root.create_component_ptr<CMeshReader>("meshreader");

  // Read the dict mesh
  CMesh::Ptr dict_mesh(allocate_component<CMesh>("dict_mesh"));
  dict_reader->do_read_mesh_into(dict_path, dict_mesh);

    // Read the reference mesh
  URI ref_path = base_dir / URI("uTestBlockMeshReader-Cavity2D-reference.neu");
  CMeshReader::Ptr ref_reader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  CMesh::Ptr ref_mesh(allocate_component<CMesh>("reference"));
  ref_reader->do_read_mesh_into(ref_path, ref_mesh);

  CFinfo << dict_mesh->tree() << CFendl;
  CFinfo << ref_mesh->tree() << CFendl;

  // Check if they are equal
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *ref_mesh, 100));
}

BOOST_AUTO_TEST_CASE( PitzDaily )
{
  BOOST_CHECK(true);

  // files should be in current working directory
  URI dict_path = base_dir / URI("pitzdaily.dict");
  CMeshReader::Ptr dict_reader = root.create_component_ptr<CMeshReader>("meshreader");

  // Read the dict mesh
  CMesh::Ptr dict_mesh(allocate_component<CMesh>("dict_mesh"));
  dict_reader->do_read_mesh_into(dict_path, dict_mesh);

    // Read the reference mesh
  URI ref_path = base_dir / URI("uTestBlockMeshReader-PitzDaily-reference.neu");
  CMeshReader::Ptr ref_reader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  CMesh::Ptr ref_mesh(allocate_component<CMesh>("reference"));
  ref_reader->do_read_mesh_into(ref_path, ref_mesh);

  // Check if they are equal
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *ref_mesh, 50000));
}

BOOST_AUTO_TEST_CASE( WriteDict )
{
  BOOST_CHECK(true);

  URI path = base_dir/URI("pitzdaily.dict");
  boost::filesystem::fstream file;
  file.open(path,std::ios_base::in);

  BlockData ref_block_data;
  parse_blockmesh_dict(file, ref_block_data);

  std::stringstream out_stream;
  out_stream << ref_block_data << std::endl;

  BlockData out_block_data;
  parse_blockmesh_dict(out_stream, out_block_data);

  BOOST_CHECK_EQUAL(ref_block_data, out_block_data);
}


*/
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

