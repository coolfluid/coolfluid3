// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/assign/list_of.hpp>
#include "common/CF.hpp"
#include "common/Exception.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/Group.hpp"

#include "math/Consts.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Domain.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/actions/LoadBalance.hpp"
#include "mesh/actions/Interpolate.hpp"

#include "solver/ModelUnsteady.hpp"
#include "solver/Time.hpp"

#include "physics/PhysModel.hpp"
#include "sdm/SDSolver.hpp"
#include "sdm/Term.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::math::Consts;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::solver;
using namespace cf3::physics;
using namespace cf3::sdm;


////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);
  PE::Comm::instance().init(argc, argv);

  try
  {
    // Create simulation

    Handle<ModelUnsteady> model = Core::instance().root().create_component<ModelUnsteady>("machcone_3d");
    Time&      time     = model->create_time();
    PhysModel& physics  = model->create_physics("cf3.physics.LinEuler.LinEuler3D");
    Solver&    solver   = model->create_solver("cf3.sdm.SDSolver");
    Domain&    domain   = model->create_domain("domain");

    // Create mesh

    Handle<Mesh>          mesh           = domain.create_component<Mesh>("mesh");
    Handle<MeshGenerator> mesh_generator = model->tools().create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")->handle<MeshGenerator>();
    mesh_generator->options().set("mesh",mesh->uri());
    mesh_generator->options().set("nb_cells",std::vector<Uint>(3,10));
    mesh_generator->options().set("lengths",std::vector<Real>(3,120.));
    std::vector<Real> offsets = list_of(0.)(-60)(-60);
    mesh_generator->options().set("offsets",offsets);
    mesh_generator->execute();
    allocate_component<LoadBalance>("repartitioner")->transform(mesh);

    // Configure solver

    solver.options().set("mesh",mesh);
    solver.options().set("time",time.handle<Time>());
    solver.options().set("solution_vars",std::string("cf3.physics.LinEuler.Cons3D"));
    solver.options().set("solution_order",5u);

    DomainDiscretization& dd = *solver.get_child("DomainDiscretization")->handle<DomainDiscretization>();

    // Configure timestepping

    solver.access_component("TimeStepping")->options().set("cfl",std::string("0.1"));
    solver.access_component("TimeStepping/IterativeSolver")->options().set("nb_stages",3u);

    // Prepare the mesh for Spectral Difference (build faces and fields etc...)

    solver.get_child("PrepareMesh")->handle<common::Action>()->execute();

    // Create convection term

    Term& convection = dd.create_term("cf3.sdm.lineuler.Convection3D","convection");
    convection.options().set("gamma", 1.);
    convection.options().set("rho0",1.);
    std::vector<Real> U0 = list_of(1.5)(0.)(0.);
    convection.options().set("U0",U0);
    convection.options().set("p0",1.);

    // create monopole source term

    Term& monopole = dd.create_term("cf3.sdm.lineuler.SourceMonopole3D","monopole");
    monopole.options().set("omega",2.*pi()/30. );
    monopole.options().set("alpha",log(2.)/2.);
    monopole.options().set("epsilon",0.5);
    std::vector<Real> monopole_loc = list_of(25.)(0.)(0.);
    monopole.options().set("source_location",monopole_loc);
    monopole.options().set("time", time.handle<Time>());

    // fields to output

    std::vector<URI> fields = list_of
        (mesh->access_component("solution_space/solution")->uri())
        (mesh->access_component("solution_space/wave_speed")->uri())
        (mesh->access_component("solution_space/residual")->uri());

    // Simulate
    Real final_time = 10.;
    Real step = 20.;

    Real simulate_to_time = 0.;
    while (simulate_to_time<final_time)
    {
      simulate_to_time += step;
      time.options().set("end_time",simulate_to_time);

      model->simulate();

      mesh->write_mesh(URI("file:mach_cone_time"+to_str(simulate_to_time)+".plt"),fields);
    }

    // Output on a fine mesh for visualization
    Handle<Mesh> vis_mesh = domain.create_component<Mesh>("vis_mesh");
    mesh_generator->options().set("mesh",vis_mesh->uri());
    mesh_generator->options().set("nb_cells",std::vector<Uint>(3,60));
    mesh_generator->execute();

    Handle<Interpolate> interpolator = model->tools().create_component<Interpolate>("interpolator");

    // need field to interpolate in
    Field& vis_solution = vis_mesh->geometry_fields().create_field("solution","rho[scalar],rho0U[vector],p[scalar]");
    interpolator->interpolate(*mesh->access_component("solution_space/solution")->handle<Field>(),
                              vis_solution.coordinates(),
                              vis_solution);
    vis_mesh->write_mesh(URI("file:mach_cone_vis.plt"),std::vector<URI>(1,vis_solution.uri()));

  }
  catch(Exception & e)
  {
    CFerror << e.what() << CFendl;
  }
  catch ( std::exception& ex )
  {
    CFerror << "Unhandled exception: " << ex.what() << CFendl;
  }
  catch ( ... )
  {
    CFerror << "Detected unknown exception" << CFendl;
  }

  PE::Comm::instance().finalize();
  Core::instance().terminate();

  return 0;
}
