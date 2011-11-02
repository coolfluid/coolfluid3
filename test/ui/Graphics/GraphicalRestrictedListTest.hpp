// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Client_uTests_GraphicalRestrictedListTest_hpp
#define cf3_ui_Client_uTests_GraphicalRestrictedListTest_hpp

#include <QObject>

class QComboBox;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace graphics { class GraphicalRestrictedList; }

namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

class GraphicalRestrictedListTest : public QObject
{
  Q_OBJECT

private slots:

  void initTestCase();

  void test_constructor();

  void test_setValue();

  void test_value();

  void test_signalEmmitting();

  void test_valueString();

  void test_isModified();

  void test_setRestrictedList();

private:

  QComboBox * findComboBox(const graphics::GraphicalRestrictedList* value);

};

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Client_uTests_GraphicalRestrictedListTest_hpp
