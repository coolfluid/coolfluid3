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
#include "Common/Log.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshWriter.hpp"
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

#include "FVM/FiniteVolumeSolver.hpp"
#include "FVM/ShockTube.hpp"
#include "FVM/BuildGhostStates.hpp"

namespace CF {
namespace FVM {

using namespace boost::assign;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

Common::ComponentBuilder < ShockTube, CWizard, LibFVM > ShockTube_Builder;

////////////////////////////////////////////////////////////////////////////////

ShockTube::ShockTube ( const std::string& name  ) :
  CWizard ( name )
{
  add_tag(LibFVM::library_namespace());
  std::string brief;
  std::string description;
  brief       += "This wizard creates and sets up a finite volume shocktube problem.\n";
  description += "  1) Run signal \"Create Model\" to create the shocktube model\n";
  description += "  2) Configure time step and end time in model/Time\n";
  description += "  3) Run signal \"Simulate\" in the shocktube model\n";
  m_properties["brief"] = brief;
  m_properties["description"] = description;

  // signals

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  regist_signal ( "create_model" , "Creates a shocktube model", "Create Model" )->signal->connect ( boost::bind ( &ShockTube::signal_create_model, this, _1 ) );
  signal("create_model")->signature->connect( boost::bind( &ShockTube::signature_create_model, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

ShockTube::~ShockTube()
{
}

////////////////////////////////////////////////////////////////////////////////

void ShockTube::signal_create_model ( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  ////////////////////////////////////////////////////////////////////////////////
  // Create Model
  ////////////////////////////////////////////////////////////////////////////////

  std::string name  = p.get_option<std::string>("model_name");
  CFinfo << "Creating model " << name << CFendl;
  CModelUnsteady& model = *Core::instance().root()->create_component<CModelUnsteady>( name );

  ////////////////////////////////////////////////////////////////////////////////
  // Create Physics
  ////////////////////////////////////////////////////////////////////////////////
  
  CFinfo << "Creating physics" << CFendl;
  CPhysicalModel& physics = *model.create_component<CPhysicalModel>("Physics");
  physics.mark_basic();
  physics.configure_property( "Dimensions", p.get_option<Uint>("dimension") );

  ////////////////////////////////////////////////////////////////////////////////
  // Create tools
  ////////////////////////////////////////////////////////////////////////////////

  CGroup& tools = *model.create_component<CGroup>("tools");
  CMeshTransformer& finite_volume_transformer = *tools.create_component<CMeshTransformer>("FiniteVolumeTransformer");
  finite_volume_transformer.create_component<CBuildFaces>("1_build_faces");
  finite_volume_transformer.create_component<BuildGhostStates>("2_build_ghoststates");
  finite_volume_transformer.create_component<CBuildVolume>("3_build_volume_field");
  
  ////////////////////////////////////////////////////////////////////////////////
  // Generate mesh
  ////////////////////////////////////////////////////////////////////////////////
  
  CFinfo << "Creating domain" << CFendl;
  CDomain& domain = model.create_domain("domain");
  CMesh::Ptr mesh_ptr;
  CFinfo << "  Generating mesh with " << p.get_option<Uint>("nb_cells") << " cells per dimension" <<CFendl;
  switch (p.get_option<Uint>("dimension"))
  {
    case 1:
      mesh_ptr = domain.create_component<CMesh>("line");
      Tools::MeshGeneration::create_line(*mesh_ptr, 10. , p.get_option<Uint>("nb_cells"));
      break;
    case 2:
      mesh_ptr = domain.create_component<CMesh>("square");
      Tools::MeshGeneration::create_rectangle(*mesh_ptr, 10. , 10. , p.get_option<Uint>("nb_cells"),  p.get_option<Uint>("nb_cells"));
      break;
    default:
      throw NotSupported(FromHere(),"Only 1D or 2D dimension supported");
  }
  CMesh& mesh = *mesh_ptr;
  CFinfo << "  Transforming mesh for finite volume: " << finite_volume_transformer.tree();
  finite_volume_transformer.transform(mesh);
  
  ////////////////////////////////////////////////////////////////////////////////
  // Create Solver / Discretization
  ////////////////////////////////////////////////////////////////////////////////
  
  CFinfo << "Creating FiniteVolumeSolver" << CFendl;
  FiniteVolumeSolver& solver = *model.create_component<FiniteVolumeSolver>("FiniteVolumeSolver");
  solver.configure_property("physical_model",physics.full_path());
  solver.configure_property("Domain",domain.full_path());
  solver.configure_option_recursively("time",model.time().full_path());
  solver.configure_option_recursively("time_accurate",true);

  ////////////////////////////////////////////////////////////////////////////////
  // Initial condition
  ////////////////////////////////////////////////////////////////////////////////
  
  CFinfo << "Setting initial condition" << CFendl;
  CInitFieldFunction& init_solution = *tools.create_component<CInitFieldFunction>("init_solution");
  init_solution.configure_property("Field",find_component_with_tag(mesh,"solution").full_path());

  const Real r_L = 4.696;             const Real r_R = 1.408;
  const Real p_L = 404400;            const Real p_R = 101100;
  const Real u_L = 0.;                const Real u_R = 0.;
  const Real v_L = 0.;                const Real v_R = 0.;
  const Real g=1.4;
  
  if (physics.dimensions() == 1)
  {
    RealVector3 left, right;
    left  << r_L , r_L*u_L , p_L/(g-1) + 0.5*r_L*u_L*u_L;
    right << r_R , r_R*u_R , p_R/(g-1) + 0.5*r_R*u_R*u_R;
    std::vector<std::string> function(3);
    for (Uint i=0; i<function.size(); ++i)
      function[i]="if(x<=5,"+to_str(left[i])+","+to_str(right[i])+")";
    init_solution.configure_property("Functions",function);    
  }
  else if (physics.dimensions() == 2)
  {
    RealVector4 left, right;
    left  << r_L , r_L*u_L , r_L*v_L, p_L/(g-1) + 0.5*r_L*(u_L*u_L+v_L*v_L);
    right << r_R , r_R*u_R , r_R*v_R, p_R/(g-1) + 0.5*r_R*(u_R*u_R+v_R*v_R);
    std::vector<std::string> function(4);
    for (Uint i=0; i<function.size(); ++i)
      function[i]="if(x<=5&y<=5,"+to_str(left[i])+","+to_str(right[i])+")";
    init_solution.configure_property("Functions",function);    
  }
  else
    throw NotSupported (FromHere(), "more than 2 dimensions not supported");
  
  init_solution.transform(mesh);
  
  ////////////////////////////////////////////////////////////////////////////////
  // Boundary conditions
  ////////////////////////////////////////////////////////////////////////////////

  CFinfo << "Setting Reflective Boundary conditions" << CFendl;  
  if (physics.dimensions() == 1)
  {
    solver.create_bc("inlet",   find_component_recursively_with_name<CRegion>(mesh.topology(),"xneg"),   "CF.FVM.BCReflectCons1D");
    solver.create_bc("outlet",   find_component_recursively_with_name<CRegion>(mesh.topology(),"xpos"),   "CF.FVM.BCReflectCons1D");
  }
  else if (physics.dimensions() == 2)
  {
    solver.create_bc("top",   find_component_recursively_with_name<CRegion>(mesh.topology(),"top"),   "CF.FVM.BCReflectCons2D");
    solver.create_bc("bottom",find_component_recursively_with_name<CRegion>(mesh.topology(),"bottom"),"CF.FVM.BCReflectCons2D");
    solver.create_bc("left",  find_component_recursively_with_name<CRegion>(mesh.topology(),"left"),  "CF.FVM.BCReflectCons2D");
    solver.create_bc("right", find_component_recursively_with_name<CRegion>(mesh.topology(),"right"), "CF.FVM.BCReflectCons2D");
  } 
  else
    throw NotSupported (FromHere(), "more than 2 dimensions not supported");
  
  ////////////////////////////////////////////////////////////////////////////////
  // Writer
  ////////////////////////////////////////////////////////////////////////////////
  
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","mesh_writer");
  tools.add_component(writer);
  writer->configure_property("Fields",std::vector<URI>(1,find_component_with_tag(mesh,"solution").full_path()));
  writer->configure_property("File",model.name()+".msh");
  writer->configure_property("Mesh",mesh.full_path());
  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShockTube::signature_create_model( SignalArgs& args )
{
  SignalFrame& p = args.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("model_name", "shocktube" , "Name for created model" );
  p.set_option<Uint>("nb_cells",  100u , "Number of Cells to be generated");
  p.set_option<Uint>("dimension", 1u , "Dimension of the problem");
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
