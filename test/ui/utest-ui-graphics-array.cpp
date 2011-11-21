// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the GUI GraphicalArray class"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QSignalSpy>
#include <QStringListModel>
#include <QTest>

#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/StringConversion.hpp"
#include "common/URI.hpp"

#include "ui/graphics/GraphicalArray.hpp"

#include "test/ui/Application.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////

QWidget * find_widget(const GraphicalArray* value, int row, int col)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // TO THE GRAPHICAL LOOK OF GraphicalArray

  QGridLayout * layout = dynamic_cast<QGridLayout*>(value->layout()->itemAt(0)->widget()->layout());
  QWidget * widget = nullptr;

  if( is_not_null(layout) )
    widget = layout->itemAtPosition(row, col)->widget();
  else
    std::cerr << "Failed to find the layout or cast it to QGridLayout." << std::endl;

  return widget;
}

//////////////////////////////////////////////////////////////////////////////

QLineEdit * find_line_edit(const GraphicalArray* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // TO THE GRAPHICAL LOOK OF GraphicalArray

  QLineEdit * lineEdit = dynamic_cast<QLineEdit*>(find_widget(value, 0, 0));

  if( is_null(lineEdit) )
    std::cerr << "Failed to find the line edit." << std::endl;

  return lineEdit;
}

//////////////////////////////////////////////////////////////////////////

QPushButton * find_remove_button(const GraphicalArray* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // TO THE GRAPHICAL LOOK OF GraphicalArray

  QPushButton * button = dynamic_cast<QPushButton*>(find_widget(value, 1, 1));

  if( is_null(button) )
    std::cerr << "Failed to find the remove button." << std::endl;

  return button;
}

//////////////////////////////////////////////////////////////////////////


QListView * find_list_view(const GraphicalArray* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // TO THE GRAPHICAL LOOK OF GraphicalArray

  QListView * listView = dynamic_cast<QListView*>(find_widget(value, 1, 0));

  if( is_null(listView) )
    std::cerr << "Failed to find the list." << std::endl;

  return listView;
}

//////////////////////////////////////////////////////////////////////////

