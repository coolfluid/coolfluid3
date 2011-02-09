// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RDM::ScalarAdvection"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"
#include "Common/CLink.hpp"
#include "Common/Foreach.hpp"

#include "Solver/CIterativeSolver.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CDiscretization.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/Actions/CLoop.hpp"

#include "Mesh/LoadMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CRegion.hpp"

#include "RDM/ScalarAdvection.hpp"
#include "RDM/ResidualDistribution.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::RDM;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ScalarAdvection_steady_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  BOOST_CHECK(true);

  ScalarAdvection::Ptr s = allocate_component<ScalarAdvection>("scalar_advection");

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);
  p.add_option<std::string>("Model name","scalar_advection");

  s->create_model(node);

  BOOST_CHECK(true);

  //--------------------------------------------

  // CFinfo << Core::instance().root()->tree() << CFendl;

  //--------------------------------------------

  boost::shared_ptr<XmlDoc> tree_doc = XmlOps::create_doc();
  XmlNode& tree_node  = *XmlOps::goto_doc_node(*doc.get());

  Core::instance().root()->list_tree(tree_node);

  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_mesh )
{
  
  BOOST_CHECK(true);

  CDomain& domain = find_component_recursively<CDomain>(*Core::instance().root());
    
  // create the xml parameters for the signal

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams xmlp (node);

  BOOST_CHECK(true);

  std::vector<URI> files;

//  files.push_back( "file:rotation-tg.neu" );
    files.push_back( "file:rotation-qd.neu" );
//  files.push_back( "file:advection_p2.msh" );
//  files.push_back( "file:advection-p2-quad.msh" );
//  files.push_back( "file:rotation-tg-p3.msh" );

  xmlp.add_option<URI>("Domain", URI( domain.full_path().string()) );
  xmlp.add_array("Files", files);

  // get the generic mesh loader from the Tools

  LoadMesh::Ptr load_mesh = Core::instance().root()->get_child("Tools")->get_child<LoadMesh>("LoadMesh");
  cf_assert( is_not_null(load_mesh) );
  
  load_mesh->signal_load_mesh( node );

  BOOST_CHECK_NE( domain.get_child_count(), (Uint) 0);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( configuration )
{
  CDomain& domain = find_component_recursively<CDomain>(*Core::instance().root());
  CIterativeSolver& solver = find_component_recursively<CIterativeSolver>(*Core::instance().root());

  solver.configure_property("Domain",URI("cpath:../Domain"));
  solver.configure_property("Number of Iterations", 500u);
  
  CDiscretization::Ptr discretization = solver.get_child<CDiscretization>("Discretization");
  BOOST_CHECK ( is_not_null(discretization) );
  
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  p.add_option<std::string>("Name","apply_inlet");

  discretization->as_type<ResidualDistribution>()->create_bc(node);
  
  CLoop::Ptr apply_inlet = discretization->get_child<CLoop>("apply_inlet");

  std::vector<URI> bc_regions;
  boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(domain,"inlet"))
    bc_regions.push_back( region.full_path() );

  BOOST_CHECK_EQUAL( bc_regions.size() , 1u);

  apply_inlet->configure_property("Regions", bc_regions);

  BOOST_CHECK(true);

  CFinfo << find_component_recursively<CModel>(*Core::instance().root()).tree() << CFendl;
  
  BOOST_CHECK(true);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( solve )
{
  BOOST_CHECK(true);
  CIterativeSolver& solver = find_component_recursively<CIterativeSolver>(*Core::instance().root());
  BOOST_CHECK(true);
  solver.solve();
  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( output )
{
  CDomain& domain = find_component_recursively<CDomain>(*Core::instance().root());
  CMesh::Ptr mesh = domain.get_child<CMesh>("Mesh");
  CMeshWriter::Ptr mesh_writer = create_component_abstract_type<CMeshWriter> ( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  boost::filesystem::path file ("scalar_advection.msh");
  mesh_writer->write_from_to(mesh,file);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

