// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the GUI GraphicalRestrictedList class"

#include <QComboBox>
#include <QHBoxLayout>
#include <QSignalSpy>

#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/StringConversion.hpp"
#include "common/URI.hpp"

#include "ui/graphics/GraphicalRestrictedList.hpp"

#include "test/ui/Application.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////////

QComboBox * find_combo_box(const GraphicalRestrictedList* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // TO THE GRAPHICAL LOOK OF GraphicalRestrictedList

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QComboBox * comboBox = nullptr;

  if( is_not_null(layout) )
  {
    comboBox = dynamic_cast<QComboBox*>(layout->itemAt(0)->widget());

    if( is_null(comboBox) )
      std::cerr << "Failed to find the line edit." << std::endl;
  }
  else
    std::cerr << "Failed to find the layout or cast it to QHBoxLayout." << std::endl;

  return comboBox;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiGraphicsGraphicalRestrictedListSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();
  GraphicalRestrictedList * value = new GraphicalRestrictedList();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  BOOST_CHECK( is_not_null( find_combo_box(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  GraphicalRestrictedList * value = new GraphicalRestrictedList();
  QComboBox * combo_box = find_combo_box(value);
  boost::shared_ptr<Option> opt( new OptionT<std::string>("opt", std::string("Hello") ) );

  // 1. empty option
  BOOST_CHECK( is_not_null(combo_box) );
  BOOST_CHECK_EQUAL( combo_box->count(), 0 );

  delete value;

  // 2. valid option with an empty list of restricted list
  value = new GraphicalRestrictedList(opt);
  combo_box = find_combo_box(value);
  BOOST_CHECK( is_not_null(combo_box) );
  BOOST_CHECK_EQUAL( combo_box->count(), 0 );

  delete value;

  // 3. option is not empty
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );
  value = new GraphicalRestrictedList(opt);
  combo_box = find_combo_box(value);

  BOOST_CHECK( is_not_null(combo_box) );
  BOOST_CHECK_EQUAL( combo_box->count(), 3 );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_value )
{
  boost::shared_ptr<Option> opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox * comboBox = find_combo_box(value);

  BOOST_CHECK( is_not_null(comboBox) );

  //
  // 1. check with strings
  //
  BOOST_CHECK( value->set_value("Hello") );
  BOOST_CHECK_EQUAL( comboBox->currentText().toStdString(), std::string("Hello") );

  BOOST_CHECK( value->set_value("World") );
  BOOST_CHECK_EQUAL( comboBox->currentText().toStdString(), std::string("World") );

  BOOST_CHECK( !value->set_value("something") ); // value does not exist in the list
  BOOST_CHECK_EQUAL( comboBox->currentText().toStdString(), std::string("World") ); // current text should be the same

  //
  // 2. check with other types
  //
  BOOST_CHECK( !value->set_value(12) );
  BOOST_CHECK_EQUAL( comboBox->currentText().toStdString(), std::string("World") );

  BOOST_CHECK( !value->set_value(3.141592) );
  BOOST_CHECK_EQUAL( comboBox->currentText().toStdString(), std::string("World") );

  BOOST_CHECK( !value->set_value(true) );
  BOOST_CHECK_EQUAL( comboBox->currentText().toStdString(), std::string("World") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value )
{
  boost::shared_ptr<Option> opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox * comboBox = find_combo_box(value);
  QVariant theValue;

  // get value when the line edit is empty
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::String );
  BOOST_CHECK_EQUAL( theValue.toString().toStdString(), std::string("Hello") );


  // get value when the line edit has a string
  comboBox->setCurrentIndex(2); // select the third value
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::String );
  BOOST_CHECK_EQUAL( theValue.toString().toStdString(), std::string("Third restricted value") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( signal_emitting )
{
  boost::shared_ptr<Option> opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox * comboBox = find_combo_box(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. through setValue()
  //
  value->set_value( QString("World") );
  value->set_value( QString("Hello") );

  // 2 signals should have been emitted
  BOOST_CHECK_EQUAL( spy.count(), 2 );

  spy.clear();

  //
  // 2. by changing the index of the combo box
  //
  value->show(); // make the value visible (it ignores keyboard events if not)
  comboBox->setCurrentIndex(2);
  comboBox->setCurrentIndex(0);
  comboBox->setCurrentIndex(1);

  // 3 signals should have been emitted
  BOOST_CHECK_EQUAL( spy.count(), 3 );

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
  boost::shared_ptr<Option> opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);

  value->set_value("Hello");
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("Hello") );

  value->set_value("World");
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("World") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_modified )
{
  boost::shared_ptr<Option> opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox* comboBox = find_combo_box(value);

  // 1. initially, it's not modified
  BOOST_CHECK( !value->is_modified() );

  // 2. change the value
  comboBox->setCurrentIndex(2);
  BOOST_CHECK( value->is_modified() );

  // 3. change the value and commit
  comboBox->setCurrentIndex(1);
  BOOST_CHECK( value->is_modified() );
  value->commit();
  BOOST_CHECK( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_restricted_list )
{
  boost::shared_ptr<Option> opt( new OptionT<std::string>("opt", std::string("Hello") ) );
  opt->restricted_list().push_back( std::string("World") );
  opt->restricted_list().push_back( std::string("Third restricted value") );

  GraphicalRestrictedList * value = new GraphicalRestrictedList(opt);
  QComboBox* comboBox = find_combo_box(value);
  QStringList newList;

  newList << "Here" << "is" << "a new" << "list";

  comboBox->setCurrentIndex(2);

  value->set_restricted_list( newList );

  BOOST_CHECK_EQUAL( comboBox->count(), newList.count() );
  BOOST_CHECK_EQUAL( comboBox->itemText(0).toStdString(), newList.at(0).toStdString() );
  BOOST_CHECK_EQUAL( comboBox->itemText(1).toStdString(), newList.at(1).toStdString() );
  BOOST_CHECK_EQUAL( comboBox->itemText(2).toStdString(), newList.at(2).toStdString() );
  BOOST_CHECK_EQUAL( comboBox->itemText(3).toStdString(), newList.at(3).toStdString() );

  BOOST_CHECK_EQUAL( comboBox->currentIndex(), 0 );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
