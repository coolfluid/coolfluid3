// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "solver/Time.hpp"
#include "CriterionConvergence.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;

using namespace solver;
using namespace solver::actions;

using namespace solver::actions::Proto;
using namespace boost::proto;

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< CriterionConvergence, Criterion, LibUFEM > CriterionConvergence_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CriterionConvergence::CriterionConvergence( const std::string& name  ) :
  Criterion ( name )
{
  // properties

  properties()["brief"] = std::string("Convergence Criterion object");
  std::string description =
      properties().value<std::string>("description")+
      "Returns true if a criterion for  iterations is achived\n";
  properties()["description"] = description;

  // options

  options().add("iterator", m_iter_comp)
      .description("Component performing iterations")
      .pretty_name("Iterative component")
      .link_to(&m_iter_comp).mark_basic();

  options().add("my_maxiter", m_max_iteration)
      .description("Maximum number of iterations (0 will perform none)")
      .pretty_name("Maximum number")
      .link_to(&m_max_iteration).mark_basic();

  FieldVariable<0, ScalarField> conduction_temperature("Temperature", "heat_conduction_solution");
  FieldVariable<1, ScalarField> Phi("Temperature", "scalar_advection_solution");

add_component(create_proto_action("ComputeMinError", nodes_expression(
          lit(m_min_error) = _min(_abs(Phi - conduction_temperature), m_min_error)
       )));
add_component(create_proto_action("ComputeMaxError", nodes_expression(
        lit(m_max_error) = _max(_abs(Phi - conduction_temperature), m_max_error)
     )));
add_component(create_proto_action("GetMaxCondTemperature", nodes_expression(
        lit(m_cond_temperature) = _max(conduction_temperature, m_cond_temperature)
     )));
add_component(create_proto_action("GetMaxFluidTemperature", nodes_expression (
        lit(m_fluid_temperature) = _max(Phi, m_fluid_temperature)
     )));
}

CriterionConvergence::~CriterionConvergence() {}

bool CriterionConvergence::operator()()
{

  /*std::ofstream convergence_history;

    convergence_history.open ("convergence_history_temperature.txt",std::ios_base::app);
    convergence_history << m_max_error << "\n"; */

  m_min_error = 0.;
  m_max_error = 0.;
  m_cond_temperature = 0.;
  m_fluid_temperature = 0.;

  Handle<Iterate> iterate(m_iter_comp);


  Handle<common::Action>(get_child("ComputeMinError"))->execute();
  Handle<common::Action>(get_child("ComputeMaxError"))->execute();
  Handle<common::Action>(get_child("GetMaxFluidTemperature"))->execute();
  Handle<common::Action>(get_child("GetMaxCondTemperature"))->execute();
 /* std::cout << "min error is " << m_min_error << std::endl;
  std::cout << "max error is " << m_max_error << std::endl;
  std::cout << "max conduction temperature is " << m_cond_temperature << std::endl;
  std::cout << "max fluid temperature is " << m_fluid_temperature << std::endl;*/
  if (m_max_error <= 10.e-1 && iterate->iter() > 1)
  {
      return true;
  }
  return false;

}


////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3
