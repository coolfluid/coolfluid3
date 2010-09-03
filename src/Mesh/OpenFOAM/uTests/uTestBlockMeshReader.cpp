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

#include "Tools/MeshDiff/MeshDiff.hpp"

using namespace CF::Common;
using namespace CF::Mesh;

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

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

