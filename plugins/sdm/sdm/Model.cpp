// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>
//#include <boost/algorithm/string.hpp>
#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
//#include "common/EventHandler.hpp"
#include "common/Group.hpp"
//#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionArray.hpp"
#include "common/Signal.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/actions/CreateField.hpp"
#include "mesh/actions/BuildFaces.hpp"

#include "mesh/Cells.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Domain.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"

//#include "solver/History.hpp"
#include "solver/Time.hpp"
#include "solver/TimeStepping.hpp"
//#include "solver/actions/SynchronizeFields.hpp"
//#include "solver/actions/Probe.hpp"

#include "sdm/BoundaryConditions.hpp"
#include "sdm/Tags.hpp"
#include "sdm/Model.hpp"
#include "sdm/TimeIntegrationStepComputer.hpp"
#include "sdm/DomainDiscretization.hpp"
#include "sdm/Solver.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::physics;
using namespace cf3::solver;
//using namespace cf3::solver::actions;


namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Model, solver::Model, LibSDM > Model_Builder;

////////////////////////////////////////////////////////////////////////////////

Model::Model ( const std::string& name  ) :
  solver::Model ( name )
{
  // properties

  properties()["brief"] = std::string("Spectral Finite Difference Simulation model");
  properties()["description"] = std::string("Long description not available");

  m_time = create_static_component<solver::Time>("time");
  m_time_stepping = create_static_component<solver::TimeStepping>("time_stepping");
  m_time_stepping->options().set(solver::Tags::time(),m_time);

  m_domain_discretization = create_static_component<DomainDiscretization>("domain_discretization");
  m_boundary_conditions = create_static_component<BoundaryConditions>("boundary_conditions");

  create_domain("domain");

  regist_signal ( "create_solution_space" )
      .description( "Create the solution space" )
      .pretty_name("Create Solution Space" )
      .connect   ( boost::bind ( &Model::signal_create_solution_space,    this, _1 ) )
      .signature ( boost::bind ( &Model::signature_create_solution_space, this, _1 ) );

  regist_signal ( "create_field" )
      .description( "Create a field" )
      .pretty_name("Create Field" )
      .connect   ( boost::bind ( &Model::signal_create_field,    this, _1 ) )
      .signature ( boost::bind ( &Model::signature_create_field, this, _1 ) );

  tools().create_component<mesh::actions::CreateField>("field_creator");

  regist_signal ( "set_time_integration" )
      .description( "Set Time Integration" )
      .pretty_name("Set Time Integration" )
      .connect   ( boost::bind ( &Model::signal_set_time_integration,    this, _1 ) )
      .signature ( boost::bind ( &Model::signature_set_time_integration, this, _1 ) );

  m_time_integration = create_static_component<Group>("time_integration");
  m_time_integration->mark_basic();

  // Set some defaults
  set_time_integration_scheme("cf3.sdm.ExplicitRungeKuttaLowStorage2");
  set_time_integration_step("cf3.sdm.ImposeCFL");
  set_time_integration_solver("cf3.sdm.ExplicitSolver");

  options().add( "solution", m_solution)
      .mark_basic()
      .link_to(&m_solution)
      .attach_trigger(boost::bind( &Model::config_solution, this));

  m_time_stepping->post_actions() << *m_boundary_conditions;
}

////////////////////////////////////////////////////////////////////////////////

Model::~Model()
{
}

////////////////////////////////////////////////////////////////////////////////

void Model::set_time_integration_scheme(const std::string& type)
{
  if (is_not_null(m_time_integration_scheme))
  {
    m_time_integration->remove_component(*m_time_integration_scheme);
  }
  m_time_integration_scheme = m_time_integration->create_component("scheme",type);
  m_time_integration_scheme->mark_basic();
  m_time_integration_scheme->configure_option_recursively("domain_discretization",m_domain_discretization);
  if (m_time_integration_scheme->options().check("time"))
    m_time_integration_scheme->options().set("time",m_time);

  if ( is_not_null(m_solution) )
  {
    m_time_integration_scheme->options().set(sdm::Tags::solution(),m_solution);
    m_time_integration_scheme->options().set(sdm::Tags::residual(),m_solution->dict().field(sdm::Tags::residual()).handle());
    m_time_integration_scheme->options().set(sdm::Tags::update_coeff(),m_solution->dict().field(sdm::Tags::update_coeff()).handle());
  }
}

////////////////////////////////////////////////////////////////////////////////

