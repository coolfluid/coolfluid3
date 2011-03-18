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
#include "Common/Signal.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/Gmsh/CWriter.hpp"
#include "Mesh/Gmsh/CReader.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/Actions/CBuildVolume.hpp"
#include "Mesh/Actions/CInitFieldFunction.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CRegion.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/Actions/CForAllElements.hpp"
#include "Solver/Actions/CForAllFaces.hpp"
#include "Solver/Actions/CLoop.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "FVM/FiniteVolumeSolver2D.hpp"
#include "FVM/ShockTube2D.hpp"

namespace CF {
namespace FVM {

using namespace boost::assign;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

Common::ComponentBuilder < ShockTube2D, CWizard, LibFVM > ShockTube2D_Builder;

////////////////////////////////////////////////////////////////////////////////

ShockTube2D::ShockTube2D ( const std::string& name  ) :
  CWizard ( name )
{
  add_tag(LibFVM::library_namespace());
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

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  regist_signal ( "create_model" , "Creates a shocktube model", "Create Model" )->signal->connect ( boost::bind ( &ShockTube2D::signal_create_model, this, _1 ) );
  signal("create_model")->signature->connect( boost::bind( &ShockTube2D::signature_create_model, this, _1));

  regist_signal ( "setup_model" , "Setup the shocktube model after mesh has been loaded", "Setup Model" )->signal->connect ( boost::bind ( &ShockTube2D::signal_setup_model, this, _1 ) );
  signal("setup_model")->signature->connect( boost::bind( &ShockTube2D::signature_setup_model, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

ShockTube2D::~ShockTube2D()
{
}

////////////////////////////////////////////////////////////////////////////////

void ShockTube2D::signal_create_model ( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

// create the model

  std::string name  = p.get_option<std::string>("model_name");

  CModel::Ptr model = Core::instance().root()->create_component<CModelUnsteady>( name );

  // create the Physical Model
  CPhysicalModel::Ptr pm = model->create_component<CPhysicalModel>("Physics");
  pm->mark_basic();

  pm->configure_property( "DOFs", 1u );
  pm->configure_property( "Dimensions", 2u );

  // setup iterative solver
  CSolver::Ptr solver = create_component_abstract_type<CSolver>("CF.FVM.FiniteVolumeSolver2D", "FiniteVolumeSolver2D");
  solver->mark_basic();
  model->add_component( solver );

  model->create_domain("domain");
  
  CGroup& tools = *model->create_component<CGroup>("tools");
  tools.mark_basic();
  tools.create_component<CBuildFaces>("build_faces");
  tools.create_component<CBuildVolume>("build_volume");

  create_component<Gmsh::CReader>("gmsh_reader");
  create_component<Gmsh::CWriter>("gmsh_writer");

}

////////////////////////////////////////////////////////////////////////////////

void ShockTube2D::signature_create_model( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("model_name", std::string(), "Name for created model" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShockTube2D::signal_setup_model ( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );
  std::string name  = p.get_option<std::string>("model_name");

  CModelUnsteady::Ptr model = Core::instance().root()->get_child_ptr( name )->as_ptr<CModelUnsteady>();
  if (is_null(model))
    throw ValueNotFound (FromHere(), "invalid model");

  FiniteVolumeSolver2D& solver = find_component<FiniteVolumeSolver2D>(*model);

  ////////////////////////////////////////////////////////////////////////////////
  // Generate mesh
  ////////////////////////////////////////////////////////////////////////////////
  
  CMesh::Ptr mesh = model->domain()->create_component<CMesh>("line");
  Tools::MeshGeneration::create_rectangle(*mesh, 10., 10., p.get_option<Uint>("nb_cells"), p.get_option<Uint>("nb_cells"));
  
  // path file_in("line.msh");
  //   model->access_component_ptr<CMeshReader>("cpath:./tools/gmsh_reader")->read_from_to(file_in,mesh);

  model->access_component_ptr("cpath:./tools/build_faces")->as_ptr<CBuildFaces>()->transform(mesh);
  model->access_component_ptr("cpath:./tools/build_volume")->as_ptr<CBuildVolume>()->transform(mesh);
  model->configure_option_recursively("volume",find_component_recursively_with_tag<CField2>(*model->domain(),"volume").full_path());

  ////////////////////////////////////////////////////////////////////////////////
  // Solver / Discretization configuration
  ////////////////////////////////////////////////////////////////////////////////
  
  model->time().configure_property("time_step", p.get_option<Real>("time_step"));
  model->time().configure_property("end_time", p.get_option<Real>("end_time"));  
  
  model->configure_option_recursively("time_accurate",true);
  model->configure_option_recursively("time",model->time().full_path());
  model->configure_option_recursively("cfl",1.);

  // This will trigger setup of everything correctly
  solver.configure_property("Domain", find_component<CDomain>(*model).full_path() );

  ////////////////////////////////////////////////////////////////////////////////
  // Initial condition
  ////////////////////////////////////////////////////////////////////////////////
  
  CInitFieldFunction::Ptr init_solution = model->get_child_ptr("tools")->create_component<CInitFieldFunction>("init_solution");
  init_solution->configure_property("Field",find_component_with_tag(*mesh,"solution").full_path());
  
  RealVector left(4);
  RealVector right(4);
  
  Real g=1.4;
    
  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real v_L = 0.;        const Real v_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;
  
  left <<  r_L, r_L*u_L, r_L*v_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L);
  right << r_R, r_R*u_R, r_R*v_R, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R);
  
  std::vector<std::string> function(4);
  function[0]="if( (x<=5)&(y<=5),"+to_str(left[0])+","+to_str(right[0])+")";
  function[1]="if( (x<=5)&(y<=5),"+to_str(left[1])+","+to_str(right[1])+")";
  function[2]="if( (x<=5)&(y<=5),"+to_str(left[2])+","+to_str(right[2])+")";
  function[3]="if( (x<=5)&(y<=5),"+to_str(left[3])+","+to_str(right[3])+")";
  
  init_solution->configure_property("Functions",function);
  init_solution->transform(mesh);
  
  ////////////////////////////////////////////////////////////////////////////////
  // Boundary conditions
  ////////////////////////////////////////////////////////////////////////////////
  
  CRegion& left_reg = find_component_recursively_with_name<CRegion>(mesh->topology(),"left");
  solver.create_bc("left",left_reg,"CF.FVM.BCReflectCons2D");

  CRegion& right_reg = find_component_recursively_with_name<CRegion>(mesh->topology(),"right");
  solver.create_bc("right",right_reg,"CF.FVM.BCReflectCons2D");

  CRegion& top_reg = find_component_recursively_with_name<CRegion>(mesh->topology(),"top");
  solver.create_bc("top",top_reg,"CF.FVM.BCReflectCons2D");

  CRegion& bottom_reg = find_component_recursively_with_name<CRegion>(mesh->topology(),"bottom");
  solver.create_bc("bottom",bottom_reg,"CF.FVM.BCReflectCons2D");

  solver.configure_option_recursively("face_normal", find_component_with_tag<CField2>(*mesh,"face_normal").full_path() );

  ////////////////////////////////////////////////////////////////////////////////
  // Writer
  ////////////////////////////////////////////////////////////////////////////////

  std::vector<URI> fields;
  boost_foreach(const CField2& field, find_components_recursively<CField2>(*mesh))
    fields.push_back(field.full_path());
  model->access_component_ptr("cpath:./tools/gmsh_writer")->configure_property("Fields",fields);
  model->access_component_ptr("cpath:./tools/gmsh_writer")->configure_property("File",model->name()+".msh");
  model->access_component_ptr("cpath:./tools/gmsh_writer")->configure_property("Mesh",mesh->full_path());

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShockTube2D::signature_setup_model( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("model_name", std::string(), "Name for created model" );
  p.set_option<Uint>("nb_cells", 100u , "Number of Cells to be generated");
  p.set_option<Real>("end_time", 0.008, "Time to stop simulation");
  p.set_option<Real>("time_step", 0.0004, "Maximum allowed time step to take");
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
