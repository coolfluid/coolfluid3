// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_GraphicalUriTest_hpp
#define CF_GUI_Client_uTests_GraphicalUriTest_hpp

#include <QObject>

class QComboBox;
class QLineEdit;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

namespace ClientUI { class GraphicalUri; }

namespace ClientTest {

//////////////////////////////////////////////////////////////////////////

class GraphicalUriTest : public QObject
{
  Q_OBJECT

private slots:

  void test_constructor();

  void test_setSchemes();

  void test_setValue();

  void test_value();

  void test_signalEmmitting();

  void test_valueString();

  void test_isModified();

private:

  QLineEdit * findLineEdit(const ClientUI::GraphicalUri* value);

  QComboBox * findComboBox(const ClientUI::GraphicalUri* value);

  QWidget * findWidget(const ClientUI::GraphicalUri* value, int index);

};

//////////////////////////////////////////////////////////////////////////

} // ClientTest
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_GraphicalUriTest_hpp
