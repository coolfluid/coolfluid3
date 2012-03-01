// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/Tags.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

#include "physics/PhysModel.hpp"

#include "LinearSolver.hpp"
#include "SparsityBuilder.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace math;
using namespace mesh;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

struct LinearSolver::Implementation
{
  Implementation(Component& comp) :
   m_component(comp),
   system_matrix(m_component.options().add_option("lss", Handle<LSS::System>()).pretty_name("LSS")),
   system_rhs(m_component.options().option("lss")),
   dirichlet(m_component.options().option("lss")),
   solution(m_component.options().option("lss")),
   m_updating(false)
  {
  }

  Component& m_component;
  SystemMatrix system_matrix;
  SystemRHS system_rhs;
  DirichletBC dirichlet;
  SolutionVector solution;

  Handle<LSS::System> m_lss;

  bool m_updating;
};

LinearSolver::LinearSolver(const std::string& name) :
  SimpleSolver(name),
  m_implementation( new Implementation(*this) ),
  system_matrix(m_implementation->system_matrix),
  system_rhs(m_implementation->system_rhs),
  dirichlet(m_implementation->dirichlet),
  solution(m_implementation->solution)
{
  options().option("lss").attach_trigger(boost::bind(&LinearSolver::trigger_lss, this));
}

LinearSolver::~LinearSolver()
{
}

void LinearSolver::execute()
{
  if(is_null(m_implementation->m_lss))
    throw SetupError(FromHere(), "Error executing " + uri().string() + ": Invalid LSS");

  SimpleSolver::execute();
}

void LinearSolver::mesh_loaded(Mesh& mesh)
{
  SimpleSolver::mesh_loaded(mesh);
  mesh_changed(mesh);
}

void LinearSolver::mesh_changed(Mesh& mesh)
{
  CFdebug << "UFEM::LinearSolver: Reacting to mesh_changed signal" << CFendl;

  // Ensure the comm pattern will be updated
  if(is_not_null(mesh.geometry_fields().get_child("CommPattern")))
  {
    mesh.geometry_fields().remove_component("CommPattern");
  }

  // Find out what tags are used
  std::set<std::string> tags;
  BOOST_FOREACH(const ProtoAction& action, find_components_recursively<ProtoAction>(*this))
  {
    action.insert_tags(tags);
  }

  // Create fields as needed
  BOOST_FOREACH(const std::string& tag, tags)
  {
    Handle< Field > field = find_component_ptr_with_tag<Field>(mesh.geometry_fields(), tag);

    // If the field was created before, destroy it
    if(is_not_null(field))
    {
      CFdebug << "Removing existing field " << field->uri().string() << CFendl;
      field->parent()->remove_component(field->name());
      field.reset();
    }

    CFdebug << "Creating field with tag " << tag << CFendl;

    // Create the field
    field_manager().create_field(tag, mesh.geometry_fields());
    field = find_component_ptr_with_tag<Field>(mesh.geometry_fields(), tag);
    cf3_assert(is_not_null(field));

    // Parallelize
    if(common::PE::Comm::instance().is_active())
    {
      field->parallelize_with(mesh.geometry_fields().comm_pattern());
    }
  }

  // Set the region of all children to the root region of the mesh
  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  configure_option_recursively(solver::Tags::regions(), root_regions);

  trigger_lss();
}

void LinearSolver::trigger_lss()
{
  m_implementation->m_lss = options().option("lss").value< Handle<LSS::System> >();
  if(is_null(m_implementation->m_lss))
    return;

  if(m_implementation->m_updating) // avoid recursion
    return;

  m_implementation->m_updating = true;

  // Create the LSS if the mesh is set
  if(is_not_null(m_mesh) && !m_implementation->m_lss->is_created())
  {
    VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physics().variable_manager(), UFEM::Tags::solution());

    std::vector<Uint> node_connectivity, starting_indices;
    build_sparsity(mesh(), node_connectivity, starting_indices);

    CFdebug << "Creating LSS for " << starting_indices.size()-1 << " blocks" << CFendl;
    m_implementation->m_lss->create(mesh().geometry_fields().comm_pattern(), descriptor.size(), node_connectivity, starting_indices);
    CFdebug << "Finished creating LSS" << CFendl;
  }

  configure_option_recursively("lss", options().option("lss").value< Handle<LSS::System> >());

  m_implementation->m_updating = false;
}



} // UFEM
} // cf3
