// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for BlockMeshReader"

#include <boost/assign/list_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/SimpleCommunicationPattern.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"
#include "Mesh/BlockMesh/WriteDict.hpp"

#include "Tools/MeshDiff/MeshDiff.hpp"
#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "BlockMeshReader/Parser.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::BlockMesh;
using namespace CF::BlockMeshReader;

//////////////////////////////////////////////////////////////////////////////

struct BlockMeshReaderFixture
{
  BlockMeshReaderFixture()
  {
    int    argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;
    if(argc < 2)
      throw ValueNotFound(FromHere(), "Path to base directory was not found");
    base_dir = boost::filesystem::path(argv[1]);
  }
  boost::filesystem::path base_dir;
  
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BlockMeshReader, BlockMeshReaderFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Channel3D )
{
  // files should be in current working directory
  boost::filesystem::path dict_path = base_dir / boost::filesystem::path("channel3d.dict");
  CMeshReader::Ptr dict_reader = create_component_abstract_type<CMeshReader>("blockMeshDict","meshreader");
  
  // Read the dict mesh
  CMesh::Ptr dict_mesh(new CMesh("dict_mesh"));
  dict_reader->read_from_to(dict_path, dict_mesh);
  
  // Read the reference mesh
  boost::filesystem::path ref_path = base_dir / boost::filesystem::path("uTestBlockMeshReader-Channel3D-reference.neu");
  CMeshReader::Ptr ref_reader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  CMesh::Ptr ref_mesh(new CMesh("reference"));
  ref_reader->read_from_to(ref_path, ref_mesh);
  
  // Check if they are equal
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *ref_mesh, 25000));
}

BOOST_AUTO_TEST_CASE( Cavity2D )
{
  // files should be in current working directory
  boost::filesystem::path dict_path = base_dir / boost::filesystem::path("cavity2d.dict");
  CMeshReader::Ptr dict_reader = create_component_abstract_type<CMeshReader>("blockMeshDict","meshreader");
  
  // Read the dict mesh
  CMesh::Ptr dict_mesh(new CMesh("dict_mesh"));
  dict_reader->read_from_to(dict_path, dict_mesh);
  
    // Read the reference mesh
  boost::filesystem::path ref_path = base_dir / boost::filesystem::path("uTestBlockMeshReader-Cavity2D-reference.neu");
  CMeshReader::Ptr ref_reader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  CMesh::Ptr ref_mesh(new CMesh("reference"));
  ref_reader->read_from_to(ref_path, ref_mesh);
  
  CFinfo << dict_mesh->tree() << CFendl;
  CFinfo << ref_mesh->tree() << CFendl;
  
  // Check if they are equal
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *ref_mesh, 100));
}

BOOST_AUTO_TEST_CASE( PitzDaily )
{
  // files should be in current working directory
  boost::filesystem::path dict_path = base_dir / boost::filesystem::path("pitzdaily.dict");
  CMeshReader::Ptr dict_reader = create_component_abstract_type<CMeshReader>("blockMeshDict","meshreader");
  
  // Read the dict mesh
  CMesh::Ptr dict_mesh(new CMesh("dict_mesh"));
  dict_reader->read_from_to(dict_path, dict_mesh);
  
    // Read the reference mesh
  boost::filesystem::path ref_path = base_dir / boost::filesystem::path("uTestBlockMeshReader-PitzDaily-reference.neu");
  CMeshReader::Ptr ref_reader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  CMesh::Ptr ref_mesh(new CMesh("reference"));
  ref_reader->read_from_to(ref_path, ref_mesh);
  
  // Check if they are equal
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *ref_mesh, 50000));
}

BOOST_AUTO_TEST_CASE( WriteDict )
{
  boost::filesystem::path path = base_dir/boost::filesystem::path("pitzdaily.dict");
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

BOOST_AUTO_TEST_CASE( PartitionBlocks )
{
  if(!PE::instance().is_init())
    PE::instance().init(0,0);
  
  boost::filesystem::path path = base_dir / boost::filesystem::path("channel3d.dict");
  boost::filesystem::fstream file;
  file.open(path,std::ios_base::in);
  
  BlockData block_data;
  parse_blockmesh_dict(file, block_data);
  
  const Uint factor = 12;
  const bool scale = true;
  const Uint nb_procs = 16;
  
  BOOST_FOREACH(BlockData::CountsT& subdivisions, block_data.block_subdivisions)
  {
    for(Uint i = 0; i != subdivisions.size(); ++i)
      subdivisions[i] *= ((i == 0 && scale) ? factor*nb_procs : factor);
  }
  
  BlockData partitioned_blocks;
  partition_blocks(block_data, nb_procs, CF::Mesh::XX, partitioned_blocks);
  
  // create a mesh with the blocks only
  CMesh::Ptr block_mesh(new CMesh("block_mesh"));
  create_block_mesh(partitioned_blocks, *block_mesh);
  
  // Write msh for verification in gmsh
//   CMeshWriter::Ptr msh_writer = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
//   boost::filesystem::path outf("PartitionBlocks.msh");
//   msh_writer->write_from_to(block_mesh, outf);
  
  if( PE::instance().rank() == 0)
    std::cout << "-------------- Partitioned blocks ----------------\n" << partitioned_blocks << std::endl;
}

BOOST_AUTO_TEST_CASE( GenerateChannel )
{
  // files should be in current working directory
  boost::filesystem::path dict_path = base_dir / boost::filesystem::path("channel3d.dict");
  CMeshReader::Ptr dict_reader = create_component_abstract_type<CMeshReader>("blockMeshDict","meshreader");
  
  // Read the dict mesh
  CMesh::Ptr dict_mesh(new CMesh("dict_mesh"));
  dict_reader->read_from_to(dict_path, dict_mesh);
  
  CMesh::Ptr gen_mesh(new CMesh("gen_mesh"));
  BlockData blocks;
  Tools::MeshGeneration::create_channel_3d(blocks, 12.5663706144, 0.5, 6.28318530718, 16, 8, 12, 0.1);
  std::vector<Uint> nodes_dist;
  build_mesh(blocks, *gen_mesh, nodes_dist);
  
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *gen_mesh, 500));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

