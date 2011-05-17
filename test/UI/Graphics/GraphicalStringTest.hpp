// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_GraphicalStringTest_hpp
#define CF_GUI_Client_uTests_GraphicalStringTest_hpp

#include <QObject>

class QLineEdit;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {

namespace Graphics { class GraphicalString; }

namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

class GraphicalStringTest : public QObject
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

  QLineEdit * findLineEdit(const Graphics::GraphicalString* value);

};

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_GraphicalStringTest_hpp
