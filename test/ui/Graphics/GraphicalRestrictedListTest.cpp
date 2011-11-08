// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QHBoxLayout>

#include <QtTest>

#include "common/OptionT.hpp"

#include "ui/graphics/GraphicalRestrictedList.hpp"

#include "test/ui/ExceptionThrowHandler.hpp"

#include "test/ui/Graphics/GraphicalRestrictedListTest.hpp"

using namespace cf3::common;
using namespace cf3::ui::graphics;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedListTest::initTestCase()
{
  GraphicalRestrictedList * value = new GraphicalRestrictedList();

  QVERIFY( is_not_null( findComboBox(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedListTest::test_constructor()
{
  GraphicalRestrictedList * value = new GraphicalRestrictedList();
  QComboBox * comboBox = findComboBox(value);
  Option::Ptr opt( new OptionT<std::string>("opt", std::string("Hello") ) );

  // 1. empty option
  QVERIFY( is_not_null(comboBox) );
  QCOMPARE( comboBox->count(), 0 );

  delete value;

  // 2. valid option with an empty list of restricted list
  value = new GraphicalRestrictedList(opt);
  comboBox = findComboBox(value);
  QVERIFY( is_not_null(comboBox) );
  QCOMPARE( comboBox->count(), 0 );

  delete value;

  // 3. option is not empty
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );
  value = new GraphicalRestrictedList(opt);
  comboBox = findComboBox(value);

  QVERIFY( is_not_null(comboBox) );
  QCOMPARE( comboBox->count(), 3 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedListTest::test_setValue()
{
  Option::Ptr opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox * comboBox = findComboBox(value);

  QVERIFY( is_not_null(comboBox) );

  //
  // 1. check with strings
  //
  QVERIFY( value->set_value("Hello") );
  QCOMPARE( comboBox->currentText(), QString("Hello") );

  QVERIFY( value->set_value("World") );
  QCOMPARE( comboBox->currentText(), QString("World") );

  QVERIFY( !value->set_value("something") ); // value does not exist in the list
  QCOMPARE( comboBox->currentText(), QString("World") ); // current text should be the same

  //
  // 2. check with other types
  //
  QVERIFY( !value->set_value(12) );
  QCOMPARE( comboBox->currentText(), QString("World") );

  QVERIFY( !value->set_value(3.141592) );
  QCOMPARE( comboBox->currentText(), QString("World") );

  QVERIFY( !value->set_value(true) );
  QCOMPARE( comboBox->currentText(), QString("World") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedListTest::test_value()
{
  Option::Ptr opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox * comboBox = findComboBox(value);
  QVariant theValue;

  // get value when the line edit is empty
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString(), QString("Hello") );


  // get value when the line edit has a string
  comboBox->setCurrentIndex(2); // select the third value
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString(), QString("Third restricted value") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedListTest::test_signalEmmitting()
{
  Option::Ptr opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox * comboBox = findComboBox(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. through setValue()
  //
  value->set_value( QString("World") );
  value->set_value( QString("Hello") );

  // 2 signals should have been emitted
  QCOMPARE( spy.count(), 2 );

  spy.clear();

  //
  // 2. by changing the index of the combo box
  //
  value->show(); // make the value visible (it ignores keyboard events if not)
  comboBox->setCurrentIndex(2);
  comboBox->setCurrentIndex(0);
  comboBox->setCurrentIndex(1);

  // 3 signals should have been emitted
  QCOMPARE( spy.count(), 3 );

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

void GraphicalRestrictedListTest::test_valueString()
{
  Option::Ptr opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);

  value->set_value("Hello");
  QCOMPARE( value->value_string(), QString("Hello") );

  value->set_value("World");
  QCOMPARE( value->value_string(), QString("World") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedListTest::test_isModified()
{
  Option::Ptr opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox* comboBox = findComboBox(value);

  // 1. initially, it's not modified
  QVERIFY( !value->is_modified() );

  // 2. change the value
  comboBox->setCurrentIndex(2);
  QVERIFY( value->is_modified() );

  // 3. change the value and commit
  comboBox->setCurrentIndex(1);
  QVERIFY( value->is_modified() );
  value->commit();
  QVERIFY( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedListTest::test_setRestrictedList()
{
  Option::Ptr opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox* comboBox = findComboBox(value);
  QStringList newList;

  newList << "Here" << "is" << "a new" << "list";

  comboBox->setCurrentIndex(2);

  value->set_restricted_list( newList );

  QCOMPARE( comboBox->count(), newList.count() );
  QCOMPARE( comboBox->itemText(0), newList.at(0) );
  QCOMPARE( comboBox->itemText(1), newList.at(1) );
  QCOMPARE( comboBox->itemText(2), newList.at(2) );
  QCOMPARE( comboBox->itemText(3), newList.at(3) );

  QCOMPARE( comboBox->currentIndex(), 0 );
}

//////////////////////////////////////////////////////////////////////////

QComboBox * GraphicalRestrictedListTest::findComboBox(const GraphicalRestrictedList* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalRestrictedList

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QComboBox * comboBox = nullptr;

  if( is_not_null(layout) )
  {
    comboBox = dynamic_cast<QComboBox*>(layout->itemAt(0)->widget());

    if( is_null(comboBox) )
      QWARN("Failed to find the line edit.");
  }
  else
    QWARN("Failed to find the layout or cast it to QHBoxLayout.");

  return comboBox;
}

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // ui
} // cf3
