// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDoubleSpinBox>
#include <QHBoxLayout>

#include <QtTest>

#include "UI/Graphics/GraphicalInt.hpp"

#include "Math/MathConsts.hpp"

#include "test/UI/Core/ExceptionThrowHandler.hpp"

#include "test/UI/Graphics/GraphicalUintTest.hpp"

using namespace CF::Common;
using namespace CF::UI::Graphics;
using namespace CF::Math;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

void GraphicalUintTest::initTestCase()
{
  GraphicalInt * value = new GraphicalInt(false);

  QVERIFY( is_not_null( findSpinBox(value) ) );
  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUintTest::test_constructor()
{
  GraphicalInt * value = new GraphicalInt(true);
  QDoubleSpinBox * spinBox = findSpinBox(value);

  QVERIFY( is_not_null(spinBox) );

  // 1. check the range
  QCOMPARE( Uint(spinBox->minimum()), MathConsts::Uint_min() );
  QCOMPARE( Uint(spinBox->maximum()), MathConsts::Uint_max() );

  // 2. value is empty, the line edit should be empty as well
  QCOMPARE( int(spinBox->value()), 0 );

  delete value;
  value = new GraphicalInt(true, 1456);
  spinBox = findSpinBox(value);

  // 3. value is not empty
  QVERIFY( is_not_null(spinBox) );
  QCOMPARE( int(spinBox->value()), 1456 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUintTest::test_setValue()
{
  GraphicalInt * value = new GraphicalInt(true);
  QDoubleSpinBox * spinBox = findSpinBox(value);

  QVERIFY( is_not_null(spinBox) );

  //
  // 1. check with Uints
  //
  QVERIFY( value->setValue(Uint(1456)) );
  QCOMPARE( Uint(spinBox->value()), Uint(1456) );

  QVERIFY( value->setValue(Uint(215468)) );
  QCOMPARE( Uint(spinBox->value()), Uint(215468) );

  //
  // 2. check with other types
  //
  QVERIFY( !value->setValue(-3592) );
  QCOMPARE( Uint(spinBox->value()), Uint(215468) );

  QVERIFY( !value->setValue(3.141592) );
  QCOMPARE( Uint(spinBox->value()), Uint(215468) );

  QVERIFY( !value->setValue(true) );
  QCOMPARE( Uint(spinBox->value()), Uint(215468) );

  QVERIFY( !value->setValue("789654123") );
  QCOMPARE( Uint(spinBox->value()), Uint(215468) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUintTest::test_value()
{
  GraphicalInt * value = new GraphicalInt(true);
  QDoubleSpinBox * spinBox = findSpinBox(value);
  QVariant theValue;

  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::UInt );
  QCOMPARE( theValue.toInt(), 0 );

  spinBox->setValue(412654);
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::UInt );
  QCOMPARE( theValue.toInt(), 412654 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUintTest::test_signalEmmitting()
{
  GraphicalInt * value = new GraphicalInt(true);
  QDoubleSpinBox * spinBox = findSpinBox(value);
  QSignalSpy spy(value, SIGNAL(valueChanged()));

  //
  // 1. through setValue()
  //
  value->setValue(Uint(125464));
  value->setValue(-876541);

  // 1 signals should have been emitted (the second value is not a Uint)
  QCOMPARE( spy.count(), 1 );

  spy.clear();

  //
  // 2. by simulating keyboard events
  //
  spinBox->clear();
  value->show(); // make the value visible (it ignores keyboard events if not)
  QTest::keyClicks(spinBox, "+2014" );
  QTest::keyClicks(spinBox, "357" );
  QTest::keyClicks(spinBox, "aq45s2" );

  // 10 signals should have been emitted (one per character)
  // (13 chars entered but '-', 'a', 'q' and 's' were ignored)
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

void GraphicalUintTest::test_valueString()
{
  GraphicalInt * value = new GraphicalInt(true);

  value->setValue(Uint(78646));
  QCOMPARE( value->valueString(), QString("78646") );

  value->setValue(Uint(165464));
  QCOMPARE( value->valueString(), QString("165464") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUintTest::test_isModified()
{
  GraphicalInt * value = new GraphicalInt(true);
  QDoubleSpinBox* spinBox = findSpinBox(value);

  // 1. initially, it's not modified
  QVERIFY( !value->isModified() );

  // 2. change the value
  spinBox->setValue(123464);
  QVERIFY( value->isModified() );

  // 3. change the value and commit
  spinBox->setValue(65454354);
  QVERIFY( value->isModified() );
  value->commit();
  QVERIFY( !value->isModified() );

  // 4. put the same value
  spinBox->setValue(65454354);
  QVERIFY( !value->isModified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

QDoubleSpinBox * GraphicalUintTest::findSpinBox(const GraphicalInt* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalUint

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
} // UI
} // CF
