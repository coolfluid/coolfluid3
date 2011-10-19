// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_uTests_TreeNodeTest_hpp
#define cf3_GUI_Client_uTests_TreeNodeTest_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

//////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

//////////////////////////////////////////////////////////////////////////

class TreeNodeTest : public QObject
{
  Q_OBJECT

private slots:

  void test_constructor();

  void test_hasParent();

  void test_child();

  void test_childByName();

  void test_updateChildList();

}; // TreeNodeTest

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3

//////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Client_uTests_TreeNodeTest_hpp
