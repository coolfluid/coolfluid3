// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/CGroup.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/Gmsh/CWriter.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CIterativeSolver.hpp"
#include "Solver/CDiscretization.hpp"


#include "FVM/ShockTube.hpp"

namespace CF {
namespace FVM {

using namespace boost::assign;
using namespace CF::Common;
using namespace CF::Common::String;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;

Common::ComponentBuilder < ShockTube, Component, LibFVM > ShockTube_Builder;

////////////////////////////////////////////////////////////////////////////////

ShockTube::ShockTube ( const std::string& name  ) :
  Component ( name )
{
  std::string brief;
  std::string description;
  brief       += "This wizard creates and sets up a finite volume 1D shocktube problem.\n";
  description += "  1) Run signal \"Create Model\" to create the shocktube model\n";
  description += "  2) Load a 1D mesh in the domain of the shocktube model";
  description += "  3) Run signal \"Setup Model\" to configure and allocate datastorage\n";
  description += "  4) Configure time step and end time in model/Time\n";
  description += "  5) Run signal \"Simulate\" in the shocktube model\n";
  m_properties["brief"] = brief;
  m_properties["description"] = description;

  // signals

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;

  regist_signal ( "create_model" , "Creates a shocktube model", "Create Model" )->connect ( boost::bind ( &ShockTube::signal_create_model, this, _1 ) );
  signal("create_model").signature->connect( boost::bind( &ShockTube::signature_create_model, this, _1));

  regist_signal ( "setup_model" , "Setup the shocktube model after mesh has been loaded", "Setup Model" )->connect ( boost::bind ( &ShockTube::signal_setup_model, this, _1 ) );
  signal("setup_model").signature->connect( boost::bind( &ShockTube::signature_setup_model, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

ShockTube::~ShockTube()
{
}

////////////////////////////////////////////////////////////////////////////////

void ShockTube::signal_create_model ( Common::XmlNode& node )
{
  XmlParams p ( node );

// create the model

  std::string name  = p.get_option<std::string>("Model name");

  CModel::Ptr model = Core::instance().root()->create_component<CModelUnsteady>( name );

  // create the CDomain
  // CDomain::Ptr domain =
  model->create_component<CDomain>("domain");

  // create the Physical Model
  CPhysicalModel::Ptr pm = model->create_component<CPhysicalModel>("Physics");
  pm->mark_basic();

  pm->configure_property( "DOFs", 1u );
  pm->configure_property( "Dimensions", 1u );

  // setup iterative solver
  CIterativeSolver::Ptr solver = create_component_abstract_type<CIterativeSolver>("CF.FVM.ForwardEuler", "IterativeSolver");
  solver->mark_basic();
  model->add_component( solver );

  // setup discretization method
  CDiscretization::Ptr cdm = create_component_abstract_type<CDiscretization>("CF.FVM.FiniteVolume", "Discretization");
  cdm->mark_basic();
  solver->add_component( cdm );
  
  CGroup& tools = *model->create_component<CGroup>("tools");
  
  CBuildFaces& build_faces = *tools.create_component<CBuildFaces>("build_faces");
  Gmsh::CWriter& gmsh_writer = *tools.create_component<Gmsh::CWriter>("gmsh_writer");
}

////////////////////////////////////////////////////////////////////////////////

void ShockTube::signature_create_model( XmlNode& node )
{
  XmlParams p(node);

  p.add_option<std::string>("Model name", std::string(), "Name for created model" );
}

////////////////////////////////////////////////////////////////////////////////

void ShockTube::signal_setup_model ( Common::XmlNode& node )
{
  XmlParams p ( node );
  std::string name  = p.get_option<std::string>("Model name");

  CModelUnsteady::Ptr model = Core::instance().root()->get_child<CModelUnsteady>( name );
  if (is_null(model))
    throw ValueNotFound (FromHere(), "invalid model");
  // configure the solution field etc.
  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>(*model);
  if (is_null(mesh))
    throw ValueNotFound (FromHere(), "Mesh is not found in the model");
  
  std::vector<std::string> args;  
  model->look_component<CBuildFaces>("cpath:./tools/build_faces")->transform(mesh,args);
  
  std::vector<std::string> solution_vars = list_of("rho[1]")("rhoU[1]")("rhoE[1]");
  CField2& solution = mesh->create_field2("solution",mesh->topology(),solution_vars,"ElementBased");

  std::vector<std::string> residual_vars = list_of("delta_rho[1]")("delta_rhoU[1]")("delta_rhoE[1]");
  CField2& residual = mesh->create_field2("residual",mesh->topology(),residual_vars,"ElementBased");

  std::vector<std::string> update_coeff_vars = list_of("uc_rho[1]")("uc_rhoU[1]")("uc_rhoE[1]");
  CField2& update_coeff = mesh->create_field2("update_coeff",mesh->topology(),update_coeff_vars,"ElementBased");

  CIterativeSolver& solver = find_component<CIterativeSolver>(*model);  
  solver.configure_property("Domain" , model->get_child<CDomain>("domain")->full_path() );
  solver.configure_property("Number of Iterations", 1u);
  
  std::vector<URI> fields;
  boost_foreach(const CField2& field, find_components_recursively<CField2>(*mesh))
    fields.push_back(field.full_path());
  model->look_component<Gmsh::CWriter>("cpath:./tools/gmsh_writer")->configure_property("Fields",fields);
}

////////////////////////////////////////////////////////////////////////////////

void ShockTube::signature_setup_model( XmlNode& node )
{
  XmlParams p(node);

  p.add_option<std::string>("Model name", std::string(), "Name for created model" );
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
