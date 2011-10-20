// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_uTests_NTreeTest_hpp
#define cf3_GUI_Client_uTests_NTreeTest_hpp

///////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "UI/Core/NRoot.hpp"

///////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

/////////////////////////////////////////////////////////////////////////

class NTreeTest : public QObject
{
  Q_OBJECT

private slots:

  void test_constructor();

  void test_setRoot();

  void test_setCurrentIndex();

  void test_currentPath();

  void test_nodePath();

  void test_pathFromIndex();

  void test_listNodeOptions();

  void test_setAdvancedMode();

  void test_areFromSameNode();

  void test_nodeByPath();

  void test_indexFromPath();

  void test_data();

  void test_index();

  void test_parent();

  void test_rowCount();

  void test_headerData();

  void test_setDebugModeEnabled();

  void test_optionsChanged();

  void test_nodeMatches();

  void test_signal_list_tree();

  void test_indexIsVisible();

}; // class NTreeTest

/////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3

///////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Client_uTests_NTreeTest_hpp
