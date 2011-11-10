// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDoubleSpinBox>
#include <QHBoxLayout>

#include <QtTest>

#include "ui/graphics/GraphicalInt.hpp"

#include "math/Consts.hpp"

#include "test/ui/ExceptionThrowHandler.hpp"

#include "test/ui/Graphics/GraphicalIntTest.hpp"

using namespace cf3::common;
using namespace cf3::ui::graphics;
using namespace cf3::math;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

void GraphicalIntTest::initTestCase()
{
  GraphicalInt * value = new GraphicalInt(false);

  QVERIFY( is_not_null( findSpinBox(value) ) );

  delete value;
}
//////////////////////////////////////////////////////////////////////////

void GraphicalIntTest::test_constructor()
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox * spinBox = findSpinBox(value);

  QVERIFY( is_not_null(spinBox) );

  // 1. check the range
  QCOMPARE( spinBox->minimum(), Consts::int_min() );
  QCOMPARE( spinBox->maximum(), Consts::int_max() );

  // 2. value is empty, the line edit should be empty as well
  QCOMPARE( int(spinBox->value()), 0 );

  delete value;
  value = new GraphicalInt(false, 1456);
  spinBox = findSpinBox(value);

  // 3. value is not empty
  QVERIFY( is_not_null(spinBox) );
  QCOMPARE( int(spinBox->value()), 1456 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalIntTest::test_setValue()
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox * spinBox = findSpinBox(value);

  QVERIFY( is_not_null(spinBox) );

  //
  // 1. check with ints
  //
  QVERIFY( value->set_value(-1456) );
  QCOMPARE( int(spinBox->value()), -1456 );

  QVERIFY( value->set_value(215468548) );
  QCOMPARE( int(spinBox->value()), 215468548 );

  //
  // 2. check with other types
  //
  QVERIFY( !value->set_value(3.141592) );
  QCOMPARE( int(spinBox->value()), 215468548 );

  QVERIFY( !value->set_value(true) );
  QCOMPARE( int(spinBox->value()), 215468548 );

  QVERIFY( !value->set_value("789654123") );
  QCOMPARE( int(spinBox->value()), 215468548 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalIntTest::test_value()
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox * spinBox = findSpinBox(value);
  QVariant theValue;

  // get value when the line edit is empty
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::Int );
  QCOMPARE( theValue.toInt(), 0 );

  // get value when the line edit has a string
  spinBox->setValue(488654);
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::Int );
  QCOMPARE( theValue.toInt(), 488654 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalIntTest::test_signalEmmitting()
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox * spinBox = findSpinBox(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. through setValue()
  //
  value->set_value(125464);
  value->set_value(-876541);

  // 2 signals should have been emitted
  QCOMPARE( spy.count(), 2 );

  spy.clear();

  //
  // 2. by simulating keyboard events
  //
  spinBox->clear();
  value->show(); // make the value visible (it ignores keyboard events if not)
  QTest::keyClicks(spinBox, "-2014" );
  QTest::keyClicks(spinBox, "357" );
  QTest::keyClicks(spinBox, "aq45s2" );

  // 10 signals should have been emitted (one per character)
  // (13 chars entered but 'a', 'q' and 's' were ignored)
  QCOMPARE( spy.count(), 10 );

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

void GraphicalIntTest::test_valueString()
{
  GraphicalInt * value = new GraphicalInt(false);

  value->set_value(78646);
  QCOMPARE( value->value_string(), QString("78646") );

  value->set_value(165464);
  QCOMPARE( value->value_string(), QString("165464") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalIntTest::test_isModified()
{
  GraphicalInt * value = new GraphicalInt(false);
  QDoubleSpinBox* spinBox = findSpinBox(value);

  // 1. initially, it's not modified
  QVERIFY( !value->is_modified() );

  // 2. change the value
  spinBox->setValue(123464);
  QVERIFY( value->is_modified() );

  // 3. change the value and commit
  spinBox->setValue(65454354);
  QVERIFY( value->is_modified() );
  value->commit();
  QVERIFY( !value->is_modified() );

  // 4. put the same value
  spinBox->setValue(65454354);
  QVERIFY( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

QDoubleSpinBox * GraphicalIntTest::findSpinBox(const GraphicalInt* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalInt

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QDoubleSpinBox * spinBox = nullptr;

  if( is_not_null(layout) )
  {
    spinBox = dynamic_cast<QDoubleSpinBox*>(layout->itemAt(0)->widget());

    if( is_null(spinBox) )
      QWARN("Failed to find the spin box.");
  }
  else
    QWARN("Failed to find the layout or cast it to QHBoxLayout.");

  return spinBox;
}

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // ui
} // cf3
