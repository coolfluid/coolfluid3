// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/CreateComponent.hpp"

#include "Solver/CEigenLSS.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"

#include "Solver/CModelSteady.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CIterativeSolver.hpp"
#include "Solver/CDiscretization.hpp"

#include "HeatConductionLinearSteady.hpp"
#include "SetupHeatConductionLinearSteady.hpp"

namespace CF {
namespace UFEM {

using namespace Actions;
using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Actions::Proto;

CF::Common::ComponentBuilder < UFEM::SetupHeatConductionLinearSteady, Component, LibUFEM > aSetupHeatConductionLinearSteady_Builder;

SetupHeatConductionLinearSteady::SetupHeatConductionLinearSteady(const std::string& name) : Component ( name )
{
  this->regist_signal ( "create_model" , "Creates a linear, steady heat conduction model", "Create Model" )->connect( boost::bind ( &SetupHeatConductionLinearSteady::create_model, this, _1 ) );
  signal("create_model").signature.insert<std::string>("Model name", "Name for created model" );

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;
}

void SetupHeatConductionLinearSteady::create_model(Common::XmlNode& node)
{
  XmlParams p ( node );

  // create the model
  std::string name  = p.get_option<std::string>("Model name");

  CModel::Ptr model = Core::instance().root()->create_component<CModelSteady>( name );

  // create the CDomain
  // CDomain::Ptr domain =
  model->create_component<CDomain>("Domain");

  // The linear system solver
  CEigenLSS::Ptr lss = model->create_component<CEigenLSS>("LSS");
  lss->mark_basic();

  // Setup heat conduction method
  HeatConductionLinearSteady::Ptr hc = model->create_component<HeatConductionLinearSteady>("HeatConductionLinearSteady");
  hc->create_component<CPhysicalModel>("Physics")->mark_basic();
  hc->initialize();
  hc->mark_basic();
  hc->get_child("HeatEquation")->configure_property( "LSS", URI(lss->full_path().string()) );

  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" );
  mesh_reader->mark_basic();
  model->add_component( mesh_reader );
}



} // UFEM
} // CF
