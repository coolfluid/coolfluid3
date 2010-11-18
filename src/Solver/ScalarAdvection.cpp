// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshReader.hpp"

#include "Solver/CModel.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CDomain.hpp"
#include "Solver/CIterativeSolver.hpp"
#include "Solver/CDiscretization.hpp"


#include "Solver/ScalarAdvection.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;
using namespace Common::String;

Common::ObjectProvider < ScalarAdvection, Component, LibSolver, NB_ARGS_1 >
ScalarAdvection_Provider ( ScalarAdvection::type_name() );

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::ScalarAdvection ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::~ScalarAdvection()
{
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::defineConfigProperties(Common::PropertyList& options)
{
//  options.add_option< OptionT<std::string> >  ( "Model",  "Model to fill, if empty a new model will be created in the root" , "" );
//  options["Model"].as_option().mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::regist_signals ( ScalarAdvection* self )
{
  self->regist_signal ( "run_wizard" , "runs the wizard ", "Run Wizard" )->connect ( boost::bind ( &ScalarAdvection::run_wizard, self, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::run_wizard ( Common::XmlNode& node )
{
//  // access the CModel
//  CModel::Ptr model = look_component_type<CModel>( property("Model").value<std::string>() );

  CModel::Ptr model =
    Core::instance().root()->create_component_type<CModel>("ScalarAdvection");

  // set the CDomain
  // CDomain::Ptr domain =
      model->create_component_type<CDomain>("Domain")->mark_basic();;

  // setup the Physical Model
  CPhysicalModel::Ptr pm = model->create_component_type<CPhysicalModel>("Physics");

  pm->configure_property( "DOFs", 1u );
  pm->configure_property( "Dimensions", 2u );

  // setup discretization method
  CDiscretization::Ptr cdm = create_component_abstract_type<CDiscretization>("ResidualDistribution", "Discretization");
  model->add_component( cdm );

  // setup iterative solver
  CIterativeSolver::Ptr solver = create_component_abstract_type<CIterativeSolver>("ForwardEuler", "IterativeSolver");
  model->add_component( solver );

//  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "Neu", "NeutralReader" );
//  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CGNS", "CGNSReader" );
//  mesh_reader->mark_basic();
//  model->add_component( mesh_reader );

  // mark the new components as basic
  model->mark_basic();
  pm->mark_basic();
  cdm->mark_basic();
  solver->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
