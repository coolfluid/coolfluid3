#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/MeshWriter.hpp"

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
  
  /// These are handy functions that should maybe be implemented somewhere easily accessible.
  
  /// create a Real vector with 2 coordinates
  RealVector create_coord(const Real& x, const Real& y) {
    RealVector coordVec(2);
    coordVec[XX]=x;
    coordVec[YY]=y;
    return coordVec;
  }
  
  /// create a Uint vector with 4 node ID's
  std::vector<Uint> create_quad(const Uint& A, const Uint& B, const Uint& C, const Uint& D) {
    Uint quad[] = {A,B,C,D};
    std::vector<Uint> quadVec;
    quadVec.assign(quad,quad+4);
    return quadVec;
  }
  
  /// create a Uint vector with 3 node ID's
  std::vector<Uint> create_triag(const Uint& A, const Uint& B, const Uint& C) {
    Uint triag[] = {A,B,C};
    std::vector<Uint> triagVec;
    triagVec.assign(triag,triag+3);
    return triagVec;
  }
  
  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshReading_TestSuite, MeshReading_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  boost::shared_ptr<CMeshReader> meshreader = CMeshReader::create_concrete("Neu","meshreader");
  BOOST_CHECK_EQUAL(meshreader->name(),"meshreader");
  BOOST_CHECK_EQUAL(meshreader->get_format(),"Neu");


  boost::shared_ptr<CMeshWriter> meshwriter ( new CMeshWriter  ( "meshwriter" ) );
  meshwriter->set_writer("Mesh::Gmsh::Writer");

 
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ConvertFromNeuToGmsh )
{
  boost::shared_ptr<CMeshReader> meshreader = CMeshReader::create_concrete("Neu","meshreader");

  // UNCOMMENT ALL THIS AND CHANGE THE FILEPATH "fp" TO A VALID PATH
  
  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  boost::shared_ptr<CMesh> mesh ( new CMesh  ( "mesh" ) );
  
  meshreader->read_from_to(fp_in,mesh);
  
  // Output data structure
//  XMLNode mesh_node = XMLNode::createXMLTopNode("xml", TRUE);
//  mesh_node.addAttribute("version","1.0");
//  mesh_node.addAttribute("encoding","UTF-8");
//  mesh_node.addAttribute("standalone","yes");
//  mesh->xml_tree( mesh_node );
//  XMLSTR xml_str = mesh_node.createXMLString();
//  CFinfo << "xml_str\n" << xml_str << CFflush;
//  freeXMLString(xml_str);
  
  CRegion::Ptr tmp_region = mesh->get_component<CRegion>("regions");
  BOOST_FOREACH(const CRegion::Ptr& region, iterate_recursive_by_type<CRegion>(tmp_region))
  {
    if (region->has_component_of_type<CRegion>())
    {
      CFinfo << "\n" << region.name() << " \n" << CFflush;
    }
    else if (region->get_component<CTable>("table")->get_table().size())
    {
      CFinfo << "    " << region.name() << " \t --> elements: "
             << region.get_component<CTable>("table")->get_table().size() << "\n" << CFflush;
    }
  }
  
  CFinfo << "There are " << mesh->get_component<CArray>("coordinates")->get_array().size() << " coordinates. \n" << CFflush;
 
 
  boost::filesystem::path fp_out ("quadtriag.msh");
  boost::shared_ptr<CMeshWriter> meshwriter ( new CMeshWriter  ( "meshwriter" ) );
  meshwriter->set_writer("Mesh::Gmsh::Writer");
  meshwriter->get_writer()->write(mesh,fp_out);
 
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

