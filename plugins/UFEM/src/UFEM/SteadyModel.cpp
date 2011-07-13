// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CNodes.hpp"

#include "Physics/PhysModel.hpp"

#include "LinearProblem.hpp"
#include "SteadyModel.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Solver;

struct SteadyModel::Implementation
{
  Implementation(Component& comp) :
   m_component(comp),
   m_problem(m_component.create_static_component<LinearProblem>("LinearProblem"))
  {
    m_problem.configure_option("physical_model", m_component.option("physical_model").value());
    m_problem.configure_option("region", m_component.option("region").value());
  }
  
  Component& m_component;
  LinearProblem& m_problem;
};

SteadyModel::SteadyModel(const std::string& name) :
  Model(name),
  m_implementation( new Implementation(*this) )
{
}

SteadyModel::~SteadyModel()
{
}

LinearProblem& SteadyModel::problem()
{
  return m_implementation->m_problem;
}



} // UFEM
} // CF
