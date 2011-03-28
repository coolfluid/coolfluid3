// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_CNodeTest_hpp
#define CF_GUI_Client_uTests_CNodeTest_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "Common/StringConversion.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
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

  void test_makeOption();

  void test_makeOptionTypes();

  void test_makeOptionUriSchemes();

  void test_makeOptionRestrictedLists();

  void test_makeOptionArrayTypes();

  void test_makeOptionArrayRestrictedLists();

}; // class CNodeTest

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_CNodeTest_hpp
