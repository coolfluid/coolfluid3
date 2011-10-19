// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_uTests_NRootTest_hpp
#define cf3_GUI_Client_uTests_NRootTest_hpp

#include <QObject>

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

////////////////////////////////////////////////////////////////////////////////

class NRootTest : public QObject
{
  Q_OBJECT

private slots:

  void test_childFromRoot();

}; // class NRootTest

///////////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3

#endif // cf3_GUI_Client_uTests_NRootTest_hpp