void Model::set_time_integration_step(const std::string& type)
{
  if (is_not_null(m_time_integration_step))
  {
    m_time_integration->remove_component(*m_time_integration_step);
  }
  m_time_integration_step = m_time_integration->create_component<TimeIntegrationStepComputer>("step",type);
  m_time_integration_step->mark_basic();

  m_time_integration_step->options().set(sdm::Tags::time(),m_time);

  m_time_integration_scheme->options().set("time_step_computer",m_time_integration_step);

  if ( is_not_null(m_solution) )
  {
    m_time_integration_step->options().set(sdm::Tags::wave_speed(),m_solution->dict().field(sdm::Tags::wave_speed()).handle());
  }

}

////////////////////////////////////////////////////////////////////////////////

void Model::set_time_integration_solver(const std::string& type)
{
  if (is_not_null(m_time_integration_solver))
  {
    m_time_stepping->remove_component(m_time_integration_solver->name());
    m_time_integration->remove_component(*m_time_integration_solver);
  }
  m_time_integration_solver = m_time_integration->create_component("solver",type)->handle<Solver>();
  m_time_integration_solver->mark_basic();
  m_time_integration_solver->options().set("time",m_time);
  m_time_integration_solver->options().set("time_integration",m_time_integration_scheme);

  if (m_solution)
  {
    m_time_integration_solver->options().set("dict",m_solution->dict().handle());
  }

  *m_time_integration_solver->get_child("pre_update")->handle<common::ActionDirector>() << *m_boundary_conditions;
  *m_time_stepping << *m_time_integration_solver;


}

////////////////////////////////////////////////////////////////////////////////

Handle<mesh::Dictionary> Model::create_solution_space(const Uint& order, const std::vector<Handle<Component> >& regions)
{
  if (find_components<Mesh>(domain()).size() == 0)
    throw SetupError(FromHere(), "Could not create solution_space because no meshes were found in "+domain().uri().string());

  boost_foreach(Mesh& mesh, find_components<Mesh>(domain()) )
  {
    build_faces(mesh);
  }

  Handle<Dictionary> dict = create_component<DiscontinuousDictionary>("solution_space");

  std::string space_lib_name = "cf3.sdm.P"+to_str(order-1);
  CFinfo << "Creating Disontinuous space " << dict->uri() << " ("<<space_lib_name<<") for entities" << CFendl;
  boost_foreach(const Handle<Component>& comp, regions)
  {
    boost_foreach(const Entities& entities, find_components_recursively<Entities>( *comp ) )
    {
      CFinfo << "    -  " <<  entities.uri() << CFendl;
    }
  }

  boost_foreach(const Handle<Component>& comp, regions)
  {
    boost_foreach(Entities& entities, find_components_recursively<Entities>( *comp ) )
    {
      entities.create_space(space_lib_name+"."+entities.element_type().shape_name(),*dict);
    }
  }
  dict->build();

  boost_foreach(Mesh& mesh, find_components<Mesh>(domain()) )
  {
    mesh.update_structures();
  }

  m_solution_space = dict;

  return dict;
}

////////////////////////////////////////////////////////////////////////////////

void Model::build_faces(Mesh& mesh)
{
  boost::shared_ptr<BuildFaces> build_faces = allocate_component<BuildFaces>("build_inner_faces");
  build_faces->options().set("store_cell2face",true);
  build_faces->transform(mesh);
}

////////////////////////////////////////////////////////////////////////////////

