#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/MeshReader.hpp"
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
  
  boost::shared_ptr<CMeshReader> meshreader ( new CMeshReader  ( "meshreader" ) );
  meshreader->set_reader("Mesh::Neu::Reader");
  
  boost::shared_ptr<CMeshWriter> meshwriter ( new CMeshWriter  ( "meshwriter" ) );
  meshwriter->set_writer("Mesh::Gmsh::Writer");
  
 
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ConvertFromNeuToGmsh )
{
  
  boost::shared_ptr<CMeshReader> meshreader ( new CMeshReader  ( "meshreader" ) );
  meshreader->set_reader("Mesh::Neu::Reader");
  

  // UNCOMMENT ALL THIS AND CHANGE THE FILEPATH "fp" TO A VALID PATH
  
  // the file to read from
  boost::filesystem::path fp_in ("/Users/willem/workspace/coolfluid3/Kernel/src/Mesh/uTests/quadtriag.neu");
  // boost::filesystem::path fp_in ("/Users/willem/workspace/testcases/square_2D_Re10000_FVM_LES/cases/refined.neu");
  // the mesh to store in
  boost::shared_ptr<CMesh> mesh ( new CMesh  ( "mesh" ) );
  
  meshreader->get_reader()->read(fp_in,mesh);
  
  // Output data structure
  XMLNode mesh_node = XMLNode::createXMLTopNode("xml", TRUE);
  mesh_node.addAttribute("version","1.0");
  mesh_node.addAttribute("encoding","UTF-8");
  mesh_node.addAttribute("standalone","yes");
  mesh->xml_tree( mesh_node );
  XMLSTR xml_str = mesh_node.createXMLString();
  CFinfo << "xml_str\n" << xml_str << CFendl;
  freeXMLString(xml_str);
  
  CRegion::Ptr tmp_region = mesh->get_component<CRegion>("regions");
  for (CRegion::iterator region = tmp_region->begin(); region != tmp_region->end(); region++)
  {
    if (region->has_subregions())
    {
      CFinfo << "\n" << region->name() << " \n" << CFendl; 
    }
    else if (region->get_component<CTable>("table")->get_table().size())
    {
      CFinfo << "    " << region->name() << " \t --> elements: " 
             << region->get_component<CTable>("table")->get_table().size() << "\n" << CFendl; 
    }
  }
  
  CFinfo << "There are " << mesh->get_component<CArray>("coordinates")->get_array().size() << " coordinates. \n" << CFendl;
 
 
  boost::filesystem::path fp_out ("/Users/willem/workspace/coolfluid3/Kernel/src/Mesh/uTests/quadtriag.msh");
  // boost::filesystem::path fp_out ("/Users/willem/workspace/testcases/square_2D_Re10000_FVM_LES/cases/refined.msh");
  boost::shared_ptr<CMeshWriter> meshwriter ( new CMeshWriter  ( "meshwriter" ) );
  meshwriter->set_writer("Mesh::Gmsh::Writer");
  meshwriter->get_writer()->write(mesh,fp_out);

 
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

