// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/EventHandler.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "SurfaceIntegral.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < SurfaceIntegral, common::Action, LibUFEM > SurfaceIntegral_Builder;

SurfaceIntegral::SurfaceIntegral(const std::string& name) :
  ProtoAction(name)
{
  options().add("variable_name", "SomeScalar")
    .pretty_name("Variable Name")
    .description("Name of the variable to use")
    .attach_trigger(boost::bind(&SurfaceIntegral::trigger_set_expression, this))
    .link_to(&m_variable_name)
    .mark_basic();
 
  options().add("field_tag", "SomeFieldTag")
    .pretty_name("Field Tag")
    .description("Tag for the field to use")
    .attach_trigger(boost::bind(&SurfaceIntegral::trigger_set_expression, this))
    .mark_basic();
    
  options().add("history",m_history)
      .pretty_name("History")
      .description("History component used to log the history of the integral value.")
      .link_to(&m_history)
      .mark_basic();
}

SurfaceIntegral::~SurfaceIntegral()
{
}

void SurfaceIntegral::trigger_set_expression()
{
  using boost::proto::lit;
  
  const std::string var = m_variable_name;
  const std::string tag = options().option("field_tag").value<std::string>();
  
  FieldVariable<0, ScalarField> s(var, tag);
  
  set_expression(elements_expression(boost::mpl::vector3<mesh::LagrangeP1::Line2D, mesh::LagrangeP1::Quad3D, mesh::LagrangeP1::Triag3D>(),
    lit(m_integral_value) += integral<1>(s * normal)));
}

void SurfaceIntegral::execute()
{
  const Uint dim = m_physical_model->ndim();
  
  m_integral_value.resize(dim);
  m_integral_value.setZero();
  
  solver::actions::Proto::ProtoAction::execute();
  
  //TODO: Stop this from counting overlapping faces twice
  std::vector<Real> local_v(dim);
  for(Uint i = 0; i != dim; ++i)
    local_v[i] = m_integral_value[i];
  std::vector<Real> global_v(local_v.begin(), local_v.end());
  
  if(common::PE::Comm::instance().is_active())
  {
    common::PE::Comm::instance().all_reduce(common::PE::plus(), local_v, global_v);
  }
  
  if(is_not_null(m_history))
  {
    m_history->set(m_variable_name, global_v);
    m_history->save_entry();
  }
  else
  {
    CFwarn << "SurfaceIntegral component " << uri().path() << " has no history set." << CFendl;
  }
}


} // namespace UFEM

} // namespace cf3
