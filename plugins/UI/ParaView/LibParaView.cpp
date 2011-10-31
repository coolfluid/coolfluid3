// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//header
#include "common/Core.hpp"
#include "common/RegistLibrary.hpp"
#include "UI/ParaView/C3DViewBuilder.hpp"
#include "UI/ParaView/LibParaView.hpp"

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace ParaView {

cf3::common::RegistLibrary<LibParaView> libParaView;

////////////////////////////////////////////////////////////////////////////////

void LibParaView::initiate()
{
  if(m_is_initiated)
    return;
  
  initiate_impl();
  m_is_initiated = true;
}


void LibParaView::initiate_impl()
{
  Component::Ptr tools = Core::instance().root().get_child_ptr("Tools");

  tools->create_component_ptr<C3DViewBuilder>("C3DViewBuilder")->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////
