// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the GUI GraphicalDouble class"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QSignalSpy>
#include <QTest>

#include "ui/graphics/GraphicalDouble.hpp"

#include "test/ui/Application.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////

QLineEdit * find_line_edit(const GraphicalDouble* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalDouble

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QLineEdit * lineEdit = nullptr;

  if( is_not_null(layout) )
  {
    lineEdit = dynamic_cast<QLineEdit*>(layout->itemAt(0)->widget());

    if( is_null(lineEdit) )
      std::cerr <<  "Failed to find the line edit." << std::endl;
  }
  else
    std::cerr << "Failed to find the layout or cast it to QHBoxLayout." << std::endl;

  return lineEdit;
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiGraphicsGraphicalBoolSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();
  GraphicalDouble * value = new GraphicalDouble();

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
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit * lineEdit = find_line_edit(value);

  // 1. value is 0.0 (note: converting 0.0 to string gives "0")
  BOOST_CHECK( is_not_null(lineEdit) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("0") );

  delete value;
  value = new GraphicalDouble(3.151492);
  lineEdit = find_line_edit(value);

  // 2. value is pi
  BOOST_CHECK( is_not_null(lineEdit) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("3.151492") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_value )
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit * lineEdit = find_line_edit(value);

  BOOST_CHECK( is_not_null(lineEdit) );

  //
  // 1. check with doubles
  //
  BOOST_CHECK( value->set_value(3.14159265) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("3.14159265") );

  BOOST_CHECK( value->set_value(-2.71) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("-2.71") );

  //
  // 2. check with other types
  //
  BOOST_CHECK( value->set_value(12) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("12") ); // value does not change

  BOOST_CHECK( !value->set_value("Hello") );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("12") ); // value does not change

  BOOST_CHECK( !value->set_value(true) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("12") ); // value does not change

  BOOST_CHECK( !value->set_value("1.6a45") );     // with an invalid character
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("12") ); // value does not change

  BOOST_CHECK( value->set_value("1.6E-45") );       // try scientific notation
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("1.6E-45") );


  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value )
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit * lineEdit = find_line_edit(value);
  QVariant theValue;

  // 1. the value is 0
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::Double );
  BOOST_CHECK_EQUAL( theValue.toDouble(), 0.0 );

  // 2. set another value
  lineEdit->setText("-2.71828183");
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::Double );
  BOOST_CHECK_EQUAL( theValue.toDouble(), -2.71828183 );

  // 3. try with scientific notation
  lineEdit->setText("1.6E45");
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::Double );
  BOOST_CHECK_EQUAL( theValue.toDouble(), 1.6e+45 );


  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( signal_emitting )
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit * lineEdit = find_line_edit(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. through setValue()
  //
  value->set_value(3.14159265);
  value->set_value(2.71);

  // 2 signals should have been emitted
  BOOST_CHECK_EQUAL( spy.count(), 2 );

  spy.clear();

  //
  // 2. by simulating keyboard events
  //
  lineEdit->clear();
  value->show(); // make the value visible (it ignores keyboard events if not)
  QTest::keyClicks(lineEdit, "-3.141-51a6e4512r4+5w6" );

  // 18 signals should have been emitted
  // (24 chars where entered, but only "-3.141516e4512456" were accepted)
  BOOST_CHECK_EQUAL( spy.count(), 18 );

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
  GraphicalDouble * value = new GraphicalDouble();

  value->set_value(-3.14159265);
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("-3.14159265") );

  value->set_value(1.6E-98);
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("1.6e-98") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_modified )
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit* lineEdit = find_line_edit(value);

  // 1. initially, it's not modified
  BOOST_CHECK( !value->is_modified() );

  // 2. change the value
  lineEdit->setText("12.45");
  BOOST_CHECK( value->is_modified() );

  // 3. change the value and commit
  lineEdit->setText("-5.879");
  BOOST_CHECK( value->is_modified() );
  value->commit();
  BOOST_CHECK( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
