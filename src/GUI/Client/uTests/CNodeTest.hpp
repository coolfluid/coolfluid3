// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_CNodeTest_hpp
#define CF_GUI_Client_uTests_CNodeTest_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientTest {

  //////////////////////////////////////////////////////////////////////////

  class CNodeTest : public QObject
  {
    Q_OBJECT

  private slots:

      void test_getComponentType();

      void test_isClientComponent();

      void test_getType();

      void test_checkType();

      void test_setOptions();

      void test_getOptions();

      void test_createFromXml();

      void test_addNode();

      void test_removeNode();

  }; // class CNodeTest

  //////////////////////////////////////////////////////////////////////////

} // ClientTest
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_CNodeTest_hpp
