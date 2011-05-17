// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_GraphicalArrayTest_hpp
#define CF_GUI_Client_uTests_GraphicalArrayTest_hpp

#include <QObject>

class QLineEdit;
class QListView;
class QPushButton;
class QStringListModel;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {

namespace Graphics { class GraphicalArray; }

namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

class GraphicalArrayTest : public QObject
{
  Q_OBJECT

private slots:

  void test_constructor();

  void test_setValidator();

  void test_setValue();

  void test_value();

  void test_signalEmmitting();

  void test_valueString();

  void test_isModified();

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
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_GraphicalArrayTest_hpp
