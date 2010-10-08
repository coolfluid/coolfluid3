// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"
#include "Common/Core.hpp"

#include "Solver/CModel.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CDomain.hpp"
#include "Solver/CIterativeSolver.hpp"
#include "Solver/CDiscretization.hpp"


#include "Solver/ScalarAdvection.hpp"

namespace CF {
namespace Solver {

using namespace Common;
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
  options.add_option< OptionT<std::string> >  ( "Model",  "Model to fill, if empty a new model will be created in the root" , "" );

  options["Model"].as_option().mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::regist_signals ( ScalarAdvection* self )
{
  self->regist_signal ( "run_wizzard" , "runs the wizzard ", "Run Wizzard" )->connect ( boost::bind ( &ScalarAdvection::run_wizzard, self, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::run_wizzard ( Common::XmlNode& node )
{
  // access the CModel
  CModel::Ptr model = look_component_type<CModel>( property("Model").value<std::string>() );

  // set the CDomain
  CDomain::Ptr domain = model->create_component_type<CDomain>("Domain");

  // setup the Physical Model
  CPhysicalModel::Ptr pm = model->create_component_type<CPhysicalModel>("Physics");

  pm->configure_property( "dof", 1 );
  pm->configure_property( "dimensions", 2 );

  // setup
  CDiscretization::Ptr cdm = create_component_abstract_type<CDiscretization>("ResidualDistribution", "Discretization");
  model->add_component( cdm );

  CIterativeSolver::Ptr solver = create_component_abstract_type<CIterativeSolver>("ForwardEuler", "IterativeSolver");
  model->add_component( solver );
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
