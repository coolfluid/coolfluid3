// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.



#include "LSSProxy.hpp"

/// @file
/// Proxy for a linear system, providing access to the linear system solver and physical model components when needed

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

using namespace Common;
  
LSSProxy::LSSProxy(CEigenLSS& lss, Physics::PhysModel& physical_model) :
  m_lss(&lss),
  m_physical_model(&physical_model)
{
}

LSSProxy::LSSProxy(Common::Option& lss_option, Common::Option& physical_model_option) :
  m_lss(0),
  m_physical_model(0)
{
  // Store links to the options
  m_lss_option = boost::dynamic_pointer_cast< OptionComponent<CEigenLSS> >( lss_option.shared_from_this() );
  m_physical_model_option = boost::dynamic_pointer_cast< OptionComponent<Physics::PhysModel> >( physical_model_option.shared_from_this() );
  
  // Attach the triggers
  lss_option.attach_trigger( boost::bind(&LSSProxy::trigger_lss, this) );
  physical_model_option.attach_trigger( boost::bind(&LSSProxy::trigger_physical_model, this) );
  
  // execute the triggers to get the initial value, if any
  trigger_lss();
  trigger_physical_model();
}

// LSSProxy::LSSProxy(const LSSProxy& other) :
//   m_lss_option(other.m_lss_option),
//   m_physical_model_option(other.m_physical_model_option),
//   m_lss(other.m_lss),
//   m_physical_model(other.m_physical_model)
// {
//   if(!(m_lss_option.expired() || m_physical_model_option.expired()))
//   {
//     m_lss_option.lock()->attach_trigger( boost::bind(&LSSProxy::trigger_lss, this) );
//     m_physical_model_option.lock()->attach_trigger( boost::bind(&LSSProxy::trigger_physical_model, this) );
//     
//     trigger_lss();
//     trigger_physical_model();
//   }
// }

CEigenLSS& LSSProxy::lss()
{
  return *m_lss;
}

Physics::PhysModel& LSSProxy::physical_model()
{
  return *m_physical_model;
}


void LSSProxy::trigger_lss()
{
  if(m_lss_option.lock()->check())
    m_lss = &m_lss_option.lock()->component();
}

void LSSProxy::trigger_physical_model()
{
  if(m_physical_model_option.lock()->check())
    m_physical_model = &m_physical_model_option.lock()->component();
}





} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF
