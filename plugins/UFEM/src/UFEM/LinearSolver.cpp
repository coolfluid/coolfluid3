// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/CBuilder.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/CDomain.hpp"
#include "mesh/CMesh.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Geometry.hpp"

#include "Solver/Tags.hpp"
#include "Solver/Actions/CSolveSystem.hpp"
#include "Solver/Actions/Proto/CProtoAction.hpp"

#include "Physics/PhysModel.hpp"

#include "LinearSolver.hpp"
#include "SparsityBuilder.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace math;
using namespace mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

class ZeroAction : public common::CAction
{
public:

  typedef boost::shared_ptr<ZeroAction> Ptr;
  typedef boost::shared_ptr<ZeroAction const> ConstPtr;

  ZeroAction(const std::string& name) : CAction(name)
  {
  }

  static std::string type_name () { return "ZeroAction"; }

  virtual void execute()
  {
    if(m_lss.expired())
      throw SetupError(FromHere(), "No LSS found when running " + uri().string());

    m_lss.lock()->reset();
  }

  boost::weak_ptr<LSS::System> m_lss;
};

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ZeroAction, CAction, LibUFEM > ZeroAction_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct LinearSolver::Implementation
{
  Implementation(Component& comp) :
   m_component(comp),
   m_solver(comp.create_static_component<CSolveSystem>("LSSSolveAction")),
   m_bc(comp.create_static_component<BoundaryConditions>("BoundaryConditions")),
   m_zero_action(comp.create_static_component<ZeroAction>("ZeroLSS")),
   system_matrix(*(m_component.options().add_option< OptionComponent<LSS::System> >("lss")->pretty_name("LSS"))),
   system_rhs(m_component.option("lss")),
   dirichlet(m_component.option("lss")),
   solution(m_component.option("lss")),
   m_updating(false)
  {
  }

  Component& m_component;
  CSolveSystem& m_solver;
  BoundaryConditions& m_bc;
  ZeroAction& m_zero_action;

  SystemMatrix system_matrix;
  SystemRHS system_rhs;
  DirichletBC dirichlet;
  SolutionVector solution;

  boost::weak_ptr<LSS::System> m_lss;

  bool m_updating;
};

LinearSolver::LinearSolver(const std::string& name) :
  CSimpleSolver(name),
  m_implementation( new Implementation(*this) ),
  system_matrix(m_implementation->system_matrix),
  system_rhs(m_implementation->system_rhs),
  dirichlet(m_implementation->dirichlet),
  solution(m_implementation->solution)
{
  option("lss").attach_trigger(boost::bind(&LinearSolver::trigger_lss, this));
}

LinearSolver::~LinearSolver()
{
}

void LinearSolver::execute()
{
  if(m_implementation->m_lss.expired())
    throw SetupError(FromHere(), "Error executing " + uri().string() + ": Invalid LSS");

  CSimpleSolver::execute();
}

void LinearSolver::mesh_loaded(CMesh& mesh)
{
  CSimpleSolver::mesh_loaded(mesh);
  mesh_changed(mesh);
}

void LinearSolver::mesh_changed(CMesh& mesh)
{
  CFdebug << "UFEM::LinearSolver: Reacting to mesh_changed signal" << CFendl;

  // Ensure the comm pattern will be updated
  if(is_not_null(mesh.geometry().get_child_ptr("CommPattern")))
  {
    mesh.geometry().remove_component("CommPattern");
  }

  // Find out what tags are used
  std::set<std::string> tags;
  BOOST_FOREACH(const CProtoAction& action, find_components_recursively<CProtoAction>(*this))
  {
    action.insert_tags(tags);
  }

  // Create fields as needed
  BOOST_FOREACH(const std::string& tag, tags)
  {
    Field::Ptr field = find_component_ptr_with_tag<Field>(mesh.geometry(), tag);

    // If the field was created before, destroy it
    if(is_not_null(field))
    {
      CFdebug << "Removing existing field " << field->uri().string() << CFendl;
      field->parent().remove_component(field->name());
      field.reset();
    }

    CFdebug << "Creating field with tag " << tag << CFendl;

    // Create the field
    field_manager().create_field(tag, mesh.geometry());
    field = find_component_ptr_with_tag<Field>(mesh.geometry(), tag);
    cf3_assert(is_not_null(field));

    // Parallelize
    if(common::PE::Comm::instance().is_active())
    {
      field->parallelize_with(mesh.geometry().comm_pattern());
    }
  }

  // Set the region of all children to the root region of the mesh
  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  configure_option_recursively(Solver::Tags::regions(), root_regions);

  trigger_lss();
}


CAction& LinearSolver::zero_action()
{
  return m_implementation->m_zero_action;
}

CAction& LinearSolver::solve_action()
{
  return m_implementation->m_solver;
}

BoundaryConditions& LinearSolver::boundary_conditions()
{
  return m_implementation->m_bc;
}

void LinearSolver::trigger_lss()
{
  if(!dynamic_cast<OptionComponent<LSS::System>&>(option("lss")).check())
    return;

  if(m_implementation->m_updating) // avoid recursion
      return;

  m_implementation->m_updating = true;

  m_implementation->m_lss = dynamic_cast<OptionComponent<LSS::System>&>(option("lss")).component().as_ptr<LSS::System>();
  cf3_assert(!m_implementation->m_lss.expired());

  // Create the LSS if the mesh is set
  if(!m_mesh.expired() && !m_implementation->m_lss.lock()->is_created())
  {
    VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physics().variable_manager(), UFEM::Tags::solution());

    std::vector<Uint> node_connectivity, starting_indices;
    build_sparsity(mesh(), node_connectivity, starting_indices);

    m_implementation->m_lss.lock()->create(mesh().geometry().comm_pattern(), descriptor.size(), node_connectivity, starting_indices);
  }

  configure_option_recursively("lss", option("lss").value<URI>());
  m_implementation->m_zero_action.m_lss = m_implementation->m_lss;
  m_implementation->m_updating = false;
}



} // UFEM
} // cf3
