// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Client_uTests_GraphicalUriTest_hpp
#define cf3_ui_Client_uTests_GraphicalUriTest_hpp

#include <QObject>

class QComboBox;
class QLineEdit;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace graphics { class GraphicalUri; }

namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

class GraphicalUriTest : public QObject
{
  Q_OBJECT

private slots:

  void initTestCase();

  void test_constructor();

  void test_setSchemes();

  void test_setValue();

  void test_value();

  void test_signalEmmitting();

  void test_valueString();

  void test_isModified();

private:

  QLineEdit * findLineEdit(const graphics::GraphicalUri* value);

  QComboBox * findComboBox(const graphics::GraphicalUri* value);

  QWidget * findWidget(const graphics::GraphicalUri* value, int index);

};

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Client_uTests_GraphicalUriTest_hpp
