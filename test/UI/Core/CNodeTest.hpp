// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_uTests_CNodeTest_hpp
#define cf3_GUI_Client_uTests_CNodeTest_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "Common/StringConversion.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

//////////////////////////////////////////////////////////////////////////

class CNodeTest : public QObject
{
  Q_OBJECT

private slots:

  void test_getComponentType();

  void test_isClientComponent();

  void test_getType();

  void test_checkType();

  void test_setProperties();

  void test_setSignals();

  void test_modifyOptions();

  void test_listOptions();

  void test_listProperties();

  void test_createFromXml();

  void test_addNode();

  void test_removeNode();

  void test_listChildPaths();

}; // class CNodeTest

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Client_uTests_CNodeTest_hpp
