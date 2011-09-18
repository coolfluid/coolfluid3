// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"

#include "Math/VariableManager.hpp"
#include "Math/VariablesDescriptor.hpp"

#include "Math/LSS/System.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/FieldManager.hpp"
#include "Mesh/Geometry.hpp"

#include "Solver/Tags.hpp"
#include "Solver/Actions/CSolveSystem.hpp"

#include "Physics/PhysModel.hpp"

#include "LinearSolver.hpp"
#include "SparsityBuilder.hpp"
#include "Tags.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Math;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

class ZeroAction : public Common::CAction
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

Common::ComponentBuilder < ZeroAction, CAction, LibUFEM > ZeroAction_Builder;

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

  // Create fields using the known tags
  field_manager().create_field(Tags::solution(), mesh.geometry());
  field_manager().create_field(Tags::source_terms(), mesh.geometry());
  
  if(Common::PE::Comm::instance().is_active())
  {
    Field::Ptr solution = find_component_ptr_with_tag<Field>(mesh.geometry(), Tags::solution());
    if(is_not_null(solution))
      solution->parallelize_with(mesh.geometry().comm_pattern());
    
    Field::Ptr source_terms = find_component_ptr_with_tag<Field>(mesh.geometry(), Tags::source_terms());
    if(is_not_null(source_terms))
      source_terms->parallelize_with(mesh.geometry().comm_pattern());
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
  cf_assert(!m_implementation->m_lss.expired());

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
} // CF
