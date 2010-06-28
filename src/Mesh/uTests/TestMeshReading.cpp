#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct MeshReading_Fixture
{
  /// common setup for each test case
  MeshReading_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MeshReading_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshReading_TestSuite, MeshReading_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  BOOST_CHECK_EQUAL(meshreader->name(),"meshreader");
  BOOST_CHECK_EQUAL(meshreader->get_format(),"Neu");

  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  BOOST_CHECK_EQUAL(meshwriter->name(),"meshwriter");
  BOOST_CHECK_EQUAL(meshwriter->get_format(),"Gmsh");

  CMeshWriter::Ptr neu_writer = create_component_abstract_type<CMeshWriter>("Neu","meshwriter");
  BOOST_CHECK_EQUAL(neu_writer->name(),"meshwriter");
  BOOST_CHECK_EQUAL(neu_writer->get_format(),"Neu");
 
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ConvertFromNeuToGmsh )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");

  // UNCOMMENT ALL THIS AND CHANGE THE FILEPATH "fp" TO A VALID PATH
  
  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  CMesh::Ptr mesh ( new CMesh  ( "mesh" ) );
  
  meshreader->read_from_to(fp_in,mesh);
  
  std::string text = (
                      "mesh\n"
                      "  coordinates\n"
                      "  regions\n"
                      "    gas\n"
                      "      P1-Quad2D\n"
                      "        table\n"
                      "        type\n"			
                      "      P1-Triag2D\n"
                      "        table\n"
                      "        type\n"
                      "    inlet\n"
                      "      P1-Line2D\n"
                      "        table\n"
                      "        type\n"
                      "    liquid\n"
                      "      P1-Triag2D\n"
                      "        table\n"
                      "        type\n"
                      "    outlet\n"
                      "      P1-Line2D\n"
                      "        table\n"
                      "        type\n"
                      "    wall\n"
                      "      P1-Line2D\n"
                      "        table\n"
                      "        type\n"
                      );
  // test if tree matches
  BOOST_CHECK_EQUAL(text,mesh->tree());

  boost::filesystem::path fp_out ("quadtriag.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  boost::filesystem::path fp_out_neu ("quadtriag_write.neu");
  CMeshWriter::Ptr neu_writer = create_component_abstract_type<CMeshWriter>("Neu","meshwriter");
  neu_writer->write_from_to(mesh,fp_out_neu);
  BOOST_CHECK_EQUAL(1,1);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ConvertFromNeuToGmsh2 )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");

  // the file to read from and to
  boost::filesystem::path fp_in ("quadtriag_write.neu");
  boost::filesystem::path fp_out("quadtriag_write.msh");

  // the mesh to store in
  CMesh::Ptr mesh ( new CMesh  ( "mesh" ) );

  //CFinfo << "ready to read" << CFendl;
  meshreader->read_from_to(fp_in,mesh);

  std::string text = (
                      "mesh\n"
                      "  coordinates\n"
                      "  regions\n"
                      "    gas\n"
                      "      P1-Quad2D\n"
                      "        table\n"
                      "        type\n"			
                      "      P1-Triag2D\n"
                      "        table\n"
                      "        type\n"
                      "    inlet\n"
                      "      P1-Line2D\n"
                      "        table\n"
                      "        type\n"
                      "    liquid\n"
                      "      P1-Triag2D\n"
                      "        table\n"
                      "        type\n"
                      "    outlet\n"
                      "      P1-Line2D\n"
                      "        table\n"
                      "        type\n"
                      "    wall\n"
                      "      P1-Line2D\n"
                      "        table\n"
                      "        type\n"
                      );
  // test if tree matches
  BOOST_CHECK_EQUAL(text,mesh->tree());
  
  
  //CFinfo << "ready to write" << CFendl;
  meshwriter->write_from_to(mesh,fp_out);
  BOOST_CHECK_EQUAL(1,1);

  CMeshTransformer::Ptr meshinfo = create_component_abstract_type<CMeshTransformer>("Info","meshinfo");
  meshinfo->transform(mesh);

}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