QStringListModel * find_model(const GraphicalArray* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // TO THE GRAPHICAL LOOK OF GraphicalArray

  QListView * view = find_list_view(value);
  QStringListModel * model = nullptr;

  if( is_not_null(view) )
  {
    model = dynamic_cast<QStringListModel*>( view->model() );

    if( is_null(model) )
      std::cerr << "Failed to find the view model." << std::endl;
  }

  return model;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiGraphicsGraphicalArraySuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();
  GraphicalArray * value = new GraphicalArray();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  // check the find_* functions are valid with the current Graphical arrangement
  BOOST_REQUIRE( is_not_null( find_line_edit(value) ) );
  BOOST_REQUIRE( is_not_null( find_remove_button(value) ) );
  BOOST_REQUIRE( is_not_null( find_list_view(value) ) );
  BOOST_REQUIRE( is_not_null( find_model(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  GraphicalArray * value = new GraphicalArray();
  QStringListModel * model = find_model(value);

  // 1. value is empty, the line edit should be empty as well
  BOOST_CHECK( is_not_null(model) );
  BOOST_CHECK( model->stringList().empty() );

  delete value;

  QIntValidator * validator = new QIntValidator();
  value = new GraphicalArray( validator );
  QLineEdit * lineEdit = find_line_edit(value);

  // 2. validator objects should be the same
  BOOST_CHECK( is_not_null(lineEdit) );
  BOOST_CHECK_EQUAL( lineEdit->validator(), validator );

  delete value;
  delete validator;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_validator )
{
  QValidator * validator = new QIntValidator();
  GraphicalArray * value = new GraphicalArray( validator );
  QLineEdit * lineEdit = find_line_edit(value);

  BOOST_CHECK( is_not_null(lineEdit) );

  // 1. validator objects should be the same
  BOOST_CHECK_EQUAL( lineEdit->validator(), validator );

  // 2. try to set a null validator
  value->set_validator(nullptr);
  BOOST_CHECK_EQUAL( lineEdit->validator(), (QValidator*) nullptr );

  delete validator;
  validator = new QDoubleValidator(nullptr);

  // 3. set a new validator
  value->set_validator(validator);
  BOOST_CHECK_EQUAL( lineEdit->validator(), validator );


  delete validator;
  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_value )
{
  QString sep(";");
  GraphicalArray * value = new GraphicalArray( new QIntValidator(), sep );
  QLineEdit * lineEdit = find_line_edit(value);
  QStringListModel * model = find_model(value);

  QStringList validList;
  QStringList invalidList;
  QStringList list;

  validList << "42" << "15465" << "-145314" << "42"; // duplicates should be kept
  invalidList << "25" << "1" << "something" << "32" << "3.14" << "-54789";

  BOOST_CHECK( is_not_null(lineEdit) );

  //
  // 1. check with strings
  //
  // 1a. list that only contains valid values
  BOOST_CHECK( value->set_value( validList.join(sep) ) );
//  BOOST_CHECK_EQUAL( model->stringList(), validList );
  list = model->stringList();

  BOOST_CHECK_EQUAL( list.count(), validList.count() );

  for( int i = 0 ; i < list.count() ; ++i)
    BOOST_CHECK_EQUAL( list.at(i).toStdString(), validList.at(i).toStdString() );

  // 1c. only one value
  BOOST_CHECK( value->set_value( QString("1456789") ) );
  BOOST_CHECK_EQUAL( model->stringList().count(), 1 );
  BOOST_CHECK_EQUAL( model->stringList().at(0).toStdString(), std::string("1456789") );

  // 1b. list that contains some invalid values
  BOOST_CHECK( !value->set_value( invalidList.join(sep) ) );
  list = model->stringList();

  // "something" and "3.14" are invalid integers and so the value shouldn't have changed
  BOOST_CHECK_EQUAL( model->stringList().count(), 1 );
  BOOST_CHECK_EQUAL( model->stringList().at(0).toStdString(), std::string("1456789") );

  //
  // 2. check with other types
  //
  BOOST_CHECK( !value->set_value(true) );
  BOOST_CHECK_EQUAL( model->stringList().at(0).toStdString(), std::string("1456789") );

  BOOST_CHECK( !value->set_value(3.145) );
  BOOST_CHECK_EQUAL( model->stringList().at(0).toStdString(), std::string("1456789") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value )
{
  GraphicalArray * value = new GraphicalArray();
  QStringListModel * model = find_model(value);
  QStringList stringList;
  QVariant theValue;
  QStringList list;

  stringList << "hello" << "world" << "!";

  // the list should be empty
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::StringList );

  list = theValue.toStringList();

  BOOST_CHECK_EQUAL( list.count(), 0 );

  for( int i = 0 ; i < list.count() ; ++i)
    BOOST_CHECK_EQUAL( list.at(i).toStdString(), stringList.at(i).toStdString() );

  // set a stringlist, the list should have 3 items
  model->setStringList( stringList );

  theValue = value->value();

  list = theValue.toStringList();

  BOOST_CHECK( theValue.type() == QVariant::StringList );

  BOOST_CHECK_EQUAL( list.count(), stringList.count() );

  for( int i = 0 ; i < list.count() ; ++i)
    BOOST_CHECK_EQUAL( list.at(i).toStdString(), stringList.at(i).toStdString() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( signal_emitting )
{
  GraphicalArray * value = new GraphicalArray();
  QLineEdit * lineEdit = find_line_edit(value);
  QStringListModel * model = find_model(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. through setValue()
  //
  value->set_value( QString("Hello World") ); // when the value is a string
  value->set_value( QStringList() << "Hello" << "World" ); // when the value is a string list
  value->set_value( 42 ); // when the value is not valid (no signal emitted)

  // 2 signals should have been emitted
  BOOST_CHECK_EQUAL( spy.count(), 2 );

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
//  BOOST_CHECK_EQUAL( spy.count(), 2 );

//  BOOST_CHECK_EQUAL( model->stringList().toStdString(), std::stringList() << "123" << "156456");
//  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string() );

  spy.clear();
  //
  // 3. when committing
  //
  value->commit();

  // 1 signal should have been emitted
  BOOST_CHECK_EQUAL( spy.count(), 1 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value_string )
{
  GraphicalArray * value = new GraphicalArray(nullptr, ";");

  value->set_value( QString("Hello") );
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("Hello") );

  value->set_value( QStringList() << "Hello" << "World");
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("Hello;World") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_modified )
{
  GraphicalArray * value = new GraphicalArray();
  QStringListModel * model = find_model(value);

  // 1. initially, it's not modified
  BOOST_CHECK( !value->is_modified() );

  // 2. change the value
  model->setStringList( QStringList() << "Hello" << "World" );
  BOOST_CHECK( value->is_modified() );

  // 3. change the value and commit
  model->setStringList( QStringList() << "Hello" << "World" << "and" << "Happy" << "New" << "Year!" );
  BOOST_CHECK( value->is_modified() );
  value->commit();
  BOOST_CHECK( !value->is_modified() );

  // 4. set the same value
  model->setStringList( QStringList() << "Hello" << "World" << "and" << "Happy" << "New" << "Year!" );
  BOOST_CHECK( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( remove_items )
{
  GraphicalArray * value = new GraphicalArray();
  QPushButton * button = find_remove_button(value);
  QStringListModel * model = find_model(value);
  QListView * view = find_list_view(value);
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

  BOOST_CHECK_EQUAL( values.count(), 2 );
  BOOST_CHECK_EQUAL( values.at(0).toStdString(), std::string("and") );
  BOOST_CHECK_EQUAL( values.at(1).toStdString(), std::string("New") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
