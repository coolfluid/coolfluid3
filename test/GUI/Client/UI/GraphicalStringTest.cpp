// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QLineEdit>
#include <QHBoxLayout>

#include <QtTest>

#include "UI/Graphics/GraphicalString.hpp"

#include "test/GUI/Client/Core/ExceptionThrowHandler.hpp"

#include "test/GUI/Client/UI/GraphicalStringTest.hpp"

using namespace CF::Common;
using namespace CF::UI::Graphics;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ClientTest {

//////////////////////////////////////////////////////////////////////////

void GraphicalStringTest::test_constructor()
{
  GraphicalString * value = new GraphicalString();
  QLineEdit * lineEdit = findLineEdit(value);

  // 1. value is empty, the line edit should be empty as well
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->text(), QString() );

  delete value;
  value = new GraphicalString("Hello, World!");
  lineEdit = findLineEdit(value);

  // 2. value is not empty
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->text(), QString("Hello, World!") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalStringTest::test_setValue()
{
  GraphicalString * value = new GraphicalString();
  QLineEdit * lineEdit = findLineEdit(value);

  QVERIFY( is_not_null(lineEdit) );

  //
  // 1. check with strings
  //
  QVERIFY( value->setValue("Hello") );
  QCOMPARE( lineEdit->text(), QString("Hello") );

  QVERIFY( value->setValue("World") );
  QCOMPARE( lineEdit->text(), QString("World") );

  //
  // 2. check with other types (this works since all primitive
  // types can be implicitly converted to QString by Qt)
  //
  QVERIFY( value->setValue(12) );
  QCOMPARE( lineEdit->text(), QString("12") );

  QVERIFY( value->setValue(3.141592) );
  QCOMPARE( lineEdit->text(), QString("3.141592") );

  QVERIFY( value->setValue(true) );
  QCOMPARE( lineEdit->text(), QString("true") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalStringTest::test_value()
{
  GraphicalString * value = new GraphicalString();
  QLineEdit * lineEdit = findLineEdit(value);
  QVariant theValue;

  // get value when the line edit is empty
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString(), QString() );

  // get value when the line edit has a string
  lineEdit->setText("This is a sample text.");
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString(), QString("This is a sample text.") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalStringTest::test_signalEmmitting()
{
  GraphicalString * value = new GraphicalString();
  QLineEdit * lineEdit = findLineEdit(value);
  QSignalSpy spy(value, SIGNAL(valueChanged()));

  //
  // 1. through setValue()
  //
  value->setValue("Hello");
  value->setValue("World!");

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

//////////////////////////////////////////////////////////////////////////

void GraphicalStringTest::test_valueString()
{
  GraphicalString * value = new GraphicalString();

  value->setValue("Hello");
  QCOMPARE( value->valueString(), QString("Hello") );

  value->setValue("World");
  QCOMPARE( value->valueString(), QString("World") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalStringTest::test_isModified()
{
  GraphicalString * value = new GraphicalString();
  QLineEdit* lineEdit = findLineEdit(value);

  // 1. initially, it's not modified
  QVERIFY( !value->isModified() );

  // 2. change the value
  lineEdit->setText("This is a sample text.");
  QVERIFY( value->isModified() );

  // 3. change the value and commit
  lineEdit->setText("This is another one.");
  QVERIFY( value->isModified() );
  value->commit();
  QVERIFY( !value->isModified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

QLineEdit * GraphicalStringTest::findLineEdit(const GraphicalString* value)
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

} // ClientTest
} // UI
} // CF
