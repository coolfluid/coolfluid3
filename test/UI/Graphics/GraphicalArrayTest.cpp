// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QStringListModel>
#include <QPushButton>

#include <QtTest>

#include "UI/Graphics/GraphicalArray.hpp"

#include "test/UI/Core/ExceptionThrowHandler.hpp"

#include "test/UI/Graphics/GraphicalArrayTest.hpp"

using namespace CF::Common;
using namespace CF::UI::Graphics;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::initTestCase()
{
  GraphicalArray * value = new GraphicalArray();

  QVERIFY( is_not_null( findLineEdit(value) ) );
  QVERIFY( is_not_null( findRemoveButton(value) ) );
  QVERIFY( is_not_null( findListView(value) ) );
  QVERIFY( is_not_null( findModel(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::test_constructor()
{
  GraphicalArray * value = new GraphicalArray();
  QStringListModel * model = findModel(value);

  // 1. value is empty, the line edit should be empty as well
  QVERIFY( is_not_null(model) );
  QVERIFY( model->stringList().empty() );

  delete value;

  QIntValidator * validator = new QIntValidator();
  value = new GraphicalArray( validator );
  QLineEdit * lineEdit = findLineEdit(value);

  // 2. validator objects should be the same
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->validator(), validator );

  delete value;
  delete validator;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::test_setValidator()
{
  QValidator * validator = new QIntValidator();
  GraphicalArray * value = new GraphicalArray( validator );
  QLineEdit * lineEdit = findLineEdit(value);

  QVERIFY( is_not_null(lineEdit) );

  // 1. validator objects should be the same
  QCOMPARE( lineEdit->validator(), validator );

  // 2. try to set a null validator
  value->setValidator(nullptr);
  QCOMPARE( lineEdit->validator(), (QValidator*) nullptr );

  delete validator;
  validator = new QDoubleValidator(nullptr);

  // 3. set a new validator
  value->setValidator(validator);
  QCOMPARE( lineEdit->validator(), validator );


  delete validator;
  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::test_setValue()
{
  QString sep(";");
  GraphicalArray * value = new GraphicalArray( new QIntValidator(), sep );
  QLineEdit * lineEdit = findLineEdit(value);
  QStringListModel * model = findModel(value);

  QStringList validList;
  QStringList invalidList;

  validList << "42" << "15465" << "-145314" << "42"; // duplicates should be kept
  invalidList << "25" << "1" << "something" << "32" << "3.14" << "-54789";

  QVERIFY( is_not_null(lineEdit) );

  //
  // 1. check with strings
  //
  // 1a. list that only contains valid values
  QVERIFY( value->setValue( validList.join(sep) ) );
  QCOMPARE( model->stringList(), validList );

  // 1c. only one value
  QVERIFY( value->setValue( QString("1456789") ) );
  QCOMPARE( model->stringList().count(), 1 );
  QCOMPARE( model->stringList().at(0), QString("1456789") );

  // 1b. list that contains some invalid values
  QVERIFY( !value->setValue( invalidList.join(sep) ) );
  QStringList list = model->stringList();

  // "something" and "3.14" are invalid integers and so the value shouldn't have changed
  QCOMPARE( model->stringList().count(), 1 );
  QCOMPARE( model->stringList().at(0), QString("1456789") );

  //
  // 2. check with other types
  //
  QVERIFY( !value->setValue(true) );
  QCOMPARE( model->stringList().at(0), QString("1456789") );

  QVERIFY( !value->setValue(3.145) );
  QCOMPARE( model->stringList().at(0), QString("1456789") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::test_value()
{
  GraphicalArray * value = new GraphicalArray();
  QStringListModel * model = findModel(value);
  QStringList stringList;
  QVariant theValue;

  stringList << "hello" << "world" << "!";

  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::StringList );
  QCOMPARE( theValue.toStringList(), QStringList() );

  model->setStringList( stringList );

  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::StringList );
  QCOMPARE( theValue.toStringList(), stringList );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::test_signalEmmitting()
{
  GraphicalArray * value = new GraphicalArray();
  QLineEdit * lineEdit = findLineEdit(value);
  QStringListModel * model = findModel(value);
  QSignalSpy spy(value, SIGNAL(valueChanged()));

  //
  // 1. through setValue()
  //
  value->setValue( QString("Hello World") ); // when the value is a string
  value->setValue( QStringList() << "Hello" << "World" ); // when the value is a string list
  value->setValue( 42 ); // when the value is not valid (so signal emitted)

  // 2 signals should have been emitted
  QCOMPARE( spy.count(), 2 );

  spy.clear();

  // clear the model
  model->setStringList( QStringList() );

  //
  // 2. by simulating keyboard events
  //
  // note: when validating, we wait 25 ms (last parameter) to let the event being processed
//  lineEdit->clear();
//  value->show(); // make the value visible (it ignores keyboard events if not)
//  lineEdit->setFocus(Qt::PopupFocusReason);
//  QTest::keyClicks(lineEdit, "123" );
//  QTest::keyClick(value, Qt::Key_Enter, Qt::NoModifier); // validate by pressing the 'ENTER' key on the keypad
//  QTest::keyClicks(lineEdit, "156" );
//  QTest::keyClicks(lineEdit, "456" );
//  QTest::keyClick(value, Qt::Key_Return, Qt::NoModifier); // validate by pressing the 'Return' key

  // 2 signals should have been emitted (one per validation)
//  QCOMPARE( spy.count(), 2 );

//  QCOMPARE( model->stringList(), QStringList() << "123" << "156456");
//  QCOMPARE( lineEdit->text(), QString() );

  spy.clear();
  //
  // 3. when committing
  //
  value->commit();

  // 1 signal should have been emitted
  QCOMPARE( spy.count(), 1 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::test_valueString()
{
  GraphicalArray * value = new GraphicalArray();

  value->setValue( QString("Hello") );
  QCOMPARE( value->valueString(), QString("Hello") );

  value->setValue( QStringList() << "Hello" << "World");
  QCOMPARE( value->valueString(), QString("Hello@@World") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::test_isModified()
{
  GraphicalArray * value = new GraphicalArray();
  QStringListModel * model = findModel(value);

  // 1. initially, it's not modified
  QVERIFY( !value->isModified() );

  // 2. change the value
  model->setStringList( QStringList() << "Hello" << "World" );
  QVERIFY( value->isModified() );

  // 3. change the value and commit
  model->setStringList( QStringList() << "Hello" << "World" << "and" << "Happy" << "New" << "Year!" );
  QVERIFY( value->isModified() );
  value->commit();
  QVERIFY( !value->isModified() );

  // 4. set the same value
  model->setStringList( QStringList() << "Hello" << "World" << "and" << "Happy" << "New" << "Year!" );
  QVERIFY( !value->isModified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalArrayTest::test_removeItems()
{
  GraphicalArray * value = new GraphicalArray();
  QPushButton * button = findRemoveButton(value);
  QStringListModel * model = findModel(value);
  QListView * view = findListView(value);
  QItemSelectionModel::SelectionFlags flag = QItemSelectionModel::Select;

  model->setStringList( QStringList() << "Hello" << "World" << "and" << "Happy" << "New" << "Year!" );
  value->show();

  view->selectionModel()->select( model->index(0), flag ); // select "Hello"
  view->selectionModel()->select( model->index(1), flag ); // select "World"
  view->selectionModel()->select( model->index(3), flag ); // select "Happy"
  view->selectionModel()->select( model->index(5), flag ); // select "Year"

  // simulate a click on the 'Remove' button
  QTest::mouseClick( button, Qt::LeftButton );

  QStringList values = model->stringList();

  QCOMPARE( values.count(), 2 );
  QCOMPARE( values.at(0), QString("and") );
  QCOMPARE( values.at(1), QString("New") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

QLineEdit * GraphicalArrayTest::findLineEdit(const GraphicalArray* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalArray

  QLineEdit * lineEdit = dynamic_cast<QLineEdit*>(findWidget(value, 0, 0));

  if( is_null(lineEdit) )
    QWARN("Failed to find the line edit.");

  return lineEdit;
}

//////////////////////////////////////////////////////////////////////////

QPushButton * GraphicalArrayTest::findRemoveButton(const GraphicalArray* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalArray

  QPushButton * button = dynamic_cast<QPushButton*>(findWidget(value, 1, 1));

  if( is_null(button) )
    QWARN("Failed to find the remove button.");

  return button;
}

//////////////////////////////////////////////////////////////////////////


QListView * GraphicalArrayTest::findListView(const GraphicalArray* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalArray

  QListView * listView = dynamic_cast<QListView*>(findWidget(value, 1, 0));

  if( is_null(listView) )
    QWARN("Failed to find the list.");

  return listView;
}

//////////////////////////////////////////////////////////////////////////

QStringListModel * GraphicalArrayTest::findModel(const GraphicalArray* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalArray

  QListView * view = findListView(value);
  QStringListModel * model = nullptr;

  if( is_not_null(view) )
  {
    model = dynamic_cast<QStringListModel*>( view->model() );

    if( is_null(model) )
      QWARN("Failed to find the view model.");
  }

  return model;
}


//////////////////////////////////////////////////////////////////////////

QWidget * GraphicalArrayTest::findWidget(const GraphicalArray* value, int row, int col)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalArray

  QGridLayout * layout = dynamic_cast<QGridLayout*>(value->layout()->itemAt(0)->widget()->layout());
  QWidget * widget = nullptr;

  if( is_not_null(layout) )
    widget = layout->itemAtPosition(row, col)->widget();
  else
    QWARN("Failed to find the layout or cast it to QGridLayout.");

  return widget;
}

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // UI
} // CF
