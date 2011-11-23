// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the GUI GraphicalInt class with int values"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QSignalSpy>
#include <QTest>

#include "math/Consts.hpp"

#include "ui/graphics/GraphicalInt.hpp"

#include "test/ui/Application.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////

QDoubleSpinBox * find_spin_box(const GraphicalInt* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // TO THE GRAPHICAL LOOK OF GraphicalInt

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QDoubleSpinBox * spin_box = nullptr;

  if( is_not_null(layout) )
  {
    spin_box = dynamic_cast<QDoubleSpinBox*>(layout->itemAt(0)->widget());

    if( is_null(spin_box) )
      std::cerr << "Failed to find the spin box." << std::endl;
  }
  else
    std::cerr << "Failed to find the layout or cast it to QHBoxLayout." << std::endl;

  return spin_box;
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiGraphicsGraphicalBoolSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();
  GraphicalInt * value = new GraphicalInt(false);

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  BOOST_CHECK( is_not_null( find_spin_box(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox * spin_box = find_spin_box(value);

  BOOST_CHECK( is_not_null(spin_box) );

  // 1. check the range
  BOOST_CHECK_EQUAL( spin_box->minimum(), Consts::int_min() );
  BOOST_CHECK_EQUAL( spin_box->maximum(), Consts::int_max() );

  // 2. value is empty, the line edit should be empty as well
  BOOST_CHECK_EQUAL( int(spin_box->value()), 0 );

  delete value;
  value = new GraphicalInt(false, 1456);
  spin_box = find_spin_box(value);

  // 3. value is not empty
  BOOST_CHECK( is_not_null(spin_box) );
  BOOST_CHECK_EQUAL( int(spin_box->value()), 1456 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_value )
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox * spin_box = find_spin_box(value);

  BOOST_CHECK( is_not_null(spin_box) );

  //
  // 1. check with ints
  //
  BOOST_CHECK( value->set_value(-1456) );
  BOOST_CHECK_EQUAL( int(spin_box->value()), -1456 );

  BOOST_CHECK( value->set_value(215468548) );
  BOOST_CHECK_EQUAL( int(spin_box->value()), 215468548 );

  //
  // 2. check with other types
  //
  BOOST_CHECK( !value->set_value(3.141592) );
  BOOST_CHECK_EQUAL( int(spin_box->value()), 215468548 );

  BOOST_CHECK( !value->set_value(true) );
  BOOST_CHECK_EQUAL( int(spin_box->value()), 215468548 );

  BOOST_CHECK( !value->set_value("789654123") );
  BOOST_CHECK_EQUAL( int(spin_box->value()), 215468548 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value )
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox * spin_box = find_spin_box(value);
  QVariant theValue;

  // get value when the line edit is empty
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::Int );
  BOOST_CHECK_EQUAL( theValue.toInt(), 0 );

  // get value when the line edit has a string
  spin_box->setValue(488654);
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::Int );
  BOOST_CHECK_EQUAL( theValue.toInt(), 488654 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( signal_emitting )
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox * spin_box = find_spin_box(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. through setValue()
  //
  value->set_value(125464);
  value->set_value(-876541);

  // 2 signals should have been emitted
  BOOST_CHECK_EQUAL( spy.count(), 2 );

  spy.clear();

  //
  // 2. by simulating keyboard events
  //
  spin_box->clear();
  value->show(); // make the value visible (it ignores keyboard events if not)
  QTest::keyClicks(spin_box, "-2014" );
  QTest::keyClicks(spin_box, "357" );
  QTest::keyClicks(spin_box, "aq45s2" );

  // 10 signals should have been emitted (one per character)
  // (13 chars entered but 'a', 'q' and 's' were ignored)
  BOOST_CHECK_EQUAL( spy.count(), 10 );

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
  GraphicalInt * value = new GraphicalInt(false);

  value->set_value(78646);
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("78646") );

  value->set_value(165464);
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("165464") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_modified )
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox* spin_box = find_spin_box(value);

  // 1. initially, it's not modified
  BOOST_CHECK( !value->is_modified() );

  // 2. change the value
  spin_box->setValue(123464);
  BOOST_CHECK( value->is_modified() );

  // 3. change the value and commit
  spin_box->setValue(65454354);
  BOOST_CHECK( value->is_modified() );
  value->commit();
  BOOST_CHECK( !value->is_modified() );

  // 4. put the same value
  spin_box->setValue(65454354);
  BOOST_CHECK( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