void Model::config_solution()
{
  // create residual and update_coefficient field
  Handle<Component> found;

  Handle<Dictionary> dict = m_solution->dict().handle<Dictionary>();

  Handle<Field> residual;
  if ( found = dict->get_child(sdm::Tags::residual()) )
  {
    residual = found->handle<Field>();
  }
  else
  {
    residual = dict->create_field(sdm::Tags::residual(), m_solution->descriptor().description()).handle<Field>();
    residual->descriptor().prefix_variable_names("rhs_");
    residual->properties()[sdm::Tags::L2norm()]=0.;
  }

  Handle<Field> update_coeff;
  if ( found = dict->get_child(sdm::Tags::update_coeff()) )
  {
    update_coeff = found->handle<Field>();
  }
  else
  {
    update_coeff = dict->create_field(sdm::Tags::update_coeff(), "uc[1]").handle<Field>();
  }

  Handle<Field> wave_speed;
  if ( found = dict->get_child(sdm::Tags::wave_speed()) )
  {
    wave_speed = found->handle<Field>();
  }
  else
  {
    wave_speed = dict->create_field(sdm::Tags::wave_speed(), "ws[1]").handle<Field>();
  }


  Field& jacob_det = dict->create_field(sdm::Tags::jacob_det(), "jacob_det[1]");
  Field& delta = dict->create_field(sdm::Tags::delta(), "delta[vector]");

  boost_foreach(const Handle<Entities>& elements, dict->entities_range())
  {
    if ( is_null(elements->handle<Cells>()) ) continue;
    const Space& space = dict->space(*elements);

    const RealMatrix& local_coords = space.shape_function().local_coordinates();

    RealMatrix geometry_coords;
    elements->geometry_space().allocate_coordinates(geometry_coords);

    RealVector dKsi (elements->element_type().dimensionality()); dKsi.setConstant(2.);
    RealVector dX (elements->element_type().dimension());
    RealMatrix jacobian(elements->element_type().dimensionality(),elements->element_type().dimension());

    const Connectivity& field_connectivity = space.connectivity();

    for (Uint elem=0; elem<elements->size(); ++elem)
    {
      elements->geometry_space().put_coordinates(geometry_coords,elem);

      for (Uint node=0; node<local_coords.rows();++node)
      {
        const Uint p = field_connectivity[elem][node];
        jacob_det[p][0]=elements->element_type().jacobian_determinant(local_coords.row(node),geometry_coords);
        if (jacob_det[p][0] < 0)
          throw BadValue(FromHere(), "jacobian determinant is negative ("+to_str(jacob_det[p][0])+") in cell "+elements->uri().string()+"["+to_str(elem)+"] (glbidx="+to_str(+elements->glb_idx()[elem])+"). This is caused by a faulty node ordering in the mesh.");
        elements->element_type().compute_jacobian(local_coords.row(node),geometry_coords,jacobian);
        dX.noalias() = jacobian.transpose()*dKsi;
        for (Uint d=0; d<dX.size(); ++d)
          delta[p][d]=dX[d];
      }

    }
  }


  m_solution->parallelize();
  residual->parallelize();
  update_coeff->parallelize();
  wave_speed->parallelize();

  m_domain_discretization->options().set(sdm::Tags::wave_speed(),wave_speed);

  m_boundary_conditions->configure_option_recursively(sdm::Tags::solution(),m_solution);

  if ( is_not_null(m_time_integration_solver) )
  {
    m_time_integration_solver->options().set("dict",m_solution->dict().handle());
  }

  if ( is_not_null(m_time_integration_scheme) )
  {
    m_time_integration_scheme->options().set(sdm::Tags::solution(),m_solution);
    m_time_integration_scheme->options().set(sdm::Tags::residual(),residual);
    m_time_integration_scheme->options().set(sdm::Tags::update_coeff(),update_coeff);
  }

  if ( is_not_null(m_time_integration_step) )
  {
    m_time_integration_step->options().set(sdm::Tags::wave_speed(),wave_speed);
    m_time_integration_step->options().set(sdm::Tags::time(),m_time);
  }

}

////////////////////////////////////////////////////////////////////////////////

void Model::signal_create_solution_space( common::SignalArgs& args)
{
  common::XML::SignalOptions opts(args);
  Handle<Dictionary> dict = create_solution_space(opts.value<Uint>("order"),opts.value< std::vector< Handle<Component> > >("regions"));

  common::XML::SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", dict->uri());
}

////////////////////////////////////////////////////////////////////////////////

void Model::signature_create_solution_space( common::SignalArgs& args)
{
  common::XML::SignalOptions opts(args);
  opts.add("order",2u);
  opts.add("regions",std::vector< Handle<Component> >(1,domain().handle()));
}

////////////////////////////////////////////////////////////////////////////////

void Model::signal_set_time_integration( common::SignalArgs& args)
{
  common::XML::SignalOptions opts(args);

  // Create scheme
  set_time_integration_scheme(opts.value<std::string>("scheme"));

  // Create step
  set_time_integration_step(opts.value<std::string>("step"));

  // Create solver
  set_time_integration_solver(opts.value<std::string>("solver"));

  common::XML::SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", m_time_integration->uri());
}

////////////////////////////////////////////////////////////////////////////////

void Model::signature_set_time_integration( common::SignalArgs& args)
{
  common::XML::SignalOptions opts(args);
  opts.add("scheme",std::string("cf3.sdm.ExplicitRungeKuttaLowStorage2"));
  opts.add("solver",std::string("cf3.sdm.ExplicitSolver"));
  opts.add("step",std::string("cf3.sdm.ImposeCFL"));
}

////////////////////////////////////////////////////////////////////////////////

void Model::signal_create_field( common::SignalArgs& args)
{
  if (is_null(m_solution_space))
    throw SetupError ( FromHere(), "First create solution space");

  common::XML::SignalOptions opts(args);
  Handle<Field>  field = tools().get_child("field_creator")->handle<mesh::actions::CreateField>()
                            ->create_field(opts.value<std::string>("name"),*m_solution_space,opts.value<std::vector<std::string> >("functions"));

  if (field->name() == "solution")
    options().set("solution",field);

  common::XML::SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", field->uri());
}

////////////////////////////////////////////////////////////////////////////////

void Model::signature_create_field( common::SignalArgs& args)
{
  common::XML::SignalOptions opts(args);
  opts.add("name",std::string("solution"));
  opts.add("functions",std::vector<std::string>());
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
