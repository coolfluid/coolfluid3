
// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh reading"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Common/FindComponents.hpp"


#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct MeshTransformer_Fixture
{
  /// common setup for each test case
  MeshTransformer_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

    root = CRoot::create("Root");
    reader = build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","MyReader");
    domain = root->create_component_ptr<CDomain>("MyDom");

    root->add_component( reader );
    
    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionDumps = false;
  }

  /// common tear-down for each test case
  ~MeshTransformer_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  CRoot::Ptr root;
  CMeshReader::Ptr reader;
  CDomain::Ptr domain;

  static CMesh::Ptr mesh;
  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

CMesh::Ptr MeshTransformer_Fixture::mesh;

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshTransformer_TestSuite, MeshTransformer_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_mesh )
{
  // CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  // 
  // // the file to read from
  // boost::filesystem::path fp_in ("quadtriag.neu");
  // 
  // // the mesh to store in
  // 
  // meshreader->do_read_mesh_into(fp_in,mesh);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_arguments )
{
  CMeshTransformer::Ptr transformer = build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CInfo","info");
  
  std::vector<std::string> args;
  args.push_back("variable:bool=true");
  args.push_back("array_vars:array<bool>=true,false,true");
  //args.push_back("center:double=[x,y,z]");
  
  //transformer->configure(args);
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

