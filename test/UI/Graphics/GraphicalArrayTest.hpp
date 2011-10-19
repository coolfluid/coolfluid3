// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Client_uTests_GraphicalArrayTest_hpp
#define cf3_GUI_Client_uTests_GraphicalArrayTest_hpp

#include <QObject>

class QLineEdit;
class QListView;
class QPushButton;
class QStringListModel;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {

namespace Graphics { class GraphicalArray; }

namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

class GraphicalArrayTest : public QObject
{
  Q_OBJECT

private slots:

  void initTestCase();

  void test_constructor();

  void test_setValidator();

  void test_setValue();

  void test_value();

  void test_signalEmmitting();

  void test_valueString();

  void test_isModified();

  void test_removeItems();

private:

  QLineEdit * findLineEdit(const Graphics::GraphicalArray* value);

  QPushButton * findRemoveButton(const Graphics::GraphicalArray* value);

  QListView * findListView(const Graphics::GraphicalArray* value);

  QStringListModel * findModel(const Graphics::GraphicalArray* value);

  QWidget * findWidget(const Graphics::GraphicalArray* value, int row, int col);

};

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Client_uTests_GraphicalArrayTest_hpp
