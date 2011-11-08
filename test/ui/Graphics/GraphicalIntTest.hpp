// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Client_uTests_GraphicalIntTest_hpp
#define cf3_ui_Client_uTests_GraphicalIntTest_hpp

#include <QObject>

class QDoubleSpinBox;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace graphics { class GraphicalInt; }

namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

class GraphicalIntTest : public QObject
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

private:

  QDoubleSpinBox * findSpinBox(const graphics::GraphicalInt* value);

};

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Client_uTests_GraphicalIntTest_hpp
