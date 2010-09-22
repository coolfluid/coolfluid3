// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_TreeHandler_hpp
#define CF_GUI_Client_uTests_TreeHandler_hpp

///////////////////////////////////////////////////////////////////////////

#include "GUI/Client/Core/CNode.hpp"

class QStringList;

///////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientTest {

  /////////////////////////////////////////////////////////////////////////

  class TreeHandler
  {
  public:

    ~TreeHandler();

    void add(CF::GUI::Client::CNode::Ptr node);

    void addChildren(CF::GUI::Client::CNode::Ptr node);

  private:

    QStringList names;

  }; // class TreeHandler

  /////////////////////////////////////////////////////////////////////////

} // namespace ClientTest
} // namespace GUI
} // namespace CF

///////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_TreeHandler_hpp
