// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the GUI GraphicalBool class"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QSignalSpy>
#include <QTest>

#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/StringConversion.hpp"
#include "common/URI.hpp"

#include "ui/graphics/GraphicalBool.hpp"

#include "test/ui/Application.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////

QCheckBox * find_check_box(const GraphicalBool* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // TO THE GRAPHICAL LOOK OF GraphicalBool

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QCheckBox * checkBox = nullptr;

  if( is_not_null(layout) )
  {
    checkBox = dynamic_cast<QCheckBox*>(layout->itemAt(0)->widget());

    if( is_null(checkBox) )
      std::cerr << "Failed to find the check box." << std::endl;
  }
  else
    std::cerr << "Failed to find the layout or cast it to QHBoxLayout." << std::endl;

  return checkBox;
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiGraphicsGraphicalBoolSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();
  GraphicalBool * value = new GraphicalBool();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  BOOST_CHECK( is_not_null(  find_check_box(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = find_check_box(value);

  // 1. value is false, the checkbox should be unchecked
  BOOST_CHECK( is_not_null(checkbox) );
  BOOST_CHECK( !checkbox->isChecked() );

  delete value;
  value = new GraphicalBool(true);
  checkbox = find_check_box(value);

  // 2. value is true, the checkbox should be checked
  BOOST_CHECK( is_not_null(checkbox) );
  BOOST_CHECK( checkbox->isChecked() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_value )
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = find_check_box(value);

  BOOST_CHECK( is_not_null(checkbox) );

  //
  // 1. check with bool values
  //
  BOOST_CHECK( value->set_value(true) );
  BOOST_CHECK( checkbox->isChecked() );
  BOOST_CHECK( value->original_value().toBool() );

  BOOST_CHECK( value->set_value(false) );
  BOOST_CHECK( !checkbox->isChecked() );
  BOOST_CHECK( !value->original_value().toBool() );

  //
  // 2. check with strings (those supported by cf3::common::from_str<bool>())
  //
  BOOST_CHECK( value->set_value("true") );
  BOOST_CHECK( checkbox->isChecked() );

  BOOST_CHECK( value->set_value("false") );
  BOOST_CHECK( !checkbox->isChecked() );

  BOOST_CHECK( value->set_value("on") );
  BOOST_CHECK( checkbox->isChecked() );

  BOOST_CHECK( value->set_value("off") );
  BOOST_CHECK( !checkbox->isChecked() );

  BOOST_CHECK( value->set_value("1") );
  BOOST_CHECK( checkbox->isChecked() );

  BOOST_CHECK( value->set_value("0") );
  BOOST_CHECK( !checkbox->isChecked() );

  BOOST_CHECK_THROW( value->set_value("ThisIsNotABoolValue"), ParsingFailed );
  BOOST_CHECK( !checkbox->isChecked() ); // state should not have changed

  //
  // 3. check with other types
  //
  BOOST_CHECK( !value->set_value(12) );
  BOOST_CHECK( !checkbox->isChecked() );

  BOOST_CHECK( !value->set_value(3.141592) );
  BOOST_CHECK( !checkbox->isChecked() );

  BOOST_CHECK( !value->set_value(-456) );
  BOOST_CHECK( !checkbox->isChecked() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value )
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = find_check_box(value);
  QVariant isChecked;

  // get value when the check box is checked
  checkbox->setChecked(true);
  isChecked = value->value();
  BOOST_CHECK( isChecked.type() == QVariant::Bool );
  BOOST_CHECK( isChecked.toBool() );

  // get value when the check box is not checked
  checkbox->setChecked(false);
  isChecked = value->value();
  BOOST_CHECK( isChecked.type() == QVariant::Bool );
  BOOST_CHECK( !isChecked.toBool() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( signal_emitting )
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = find_check_box(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. check/uncheck through setValue()
  //
  value->set_value(true);
  value->set_value(false);

  // 2 signal should have been emitted
  BOOST_CHECK_EQUAL( spy.count(), 2 );

  spy.clear();

  //
  // 2. check/uncheck by simulating a mouse click
  //
  value->show(); // make the value visible (it ignores mouse events if not)
  QTest::mouseClick(checkbox, Qt::LeftButton);
  QTest::mouseClick(checkbox, Qt::LeftButton);

  // 2 signal should have been emitted as well
  BOOST_CHECK_EQUAL( spy.count(), 2 );

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
  GraphicalBool * value = new GraphicalBool(false);

  value->set_value(true);
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("true") );

  value->set_value(false);
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("false") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_modified )
{
  GraphicalBool * value = new GraphicalBool(false);
  QCheckBox * checkbox = find_check_box(value);

  // 1. initially, it's not modified
  BOOST_CHECK( !value->is_modified() );

  // 2. change the value
  checkbox->setChecked(true);
  BOOST_CHECK( value->is_modified() );

  // 3. set the same value again
  value->set_value(true);
  BOOST_CHECK( !value->is_modified() );

  // 4. change the value and commit
  checkbox->setChecked(false);
  BOOST_CHECK( value->is_modified() );
  value->commit();
  BOOST_CHECK( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
