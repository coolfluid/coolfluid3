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

#include "Mesh/CDomain.hpp"
#include "Mesh/FieldManager.hpp"
#include "Mesh/Geometry.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/Tags.hpp"
#include "Solver/Actions/CSolveSystem.hpp"

#include "Physics/PhysModel.hpp"

#include "LinearSolver.hpp"
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
   system_matrix(m_solver.option("lss")),
   system_rhs(m_solver.option("lss")),
   dirichlet(m_solver.option("lss")),
   solution(m_solver.option("lss"))
  {
    m_solver.option("lss").attach_trigger(boost::bind(&Implementation::trigger_lss, this));
  }
  
  void trigger_lss()
  {
    m_lss = dynamic_cast<OptionComponent<CEigenLSS>&>(m_solver.option("lss")).component().as_ptr<CEigenLSS>();
    if(m_lss.expired())
    {
      CFdebug << "Triggering on bad LSS in " << m_solver.uri().string() << CFendl;
      return;
    }

    m_bc.option("lss").change_value(m_lss.lock()->uri());
    m_zero_action.m_lss = m_lss;
  }

  Component& m_component;
  CSolveSystem& m_solver;
  BoundaryConditions& m_bc;
  ZeroAction& m_zero_action;

  SystemMatrix system_matrix;
  SystemRHS system_rhs;
  DirichletBC dirichlet;
  SolutionVector solution;
  
  boost::weak_ptr<CEigenLSS> m_lss;
};

LinearSolver::LinearSolver(const std::string& name) :
  CSimpleSolver(name),
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
  
  VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physics().variable_manager(), UFEM::Tags::solution());
  
  m_implementation->m_lss.lock()->resize(descriptor.size() * mesh().topology().geometry().size());
  CSimpleSolver::execute();
}

void LinearSolver::mesh_loaded(CMesh& mesh)
{
  CSimpleSolver::mesh_loaded(mesh);
  
  // Create fields using the known tags
  field_manager().create_field(Tags::solution(), mesh.geometry());
  field_manager().create_field(Tags::source_terms(), mesh.geometry());
  
  // Set the region of all children to the root region of the mesh
  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  configure_option_recursively(Solver::Tags::regions(), root_regions);
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
