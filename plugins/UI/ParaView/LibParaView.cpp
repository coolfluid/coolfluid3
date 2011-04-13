// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/RegistLibrary.hpp"

#include "UI/ParaView/C3DViewBuilder.hpp"
#include "UI/ParaView/LibParaView.hpp"

using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

  CF::Common::RegistLibrary<LibParaView> libParaView;

  ////////////////////////////////////////////////////////////////////////////////

  void LibParaView::initiate_impl()
  {
    Component::Ptr tools = Core::instance().root()->get_child_ptr("Tools");

    tools->create_component<C3DViewBuilder>("C3DViewBuilder")->mark_basic();
  }

  void LibParaView::terminate_impl()
  {

  }

  ////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////
