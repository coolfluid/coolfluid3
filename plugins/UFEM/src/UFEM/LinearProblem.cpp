// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"

#include "Mesh/CNodes.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/Actions/CSolveSystem.hpp"

#include "LinearProblem.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

struct LinearProblem::Implementation
{
  Implementation(Component& comp) :
   m_component(comp),
   m_solver(comp.create_static_component<CSolveSystem>("LSSSolveAction")),
   m_bc(comp.create_static_component<BoundaryConditions>("BoundaryConditions")),
   m_proxy(m_solver.option("lss"), m_component.option("physical_model")),
   system_matrix(m_proxy),
   system_rhs(m_proxy),
   dirichlet(m_proxy),
   solution(m_proxy)
  {
    m_solver.option("lss").link_to(&m_lss)->attach_trigger(boost::bind(&Implementation::trigger_lss, this));
  }
  
  void trigger_lss()
  {
    if(!m_lss.expired())
      m_bc.option("lss").change_value(m_lss.lock()->uri());
  }

  Component& m_component;
  CSolveSystem& m_solver;
  BoundaryConditions& m_bc;
  LSSProxy m_proxy;

  SystemMatrix system_matrix;
  SystemRHS system_rhs;
  DirichletBC dirichlet;
  SolutionVector solution;
  
  boost::weak_ptr<CEigenLSS> m_lss;
};

LinearProblem::LinearProblem(const std::string& name) :
  CProtoActionDirector(name),
  m_implementation( new Implementation(*this) ),
  system_matrix(m_implementation->system_matrix),
  system_rhs(m_implementation->system_rhs),
  dirichlet(m_implementation->dirichlet),
  solution(m_implementation->solution)
{
}

LinearProblem::~LinearProblem()
{
}

void LinearProblem::execute()
{
  if(m_implementation->m_lss.expired())
    throw SetupError(FromHere(), "Error executing " + uri().string() + ": Invalid LSS");
  
  m_implementation->m_lss.lock()->resize(physical_model().neqs() * physical_model().nb_nodes());
  m_implementation->m_lss.lock()->set_zero();
  CProtoActionDirector::execute();
}

CAction& LinearProblem::solve_action()
{
  return m_implementation->m_solver;
}

BoundaryConditions& LinearProblem::boundary_conditions()
{
  return m_implementation->m_bc;
}


} // UFEM
} // CF
