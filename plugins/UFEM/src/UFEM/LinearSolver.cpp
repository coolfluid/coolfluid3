// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"

#include "Mesh/CNodes.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/Actions/CSolveSystem.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/VariableManager.hpp"

#include "LinearSolver.hpp"
#include <Mesh/CDomain.hpp>

namespace CF {
namespace UFEM {

using namespace Common;
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
    
    m_lss.lock()->set_zero();
  }
  
  boost::weak_ptr<CEigenLSS> m_lss;
};

struct LinearSolver::Implementation
{
  Implementation(Component& comp) :
   m_component(comp),
   m_solver(comp.create_static_component<CSolveSystem>("LSSSolveAction")),
   m_bc(comp.create_static_component<BoundaryConditions>("BoundaryConditions")),
   m_zero_action(comp.create_static_component<ZeroAction>("ZeroLSS")),
   m_physical_model(),
   m_proxy(m_solver.option("lss"),
           *(m_component.options().add_option( OptionComponent<Physics::PhysModel>::create("physical_model", &m_physical_model) )
              ->set_pretty_name("Physical Model")
              ->set_description("Physical Model"))),
   system_matrix(m_proxy),
   system_rhs(m_proxy),
   dirichlet(m_proxy),
   solution(m_proxy)
  {
    m_solver.option("lss").link_to(&m_lss)->attach_trigger(boost::bind(&Implementation::trigger_lss, this));
  }
  
  void trigger_lss()
  {
    if(m_lss.expired())
      return;

    m_bc.option("lss").change_value(m_lss.lock()->uri());
    m_zero_action.m_lss = m_lss;
  }
  
  // Checked access to the physical model
  Physics::PhysModel& physical_model()
  {
    if(m_physical_model.expired())
      throw SetupError(FromHere(), "Error accessing physical model from " + m_component.uri().string());
    
    return *m_physical_model.lock();
  }

  Component& m_component;
  CSolveSystem& m_solver;
  BoundaryConditions& m_bc;
  ZeroAction& m_zero_action;
  boost::weak_ptr<Physics::PhysModel> m_physical_model;
  LSSProxy m_proxy;

  SystemMatrix system_matrix;
  SystemRHS system_rhs;
  DirichletBC dirichlet;
  SolutionVector solution;
  
  boost::weak_ptr<CEigenLSS> m_lss;
  boost::weak_ptr<CMesh> m_mesh;
};

LinearSolver::LinearSolver(const std::string& name) :
  CSolver(name),
  m_implementation( new Implementation(*this) ),
  system_matrix(m_implementation->system_matrix),
  system_rhs(m_implementation->system_rhs),
  dirichlet(m_implementation->dirichlet),
  solution(m_implementation->solution)
{
}

LinearSolver::~LinearSolver()
{
}

void LinearSolver::execute()
{
  if(m_implementation->m_lss.expired())
    throw SetupError(FromHere(), "Error executing " + uri().string() + ": Invalid LSS");
  
  Physics::PhysModel& physical_model = m_implementation->physical_model();
  
  if(m_implementation->m_mesh.expired())
    throw SetupError(FromHere(), "No mesh set for solver at " + uri().string());
  
  CMesh& mesh = *m_implementation->m_mesh.lock();
  
  m_implementation->m_lss.lock()->resize(physical_model.variable_manager().nb_dof() * mesh.topology().nodes().size());
  CSolver::execute();
}

void LinearSolver::mesh_changed(CMesh& mesh)
{
  // Set the region of all children to the root region of the mesh
  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  configure_option_recursively("regions", root_regions);
  m_implementation->m_mesh = mesh.as_ptr<CMesh>();
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


} // UFEM
} // CF
