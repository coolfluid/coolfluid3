// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "sdm/lusgs/Solver.hpp"
#include "sdm/lusgs/LUSGS.hpp"

namespace cf3 {
namespace sdm {
namespace lusgs {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < sdm::lusgs::Solver, common::Action, LibLUSGS > lusgsSolver_Builder;

////////////////////////////////////////////////////////////////////////////////

Solver::Solver ( const std::string& name ) :
  sdm::Solver(name)
{
  options()["time_integration"]
      .attach_trigger( boost::bind( &Solver::configure_lusgs, this) );
  
  m_lusgs = create_static_component<LUSGS>("lusgs");
  m_lusgs->mark_basic();
}

///////////////////////////////////////////////////////////////////////////////////////

void Solver::configure_lusgs()
{
  m_lusgs->options().set("system",m_time_integration);
  m_lusgs->options().set("pre_update",m_pre_update);
  m_lusgs->options().set("post_update",m_post_update);
}

////////////////////////////////////////////////////////////////////////////////

void Solver::setup()
{
  m_lusgs->options().set("dict",m_dict);
}

///////////////////////////////////////////////////////////////////////////////////////

void Solver::step()
{
  m_lusgs->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // lusgs
} // sdm
} // cf3
