// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CNodes.hpp"

#include "Solver/CPhysicalModel.hpp"

#include "LinearProblem.hpp"
#include "TimeLoop.hpp"
#include "UnsteadyModel.hpp"

#include "Solver/Actions/Proto/Terminals.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Solver;

struct UnsteadyModel::Implementation
{
  Implementation(Component& comp) :
   m_component(comp),
   m_problem(m_component.create_static_component< TimeLoop<LinearProblem> >("TimeLoop")),
   m_time(m_component.create_static_component_ptr<CTime>("Time"))
  {
    m_problem.configure_option("physical_model", m_component.option("physical_model").value());
    m_problem.configure_option("region", m_component.option("region").value());
    m_problem.configure_option("time", m_time);
    m_time.lock()->option("time_step").attach_trigger(boost::bind(&Implementation::trigger_dt, this));
    trigger_dt();
  }
  
  /// Triggered when the timsestep is changed
  void trigger_dt()
  {
    m_invdt = 1. / m_time.lock()->dt();
  }
  
  Component& m_component;
  LinearProblem& m_problem;
  boost::weak_ptr<CTime> m_time;
  
  Real m_invdt;
};

UnsteadyModel::UnsteadyModel(const std::string& name) :
  Model(name),
  m_implementation( new Implementation(*this) )
{
}

UnsteadyModel::~UnsteadyModel()
{
}

LinearProblem& UnsteadyModel::problem()
{
  return m_implementation->m_problem;
}

CTime& UnsteadyModel::time()
{
  return *m_implementation->m_time.lock();
}

Actions::Proto::StoredReference< Real > UnsteadyModel::invdt() const
{
  return Actions::Proto::store(m_implementation->m_invdt);
}



} // UFEM
} // CF
