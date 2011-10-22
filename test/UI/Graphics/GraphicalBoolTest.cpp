// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

using namespace cf3::common;
using namespace cf3::UI::Graphics;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
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
  QVERIFY( value->set_value(true) );
  QVERIFY( checkbox->isChecked() );
  QVERIFY( value->original_value().toBool() );

  QVERIFY( value->set_value(false) );
  QVERIFY( !checkbox->isChecked() );
  QVERIFY( !value->original_value().toBool() );

  //
  // 2. check with strings (those supported by cf3::common::from_str<bool>())
  //
  QVERIFY( value->set_value("true") );
  QVERIFY( checkbox->isChecked() );

  QVERIFY( value->set_value("false") );
  QVERIFY( !checkbox->isChecked() );

  QVERIFY( value->set_value("on") );
  QVERIFY( checkbox->isChecked() );

  QVERIFY( value->set_value("off") );
  QVERIFY( !checkbox->isChecked() );

  QVERIFY( value->set_value("1") );
  QVERIFY( checkbox->isChecked() );

  QVERIFY( value->set_value("0") );
  QVERIFY( !checkbox->isChecked() );

  GUI_CHECK_THROW( value->set_value("ThisIsNotABoolValue"), ParsingFailed );
  QVERIFY( !checkbox->isChecked() ); // state should not have changed

  //
  // 3. check with other types
  //
  QVERIFY( !value->set_value(12) );
  QVERIFY( !checkbox->isChecked() );

  QVERIFY( !value->set_value(3.141592) );
  QVERIFY( !checkbox->isChecked() );

  QVERIFY( !value->set_value(-456) );
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
  value->set_value(true);
  value->set_value(false);

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

  value->set_value(true);
  QCOMPARE( value->value_string(), QString("true") );

  value->set_value(false);
  QCOMPARE( value->value_string(), QString("false") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalBoolTest::test_isModified()
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = findCheckBox(value);

  // 1. initially, it's not modified
  QVERIFY( !value->is_modified() );

  // 2. change the value
  checkbox->setChecked(true);
  QVERIFY( value->is_modified() );

  // 3. set the same value again
  value->set_value(true);
  QVERIFY( !value->is_modified() );

  // 4. change the value and commit
  checkbox->setChecked(false);
  QVERIFY( value->is_modified() );
  value->commit();
  QVERIFY( !value->is_modified() );

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
} // cf3
