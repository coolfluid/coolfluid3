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
#include "common/PropertyList.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/Group.hpp"

#include "math/Consts.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Domain.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/Interpolator.hpp"
#include "mesh/actions/LoadBalance.hpp"
#include "mesh/actions/Interpolate.hpp"

#include "solver/Time.hpp"
#include "solver/TimeStepping.hpp"

#include "sdm/Term.hpp"
#include "sdm/Model.hpp"
#include "sdm/DomainDiscretization.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::math::Consts;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::solver;
using namespace cf3::sdm;


////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);
  PE::Comm::instance().init(argc, argv);

  try
  {
    // Create simulation

    Handle<sdm::Model> model = Core::instance().root().create_component<sdm::Model>("machcone_3d");

    // Create mesh

    Handle<Mesh>          mesh           = model->domain().create_component<Mesh>("mesh");
    Handle<MeshGenerator> mesh_generator = model->tools().create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")->handle<MeshGenerator>();
    mesh_generator->options().set("mesh",mesh->uri());
    mesh_generator->options().set("nb_cells",std::vector<Uint>(3,10));
    mesh_generator->options().set("lengths",std::vector<Real>(3,120.));
    std::vector<Real> offsets = list_of(0.)(-60)(-60);
    mesh_generator->options().set("offsets",offsets);
    mesh_generator->execute();
    allocate_component<LoadBalance>("repartitioner")->transform(mesh);

    // Configure model
    model->access_component("time_stepping")->options().set("end_time",1.);
    model->access_component("time_stepping")->options().set("time_step",1.);
    model->access_component("time_integration/step")->options().set("cfl",0.2);
    model->access_component("time_integration/scheme")->options().set("nb_stages",3);

    Handle<Dictionary> solution_space = model->create_solution_space(4u,std::vector< Handle<Component> >(1, mesh->handle() ));
    Handle<Field> solution = solution_space->create_field("solution","rho[scal],U[vec],p[scal]").handle<Field>();
    model->options().set("solution",solution);

    Handle<DomainDiscretization> dd = model->access_component("domain_discretization")->handle<DomainDiscretization>();

    // Create convection term

    Term& convection = dd->create_term("convection","cf3.sdm.lineuler.Convection3D");
    convection.options().set("gamma", 1.);
    convection.options().set("rho0",1.);
    std::vector<Real> U0 = list_of(1.5)(0.)(0.);
    convection.options().set("U0",U0);
    convection.options().set("p0",1.);

    // create monopole source term

    Term& monopole = dd->create_term("monopole","cf3.sdm.lineuler.SourceMonopole3D");
    monopole.options().set("omega",2.*pi()/30. );
    monopole.options().set("alpha",log(2.)/2.);
    monopole.options().set("epsilon",0.5);
    std::vector<Real> monopole_loc = list_of(25.)(0.)(0.);
    monopole.options().set("source_location",monopole_loc);
    monopole.options().set("time", model->access_component("time"));

    // fields to output

    std::vector<URI> fields = list_of
        (solution_space->access_component("solution")->uri())
        (solution_space->access_component("wave_speed")->uri())
        (solution_space->access_component("residual")->uri());

    // Simulate
    while (model->access_component("time_stepping")->properties().value<bool>("finished") == false)
    {
      model->access_component("time_stepping")->handle<TimeStepping>()->do_step();

      mesh->write_mesh(URI("file:mach_cone_time"+to_str(model->access_component("time")->options().value<Real>("current_time"))+".plt"),fields);
    }

    // Output on a fine mesh for visualization
    Handle<Mesh> vis_mesh = model->domain().create_component<Mesh>("vis_mesh");
    mesh_generator->options().set("mesh",vis_mesh->uri());
    mesh_generator->options().set("nb_cells",std::vector<Uint>(3,60));
    mesh_generator->execute();

    Handle<AInterpolator> interpolator = model->tools().create_component<AInterpolator>("interpolator","cf3.mesh.ShapeFunctionInterpolator");

    // need field to interpolate in
    Field& vis_solution = vis_mesh->geometry_fields().create_field("solution","rho[scalar],rho0U[vector],p[scalar]");
    interpolator->interpolate(*solution_space->access_component("solution")->handle<Field>(), vis_solution);
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
