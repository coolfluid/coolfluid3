// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_uTests_TreeHandler_hpp
#define cf3_GUI_Client_uTests_TreeHandler_hpp

///////////////////////////////////////////////////////////////////////////

#include "UI/Core/CNode.hpp"

class QStringList;

///////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

  /////////////////////////////////////////////////////////////////////////

  class TreeHandler
  {
  public:

    ~TreeHandler();

    void add(cf3::UI::Core::CNode::Ptr node);

    void addChildren(cf3::UI::Core::CNode::Ptr node);

  private:

    QStringList names;

  }; // class TreeHandler

  /////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3

///////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Client_uTests_TreeHandler_hpp
