// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"

#include "Solver/CModel.hpp"
#include "Solver/CPhysicalModel.hpp"
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
  // signals

  this->regist_signal ( "create_model" , "Creates a scalar advection model", "Create Model" )->connect ( boost::bind ( &ScalarAdvection::create_model, this, _1 ) );
  signal("create_model").signature
      .insert<std::string>("Model name", "Name for created model" );

}

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::~ScalarAdvection()
{
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::create_model ( Common::XmlNode& node )
{
  XmlParams p ( node );

//  // access the CModel
//  CModel::Ptr model = look_component_type<CModel>( property("Model").value<std::string>() );

// create the model

  std::string name  = p.get_option<std::string>("Model name");

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
  model->add_component( solver );

  // setup discretization method
  CDiscretization::Ptr cdm = create_component_abstract_type<CDiscretization>("CF.Solver.ResidualDistribution", "Discretization");
  cdm->mark_basic();
  solver->add_component( cdm );

  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" );
//  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.CGNS.CReader", "CGNSReader" );
  mesh_reader->mark_basic();
  model->add_component( mesh_reader );

}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
