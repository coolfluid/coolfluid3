// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_uTests_CommitDetailsTest_hpp
#define cf3_GUI_Client_uTests_CommitDetailsTest_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

//////////////////////////////////////////////////////////////////////////

class CommitDetailsTest : public QObject
{
  Q_OBJECT

private slots:

    void test_data();

    void test_headerData();

    void test_index();

    void test_setOption();

    void test_clear();
};

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Client_uTests_CommitDetailsTest_hpp
