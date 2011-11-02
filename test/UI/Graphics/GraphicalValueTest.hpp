// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Client_uTests_GraphicalValueTest_hpp
#define cf3_ui_Client_uTests_GraphicalValueTest_hpp

#include <QObject>

class QValidator;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace graphics { class GraphicalArray; }

namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

class GraphicalValueTest : public QObject
{
  Q_OBJECT

private slots:

  void test_createFromOption();

  void test_createFromOptionArray();

  void test_createFromOptionRestrValues();

  void test_createFromOptionArrayRestrValues();

private:

  const QValidator * arrayValidator(const graphics::GraphicalArray* array);

}; // GraphicalValueTest

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Client_uTests_GraphicalValueTest_hpp
