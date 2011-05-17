// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QLineEdit>
#include <QHBoxLayout>

#include <QtTest>

#include "UI/Graphics/GraphicalDouble.hpp"

#include "test/UI/Core/ExceptionThrowHandler.hpp"

#include "test/UI/Graphics/GraphicalDoubleTest.hpp"

using namespace CF::Common;
using namespace CF::UI::Graphics;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace GraphicsTest {


//////////////////////////////////////////////////////////////////////////

void GraphicalDoubleTest::initTestCase()
{
  GraphicalDouble * value = new GraphicalDouble();

  QVERIFY( is_not_null( findLineEdit(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalDoubleTest::test_constructor()
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit * lineEdit = findLineEdit(value);

  // 1. value is 0.0 (note: converting 0.0 to string gives "0")
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->text(), QString("0") );

  delete value;
  value = new GraphicalDouble(3.151492);
  lineEdit = findLineEdit(value);

  // 2. value is pi
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->text(), QString("3.151492") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalDoubleTest::test_setValue()
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit * lineEdit = findLineEdit(value);

  QVERIFY( is_not_null(lineEdit) );

  //
  // 1. check with doubles
  //
  QVERIFY( value->setValue(3.14159265) );
  QCOMPARE( lineEdit->text(), QString("3.14159265") );

  QVERIFY( value->setValue(-2.71) );
  QCOMPARE( lineEdit->text(), QString("-2.71") );

  //
  // 2. check with other types
  //
  QVERIFY( value->setValue(12) );
  QCOMPARE( lineEdit->text(), QString("12") ); // value does not change

  QVERIFY( !value->setValue("Hello") );
  QCOMPARE( lineEdit->text(), QString("12") ); // value does not change

  QVERIFY( !value->setValue(true) );
  QCOMPARE( lineEdit->text(), QString("12") ); // value does not change

  QVERIFY( !value->setValue("1.6a45") );     // with an invalid character
  QCOMPARE( lineEdit->text(), QString("12") ); // value does not change

  QVERIFY( value->setValue("1.6E-45") );       // try scientific notation
  QCOMPARE( lineEdit->text(), QString("1.6E-45") );


  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalDoubleTest::test_value()
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit * lineEdit = findLineEdit(value);
  QVariant theValue;

  // 1. the value is 0
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::Double );
  QCOMPARE( theValue.toDouble(), 0.0 );

  // 2. set another value
  lineEdit->setText("-2.71828183");
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::Double );
  QCOMPARE( theValue.toDouble(), -2.71828183 );

  // 3. try with scientific notation
  lineEdit->setText("1.6E45");
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::Double );
  QCOMPARE( theValue.toDouble(), 1.6e+45 );


  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalDoubleTest::test_signalEmmitting()
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit * lineEdit = findLineEdit(value);
  QSignalSpy spy(value, SIGNAL(valueChanged()));

  //
  // 1. through setValue()
  //
  value->setValue(3.14159265);
  value->setValue(2.71);

  // 2 signals should have been emitted
  QCOMPARE( spy.count(), 2 );

  spy.clear();

  //
  // 2. by simulating keyboard events
  //
  lineEdit->clear();
  value->show(); // make the value visible (it ignores keyboard events if not)
  QTest::keyClicks(lineEdit, "-3.141-51a6e45.12r4+5w6" );

  // 18 signals should have been emitted
  // (24 chars where entered, but only "-3.141516e4512456" were accepted)
  QCOMPARE( spy.count(), 18 );

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

void GraphicalDoubleTest::test_valueString()
{
  GraphicalDouble * value = new GraphicalDouble();

  value->setValue(-3.14159265);
  QCOMPARE( value->valueString(), QString("-3.14159265") );

  value->setValue(1.6E-98);
  QCOMPARE( value->valueString(), QString("1.6e-98") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalDoubleTest::test_isModified()
{
  GraphicalDouble * value = new GraphicalDouble();
  QLineEdit* lineEdit = findLineEdit(value);

  // 1. initially, it's not modified
  QVERIFY( !value->isModified() );

  // 2. change the value
  lineEdit->setText("12.45");
  QVERIFY( value->isModified() );

  // 3. change the value and commit
  lineEdit->setText("-5.879");
  QVERIFY( value->isModified() );
  value->commit();
  QVERIFY( !value->isModified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

QLineEdit * GraphicalDoubleTest::findLineEdit(const GraphicalDouble* value)
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
      QWARN("Failed to find the line edit.");
  }
  else
    QWARN("Failed to find the layout or cast it to QHBoxLayout.");

  return lineEdit;
}

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // UI
} // CF
