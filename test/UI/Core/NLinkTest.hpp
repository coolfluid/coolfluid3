// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_uTests_NLinkTest_hpp
#define cf3_GUI_Client_uTests_NLinkTest_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

//////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

//////////////////////////////////////////////////////////////////////////

  class NLinkTest : public QObject
  {
    Q_OBJECT

  private slots:

    void test_tootip();

    void test_targetPath();

    void test_goToTarget();

    void test_setTargetPath();

    void test_setTargetNode();

  }; // class NLinkTest

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3

//////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Client_uTests_NLinkTest_hpp
