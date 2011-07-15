// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

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
#include "Mesh/Actions/CreateSpaceP0.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CTime.hpp"
#include "Solver/CModelUnsteady.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/Actions/CForAllElements.hpp"
#include "Solver/Actions/CForAllFaces.hpp"
#include "Solver/Actions/CLoop.hpp"

#include "FVM/Core/FiniteVolumeSolver.hpp"
#include "FVM/Core/ShockTube.hpp"
#include "FVM/Core/BuildGhostStates.hpp"

namespace CF {
namespace FVM {
namespace Core {

using namespace boost::assign;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

Common::ComponentBuilder < ShockTube, CWizard, LibCore > ShockTube_Builder;

////////////////////////////////////////////////////////////////////////////////

ShockTube::ShockTube ( const std::string& name  ) :
  CWizard ( name )
{
  add_tag(LibCore::library_namespace());
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
  CModelUnsteady& model = *Common::Core::instance().root().create_component_ptr<CModelUnsteady>( name );

  ////////////////////////////////////////////////////////////////////////////////
  // Create Physics
  ////////////////////////////////////////////////////////////////////////////////

  CFinfo << "Creating physics" << CFendl;
  Physics::PhysModel& physics = model.create_physics("physics");
  physics.configure_option( "Dimensions", p.get_option<Uint>("dimension") );

  ////////////////////////////////////////////////////////////////////////////////
  // Create tools
  ////////////////////////////////////////////////////////////////////////////////

  CMeshTransformer& finite_volume_transformer = model.tools().create_component<CMeshTransformer>("FiniteVolumeTransformer");
  finite_volume_transformer.create_component_ptr<CBuildFaces>     ("1_build_faces")->mark_basic();
  finite_volume_transformer.create_component_ptr<BuildGhostStates>("2_build_ghoststates")->mark_basic();
  finite_volume_transformer.create_component_ptr<CreateSpaceP0>   ("3_create_space_P0")->mark_basic();
  finite_volume_transformer.create_component_ptr<CBuildVolume>    ("4_build_volume_field")->mark_basic();

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
      mesh_ptr = domain.create_component_ptr<CMesh>("line");
      CSimpleMeshGenerator::create_line(*mesh_ptr, 10. , p.get_option<Uint>("nb_cells"));
      break;
    case 2:
      mesh_ptr = domain.create_component_ptr<CMesh>("square");
      CSimpleMeshGenerator::create_rectangle(*mesh_ptr, 10. , 10. , p.get_option<Uint>("nb_cells"),  p.get_option<Uint>("nb_cells"));
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
  FiniteVolumeSolver& solver = model.create_solver("CF.FVM.Core.FiniteVolumeSolver").as_type<FiniteVolumeSolver>();
  solver.configure_option("physical_model",physics.uri());
  solver.configure_option("domain",domain.uri());
  solver.configure_option_recursively("time",model.as_type<CModelUnsteady>().time().uri());
  solver.configure_option_recursively("time_accurate",true);

  ////////////////////////////////////////////////////////////////////////////////
  // Initial condition
  ////////////////////////////////////////////////////////////////////////////////

  CFinfo << "Setting initial condition" << CFendl;
  CInitFieldFunction& init_solution = model.tools().create_component<CInitFieldFunction>("init_solution");
  init_solution.configure_option("field",find_component_with_tag(mesh,"solution").uri());

  const Real r_L = 4.696;             const Real r_R = 1.408;
  const Real p_L = 404400;            const Real p_R = 101100;
  const Real u_L = 0.;                const Real u_R = 0.;
  const Real v_L = 0.;                const Real v_R = 0.;
  const Real g=1.4;

  if (physics.ndim() == 1)
  {
    RealVector3 left, right;
    left  << r_L , r_L*u_L , p_L/(g-1) + 0.5*r_L*u_L*u_L;
    right << r_R , r_R*u_R , p_R/(g-1) + 0.5*r_R*u_R*u_R;
    std::vector<std::string> function(3);
    for (Uint i=0; i<function.size(); ++i)
      function[i]="if(x<=5,"+to_str(left[i])+","+to_str(right[i])+")";
    init_solution.configure_option("functions",function);
  }
  else if (physics.ndim() == 2)
  {
    RealVector4 left, right;
    left  << r_L , r_L*u_L , r_L*v_L, p_L/(g-1) + 0.5*r_L*(u_L*u_L+v_L*v_L);
    right << r_R , r_R*u_R , r_R*v_R, p_R/(g-1) + 0.5*r_R*(u_R*u_R+v_R*v_R);
    std::vector<std::string> function(4);
    for (Uint i=0; i<function.size(); ++i)
      function[i]="if(x<=5&y<=5,"+to_str(left[i])+","+to_str(right[i])+")";
    init_solution.configure_option("functions",function);
  }
  else
    throw NotSupported (FromHere(), "more than 2 dimensions not supported");

  init_solution.transform(mesh);

  ////////////////////////////////////////////////////////////////////////////////
  // Boundary conditions
  ////////////////////////////////////////////////////////////////////////////////

  CFinfo << "Setting Reflective Boundary conditions" << CFendl;
  if (physics.ndim() == 1)
  {
    solver.create_bc("inlet",   find_component_recursively_with_name<CRegion>(mesh.topology(),"xneg"),   "CF.FVM.Core.BCReflectCons1D");
    solver.create_bc("outlet",   find_component_recursively_with_name<CRegion>(mesh.topology(),"xpos"),   "CF.FVM.Core.BCReflectCons1D");
  }
  else if (physics.ndim() == 2)
  {
    solver.create_bc("top",   find_component_recursively_with_name<CRegion>(mesh.topology(),"top"),   "CF.FVM.Core.BCReflectCons2D");
    solver.create_bc("bottom",find_component_recursively_with_name<CRegion>(mesh.topology(),"bottom"),"CF.FVM.Core.BCReflectCons2D");
    solver.create_bc("left",  find_component_recursively_with_name<CRegion>(mesh.topology(),"left"),  "CF.FVM.Core.BCReflectCons2D");
    solver.create_bc("right", find_component_recursively_with_name<CRegion>(mesh.topology(),"right"), "CF.FVM.Core.BCReflectCons2D");
  }
  else
    throw NotSupported (FromHere(), "more than 2 dimensions not supported");

  ////////////////////////////////////////////////////////////////////////////////
  // Writer
  ////////////////////////////////////////////////////////////////////////////////

  CMeshWriter::Ptr writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","mesh_writer");
  model.tools().add_component(writer);
  writer->configure_option("fields",std::vector<URI>(1,find_component_with_tag(mesh,"solution").uri()));
  writer->configure_option("file",URI(model.name()+".msh"));
  writer->configure_option("mesh",mesh.uri());

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

} // Core
} // FVM
} // CF
