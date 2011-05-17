// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QCheckBox>
#include <QHBoxLayout>

#include <QtTest>

#include "UI/Graphics/GraphicalBool.hpp"

#include "test/UI/Core/ExceptionThrowHandler.hpp"

#include "test/UI/Graphics/GraphicalBoolTest.hpp"

using namespace CF::Common;
using namespace CF::UI::Graphics;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace GraphicsTest {

///////////////////////////////////////////////////////////////////////////

void GraphicalBoolTest::initTestCase()
{
  GraphicalBool * value = new GraphicalBool();

  QVERIFY( is_not_null(  findCheckBox(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalBoolTest::test_constructor()
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = findCheckBox(value);

  // 1. value is false, the checkbox should be unchecked
  QVERIFY( is_not_null(checkbox) );
  QVERIFY( !checkbox->isChecked() );

  delete value;
  value = new GraphicalBool(true);
  checkbox = findCheckBox(value);

  // 2. value is true, the checkbox should be checked
  QVERIFY( is_not_null(checkbox) );
  QVERIFY( checkbox->isChecked() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalBoolTest::test_setValue()
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = findCheckBox(value);

  QVERIFY( is_not_null(checkbox) );

  //
  // 1. check with bool values
  //
  QVERIFY( value->setValue(true) );
  QVERIFY( checkbox->isChecked() );
  QVERIFY( value->originalValue().toBool() );

  QVERIFY( value->setValue(false) );
  QVERIFY( !checkbox->isChecked() );
  QVERIFY( !value->originalValue().toBool() );

  //
  // 2. check with strings (those supported by CF::Common::from_str<bool>())
  //
  QVERIFY( value->setValue("true") );
  QVERIFY( checkbox->isChecked() );

  QVERIFY( value->setValue("false") );
  QVERIFY( !checkbox->isChecked() );

  QVERIFY( value->setValue("on") );
  QVERIFY( checkbox->isChecked() );

  QVERIFY( value->setValue("off") );
  QVERIFY( !checkbox->isChecked() );

  QVERIFY( value->setValue("1") );
  QVERIFY( checkbox->isChecked() );

  QVERIFY( value->setValue("0") );
  QVERIFY( !checkbox->isChecked() );

  GUI_CHECK_THROW( value->setValue("ThisIsNotABoolValue"), ParsingFailed );
  QVERIFY( !checkbox->isChecked() ); // state should not have changed

  //
  // 3. check with other types
  //
  QVERIFY( !value->setValue(12) );
  QVERIFY( !checkbox->isChecked() );

  QVERIFY( !value->setValue(3.141592) );
  QVERIFY( !checkbox->isChecked() );

  QVERIFY( !value->setValue(-456) );
  QVERIFY( !checkbox->isChecked() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalBoolTest::test_value()
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = findCheckBox(value);
  QVariant isChecked;

  // get value when the check box is checked
  checkbox->setChecked(true);
  isChecked = value->value();
  QVERIFY( isChecked.type() == QVariant::Bool );
  QVERIFY( isChecked.toBool() );

  // get value when the check box is not checked
  checkbox->setChecked(false);
  isChecked = value->value();
  QVERIFY( isChecked.type() == QVariant::Bool );
  QVERIFY( !isChecked.toBool() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalBoolTest::test_signalEmmitting()
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = findCheckBox(value);
  QSignalSpy spy(value, SIGNAL(valueChanged()));

  //
  // 1. check/uncheck through setValue()
  //
  value->setValue(true);
  value->setValue(false);

  // 2 signal should have been emitted
  QCOMPARE( spy.count(), 2 );

  spy.clear();

  //
  // 2. check/uncheck by simulating a mouse click
  //
  value->show(); // make the value visible (it ignores mouse events if not)
  QTest::mouseClick(checkbox, Qt::LeftButton);
  QTest::mouseClick(checkbox, Qt::LeftButton);

  // 2 signal should have been emitted as well
  QCOMPARE( spy.count(), 2 );

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

void GraphicalBoolTest::test_valueString()
{
  GraphicalBool * value = new GraphicalBool(false);

  value->setValue(true);
  QCOMPARE( value->valueString(), QString("true") );

  value->setValue(false);
  QCOMPARE( value->valueString(), QString("false") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalBoolTest::test_isModified()
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = findCheckBox(value);

  // 1. initially, it's not modified
  QVERIFY( !value->isModified() );

  // 2. change the value
  checkbox->setChecked(true);
  QVERIFY( value->isModified() );

  // 3. set the same value again
  value->setValue(true);
  QVERIFY( !value->isModified() );

  // 4. change the value and commit
  checkbox->setChecked(false);
  QVERIFY( value->isModified() );
  value->commit();
  QVERIFY( !value->isModified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

QCheckBox * GraphicalBoolTest::findCheckBox(const GraphicalBool* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalBool

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QCheckBox * checkBox = nullptr;

  if( is_not_null(layout) )
  {
    checkBox = dynamic_cast<QCheckBox*>(layout->itemAt(0)->widget());

    if( is_null(checkBox) )
      QWARN("Failed to find the check box.");
  }
  else
    QWARN("Failed to find the layout or cast it to QHBoxLayout.");

  return checkBox;
}

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // UI
} // CF
