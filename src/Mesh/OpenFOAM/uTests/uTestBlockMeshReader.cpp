// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::OpenFOAM::BlockMeshReader"

#include <boost/assign/list_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/OpenFOAM/BlockData.hpp"
#include "Mesh/OpenFOAM/Parser.hpp"
#include "Mesh/OpenFOAM/SimpleCommunicationPattern.hpp"
#include "Mesh/OpenFOAM/WriteDict.hpp"

#include "Tools/MeshDiff/MeshDiff.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::OpenFOAM;

//////////////////////////////////////////////////////////////////////////////

struct BlockMeshReader_Fixture
{
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BlockMeshReader, BlockMeshReader_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Channel3D )
{
  // files should be in current working directory
  boost::filesystem::path dict_path = boost::filesystem::path("channel3d.dict");
  CMeshReader::Ptr dict_reader = create_component_abstract_type<CMeshReader>("blockMeshDict","meshreader");
  
  // Read the dict mesh
  CMesh::Ptr dict_mesh(new CMesh("dict_mesh"));
  dict_reader->read_from_to(dict_path, dict_mesh);
  
  // Read the reference mesh
  boost::filesystem::path ref_path ("uTestBlockMeshReader-Channel3D-reference.neu");
  CMeshReader::Ptr ref_reader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  CMesh::Ptr ref_mesh(new CMesh("reference"));
  ref_reader->read_from_to(ref_path, ref_mesh);
  
  // Check if they are equal
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *ref_mesh, 25000));
}

BOOST_AUTO_TEST_CASE( Cavity2D )
{
  // files should be in current working directory
  boost::filesystem::path dict_path = boost::filesystem::path("cavity2d.dict");
  CMeshReader::Ptr dict_reader = create_component_abstract_type<CMeshReader>("blockMeshDict","meshreader");
  
  // Read the dict mesh
  CMesh::Ptr dict_mesh(new CMesh("dict_mesh"));
  dict_reader->read_from_to(dict_path, dict_mesh);
  
    // Read the reference mesh
  boost::filesystem::path ref_path ("uTestBlockMeshReader-Cavity2D-reference.neu");
  CMeshReader::Ptr ref_reader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  CMesh::Ptr ref_mesh(new CMesh("reference"));
  ref_reader->read_from_to(ref_path, ref_mesh);
  
  // Check if they are equal
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *ref_mesh, 100));
}

BOOST_AUTO_TEST_CASE( PitzDaily )
{
  // files should be in current working directory
  boost::filesystem::path dict_path = boost::filesystem::path("pitzdaily.dict");
  CMeshReader::Ptr dict_reader = create_component_abstract_type<CMeshReader>("blockMeshDict","meshreader");
  
  // Read the dict mesh
  CMesh::Ptr dict_mesh(new CMesh("dict_mesh"));
  dict_reader->read_from_to(dict_path, dict_mesh);
  
    // Read the reference mesh
  boost::filesystem::path ref_path ("uTestBlockMeshReader-PitzDaily-reference.neu");
  CMeshReader::Ptr ref_reader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  CMesh::Ptr ref_mesh(new CMesh("reference"));
  ref_reader->read_from_to(ref_path, ref_mesh);
  
  // Check if they are equal
  BOOST_CHECK(CF::Tools::MeshDiff::diff(*dict_mesh, *ref_mesh, 50000));
}

BOOST_AUTO_TEST_CASE( WriteDict )
{
  boost::filesystem::path path = boost::filesystem::path("pitzdaily.dict");
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
  if(!PEInterface::instance().is_init())
    PEInterface::instance().init(0,0);
  
  boost::filesystem::path path = boost::filesystem::path("channel3d.dict");
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
  partition_blocks(block_data, nb_procs, CF::XX, partitioned_blocks);
  
  // create a mesh with the blocks only
  CMesh::Ptr block_mesh(new CMesh("block_mesh"));
  create_block_mesh(partitioned_blocks, *block_mesh);
  
  // Write msh for verification in gmsh
  CMeshWriter::Ptr msh_writer = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  boost::filesystem::path outf("PartitionBlocks.msh");
  msh_writer->write_from_to(block_mesh, outf);
  
  if( PEInterface::instance().rank() == 0)
    std::cout << "-------------- Partitioned blocks ----------------\n" << partitioned_blocks << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

