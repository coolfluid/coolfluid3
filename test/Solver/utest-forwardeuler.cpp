// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Solver::ForwardEuler"

#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"
#include "Common/CLink.hpp"

#include "Solver/CIterativeSolver.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CDiscretization.hpp"
#include "Solver/CPhysicalModel.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Solver;
using namespace CF::Mesh;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ForwardEulerSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  CIterativeSolver::Ptr comp = create_component_abstract_type<CIterativeSolver>("CF.Solver.ForwardEuler", "ForwardEuler");
  BOOST_CHECK( is_not_null(comp) );

  BOOST_CHECK( find_component_with_name<CLink>(*comp, "solution_field").is_link() == true );  
  BOOST_CHECK( find_component_with_name<CLink>(*comp, "solution_field").is_linked() == false );
  BOOST_CHECK( is_null(find_component_with_name<CLink>(*comp, "solution_field").get()) );

  BOOST_CHECK( find_component_with_name<CLink>(*comp, "residual_field").is_link() == true );  
  BOOST_CHECK( find_component_with_name<CLink>(*comp, "residual_field").is_linked() == false );
  BOOST_CHECK( is_null(find_component_with_name<CLink>(*comp, "residual_field").get()) );



  std::string name  = "scalar_advection";

    CModel::Ptr model = Core::instance().root()->create_component_type<CModel>( name );

    // create the CDomain
    // CDomain::Ptr domain =
        model->create_component_type<CDomain>("Domain");

    // create the Physical Model
    CPhysicalModel::Ptr pm = model->create_component_type<CPhysicalModel>("Physics");
    pm->mark_basic();

    pm->configure_property( "DOFs", 1u );
    pm->configure_property( "Dimensions", 2u );

    // setup iterative solver
    CIterativeSolver::Ptr solver = create_component_abstract_type<CIterativeSolver>("CF.Solver.ForwardEuler", "IterativeSolver");
    solver->mark_basic();
    model->add_component( solver );

    // setup discretization method
    CDiscretization::Ptr cdm = create_component_abstract_type<CDiscretization>("CF.Solver.ResidualDistribution", "Discretization");
    cdm->mark_basic();
    solver->add_component( cdm );

    CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" );
  //  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.CGNS.CReader", "CGNSReader" );
    mesh_reader->mark_basic();
    model->add_component( mesh_reader );

    CFinfo << model->tree() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

