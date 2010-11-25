// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/CreateComponent.hpp"

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

Common::ComponentBuilder < ScalarAdvection, Component, LibSolver > ScalarAdvection_Builder;

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::ScalarAdvection ( const std::string& name  ) :
  Component ( name )
{
  BuildComponent<full>().build(this);
}

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::~ScalarAdvection()
{
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::define_config_properties()
{
//  m_properties.add_option< OptionT<std::string> >  ( "Model",  "Model to fill, if empty a new model will be created in the root" , "" );
//  m_properties["Model"].as_option().mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::define_signals ()
{
  this->regist_signal ( "run_wizard" , "runs the wizard ", "Run Wizard" )->connect ( boost::bind ( &ScalarAdvection::run_wizard, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::run_wizard ( Common::XmlNode& node )
{
//  // access the CModel
//  CModel::Ptr model = look_component_type<CModel>( property("Model").value<std::string>() );

  CModel::Ptr model = Core::instance().root()->create_component_type<CModel>("CF.Solver.ScalarAdvection");
  model->mark_basic();

  // create the CDomain
  CDomain::Ptr domain = model->create_component_type<CDomain>("Domain");
  domain->mark_basic();

  // create the Physical Model
  CPhysicalModel::Ptr pm = model->create_component_type<CPhysicalModel>("Physics");
  pm->mark_basic();

  pm->configure_property( "DOFs", 1u );
  pm->configure_property( "Dimensions", 2u );

  // setup discretization method
  CDiscretization::Ptr cdm = create_component_abstract_type<CDiscretization>("CF.Solver.ResidualDistribution", "Discretization");
  cdm->mark_basic();
  model->add_component( cdm );

  // setup iterative solver
  CIterativeSolver::Ptr solver = create_component_abstract_type<CIterativeSolver>("CF.Solver.ForwardEuler", "IterativeSolver");
  solver->mark_basic();
  model->add_component( solver );

  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" );
//  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.CGNS.CReader", "CGNSReader" );
  mesh_reader->mark_basic();
  model->add_component( mesh_reader );

}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
