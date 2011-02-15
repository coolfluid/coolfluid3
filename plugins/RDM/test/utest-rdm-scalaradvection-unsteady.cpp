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

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/LoadMesh.hpp"

#include "RDM/ScalarAdvection.hpp"
#include "RDM/ResidualDistribution.hpp"


using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::RDM;

struct scalar_advection_global_fixture
{
  scalar_advection_global_fixture()
  {
    scalar_advection_wizard = allocate_component<ScalarAdvection>("scalar_advection");

    boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
    XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
    XmlParams p(node);
    p.add_option<std::string>("Model name","scalar_advection");

    scalar_advection_wizard->create_model(node);
  }

  ScalarAdvection::Ptr scalar_advection_wizard;

};

struct scalar_advection_local_fixture
{
  scalar_advection_local_fixture() :
    model  ( * Core::instance().root()->get_child("scalar_advection")->as_type<CModel>() ),
    domain ( find_component_recursively<CDomain>(model)  ),
    solver ( find_component_recursively<CIterativeSolver>(model) ),
    discretization( find_component_recursively<CDiscretization>(solver) )
	{}
	
  CModel& model;
  CDomain& domain;
  CIterativeSolver& solver;
  CDiscretization& discretization;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( scalar_advection_global_fixture )

BOOST_AUTO_TEST_SUITE( scalar_advection_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( check_tree , scalar_advection_local_fixture )
{
  BOOST_CHECK(true);

  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& tree_node  = *XmlOps::goto_doc_node(*doc.get());

  Core::instance().root()->list_tree(tree_node);

  CFinfo << Core::instance().root()->tree() << CFendl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

