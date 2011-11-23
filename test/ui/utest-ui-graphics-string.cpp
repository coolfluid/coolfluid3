// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the GUI GraphicalString class"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QSignalSpy>
#include <QTest>

#include "math/Consts.hpp"

#include "ui/graphics/GraphicalString.hpp"

#include "test/ui/Application.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////

QLineEdit * find_line_edit(const GraphicalString* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalString

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QLineEdit * lineEdit = nullptr;

  if( is_not_null(layout) )
  {
    lineEdit = dynamic_cast<QLineEdit*>(layout->itemAt(0)->widget());

    if( is_null(lineEdit) )
      QWARN("Failed to find the line edit.");
  }
  else
    QWARN("Failed to find the layout or cast it to QHBoxLayout.");

  return lineEdit;
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiGraphicsGraphicalBoolSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();
  GraphicalString * value = new GraphicalString();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  BOOST_CHECK( is_not_null( find_line_edit(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  GraphicalString * value = new GraphicalString();
  QLineEdit * lineEdit = find_line_edit(value);

  // 1. value is empty, the line edit should be empty as well
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->text().toStdString(), std::string() );

  delete value;
  value = new GraphicalString("Hello, World!");
  lineEdit = find_line_edit(value);

  // 2. value is not empty
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->text().toStdString(), std::string("Hello, World!") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_value )
{
  GraphicalString * value = new GraphicalString();
  QLineEdit * lineEdit = find_line_edit(value);

  QVERIFY( is_not_null(lineEdit) );

  //
  // 1. check with strings
  //
  QVERIFY( value->set_value("Hello") );
  QCOMPARE( lineEdit->text().toStdString(), std::string("Hello") );

  QVERIFY( value->set_value("World") );
  QCOMPARE( lineEdit->text().toStdString(), std::string("World") );

  //
  // 2. check with other types (this works since all primitive
  // types can be implicitly converted to QString by Qt)
  //
  QVERIFY( value->set_value(12) );
  QCOMPARE( lineEdit->text().toStdString(), std::string("12") );

  QVERIFY( value->set_value(3.141592) );
  QCOMPARE( lineEdit->text().toStdString(), std::string("3.141592") );

  QVERIFY( value->set_value(true) );
  QCOMPARE( lineEdit->text().toStdString(), std::string("true") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value )
{
  GraphicalString * value = new GraphicalString();
  QLineEdit * lineEdit = find_line_edit(value);
  QVariant theValue;

  // get value when the line edit is empty
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString().toStdString(), std::string() );

  // get value when the line edit has a string
  lineEdit->setText("This is a sample text.");
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString().toStdString(), std::string("This is a sample text.") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( signal_emitting )
{
  GraphicalString * value = new GraphicalString();
  QLineEdit * lineEdit = find_line_edit(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. through setValue()
  //
  value->set_value("Hello");
  value->set_value("World!");

  // 2 signals should have been emitted
  QCOMPARE( spy.count(), 2 );

  spy.clear();

  //
  // 2. by simulating keyboard events
  //
  value->show(); // make the value visible (it ignores keyboard events if not)
  QTest::keyClicks(lineEdit, "Hello " );
  QTest::keyClicks(lineEdit, "World " );
  QTest::keyClicks(lineEdit, "And others ;)" );

  // 25 signals should have been emitted (one per character)
  QCOMPARE( spy.count(), 25 );

  spy.clear();
  //
  // 3. when committing
  //
  value->commit();

  // 1 signal should have been emitted
  QCOMPARE( spy.count(), 1 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value_string )
{
  GraphicalString * value = new GraphicalString();

  value->set_value("Hello");
  QCOMPARE( value->value_string().toStdString(), std::string("Hello") );

  value->set_value("World");
  QCOMPARE( value->value_string().toStdString(), std::string("World") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_modified )
{
  GraphicalString * value = new GraphicalString();
  QLineEdit* lineEdit = find_line_edit(value);

  // 1. initially, it's not modified
  QVERIFY( !value->is_modified() );

  // 2. change the value
  lineEdit->setText("This is a sample text.");
  QVERIFY( value->is_modified() );

  // 3. change the value and commit
  lineEdit->setText("This is another one.");
  QVERIFY( value->is_modified() );
  value->commit();
  QVERIFY( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
