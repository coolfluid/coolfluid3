// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_GUI_Client_uTests_PropertyModelTest_hpp
#define CF_GUI_Client_uTests_PropertyModelTest_hpp

#include <QObject>

///////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace CoreTest {

/////////////////////////////////////////////////////////////////////////

class PropertyModelTest : public QObject
{
  Q_OBJECT

private slots:

  // called in first by Qt to initialize the testing environment
  void initTestCase();

  void test_dataFunction();

  void test_index();

  void test_rowCount();

  void test_headerData();

  // called in the last by Qt to clean the testing environment.
  void cleanupTestCase();

}; // PropertyModelTest

/////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // CF

/////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_PropertyModelTest_hpp
