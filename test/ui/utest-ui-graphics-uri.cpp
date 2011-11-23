// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the GUI GraphicalString class"

#include <QComboBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QSignalSpy>
#include <QTest>

#include "math/Consts.hpp"

#include "ui/graphics/GraphicalUri.hpp"

#include "test/ui/Application.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////

QWidget * find_widget(const GraphicalUri* value, int index)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalUri

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QWidget * widget = nullptr;

  if( is_not_null(layout) )
    widget = layout->itemAt(index)->widget();
  else
    std::cerr << "Failed to find the layout or cast it to QHBoxLayout." << std::endl;

  return widget;
}

//////////////////////////////////////////////////////////////////////////

QLineEdit * find_line_edit(const GraphicalUri* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalUri

  QLineEdit * lineEdit = dynamic_cast<QLineEdit*>(find_widget(value, 1));

  if( is_null(lineEdit) )
    std::cerr << "Failed to find the line edit." << std::endl;

  return lineEdit;
}

//////////////////////////////////////////////////////////////////////////

QComboBox * find_combo_box(const GraphicalUri* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalUri

  QComboBox * comboBox = dynamic_cast<QComboBox*>(find_widget(value, 0));

  if( is_null(comboBox) )
    std::cerr << "Failed to find the combo box." << std::endl;

  return comboBox;
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiGraphicsGraphicalBoolSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();
  GraphicalUri * value = new GraphicalUri();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  BOOST_CHECK( is_not_null( find_line_edit(value) ) );
  BOOST_CHECK( is_not_null( find_combo_box(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  GraphicalUri * value = new GraphicalUri();
  QLineEdit * lineEdit = find_line_edit(value);

  // 1. value is empty, the line edit should be empty as well
  BOOST_CHECK( is_not_null(lineEdit) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string() );

  delete value;
  OptionURI::Ptr option(new OptionURI("Option",  URI("cpath:/")));
  value = new GraphicalUri(option);
  lineEdit = find_line_edit(value);

  // 2. value is not empty
  BOOST_CHECK( is_not_null(lineEdit) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("cpath:/") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_schemes )
{
  OptionURI::Ptr option(new OptionURI("Option", URI("cpath:/")));
  GraphicalUri * value = new GraphicalUri(option);
  QComboBox * comboBox = find_combo_box(value);
  std::vector<URI::Scheme::Type> schemes;

  BOOST_CHECK( is_not_null(comboBox) );

  // 1. all protocols are used by default
  BOOST_CHECK_EQUAL( comboBox->count(), 3);
  BOOST_CHECK_EQUAL( comboBox->itemText(0).toStdString(), std::string("cpath") );
  BOOST_CHECK_EQUAL( comboBox->itemText(1).toStdString(), std::string("file") );
  BOOST_CHECK_EQUAL( comboBox->itemText(2).toStdString(), std::string("http") );

  // 2. some protocols
  schemes.push_back(URI::Scheme::CPATH);
  schemes.push_back(URI::Scheme::FILE);

  value->set_schemes(schemes);
  BOOST_CHECK_EQUAL( comboBox->count(), 2);
  BOOST_CHECK_EQUAL( comboBox->itemText(0).toStdString(), std::string("cpath") );
  BOOST_CHECK_EQUAL( comboBox->itemText(1).toStdString(), std::string("file") );

  // 3. some protocols where one appears twice
  schemes.clear();
  schemes.push_back(URI::Scheme::CPATH);
  schemes.push_back(URI::Scheme::FILE);
  schemes.push_back(URI::Scheme::CPATH);

  value->set_schemes(schemes);
  BOOST_CHECK_EQUAL( comboBox->count(), 2);
  BOOST_CHECK_EQUAL( comboBox->itemText(0).toStdString(), std::string("cpath") );
  BOOST_CHECK_EQUAL( comboBox->itemText(1).toStdString(), std::string("file") );

  // 4. giving an empty vector should enable all schemes
  schemes.clear();
  value->set_schemes(schemes);

  BOOST_CHECK_EQUAL( comboBox->count(), 3);
  BOOST_CHECK_EQUAL( comboBox->itemText(0).toStdString(), std::string("cpath") );
  BOOST_CHECK_EQUAL( comboBox->itemText(1).toStdString(), std::string("file") );
  BOOST_CHECK_EQUAL( comboBox->itemText(2).toStdString(), std::string("http") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_value )
{
  OptionURI::Ptr option(new OptionURI("Option", URI("cpath:/")));
  GraphicalUri * value = new GraphicalUri(option);
  QLineEdit * lineEdit = find_line_edit(value);

  BOOST_CHECK( is_not_null(lineEdit) );

  //
  // 1. check with strings
  //
  BOOST_CHECK( value->set_value("cpath:/Component") );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("cpath:/Component") );

  BOOST_CHECK( value->set_value("coolfluidsrv.vki.ac.be") );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("cpath:coolfluidsrv.vki.ac.be") );

  //
  // 2. check with other types
  //
  BOOST_CHECK( !value->set_value(12) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("cpath:coolfluidsrv.vki.ac.be") );

  BOOST_CHECK( !value->set_value(-421) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("cpath:coolfluidsrv.vki.ac.be") );

  BOOST_CHECK( !value->set_value(3.141592) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("cpath:coolfluidsrv.vki.ac.be") );

  BOOST_CHECK( !value->set_value(true) );
  BOOST_CHECK_EQUAL( lineEdit->text().toStdString(), std::string("cpath:coolfluidsrv.vki.ac.be") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( value )
{
  OptionURI::Ptr option(new OptionURI("Option", URI("cpath:/")));
  GraphicalUri * value = new GraphicalUri(option);
  QLineEdit * lineEdit = find_line_edit(value);
  QComboBox * comboBox = find_combo_box(value);
  QVariant theValue;

  value->show();

  // change the scheme
  comboBox->setCurrentIndex( comboBox->findText("file") );

  // 1. get value when the scheme exists in the line edit
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::String );
  BOOST_CHECK_EQUAL( theValue.toString().toStdString(), std::string("cpath:/") );

  // 2. get value when the scheme is determined by the combo box
  lineEdit->setText("/etc/fstab");
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::String );
  BOOST_CHECK_EQUAL( theValue.toString().toStdString(), std::string("file:/etc/fstab") );

  // change the scheme again
  comboBox->setCurrentIndex( comboBox->findText("http") );

  lineEdit->setText("coolfluidsrv.vki.ac.be");
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::String );
  BOOST_CHECK_EQUAL( theValue.toString().toStdString(), std::string("http:coolfluidsrv.vki.ac.be") );

  // 3. line edit is empty, should get an empty string
  lineEdit->setText("");
  theValue = value->value();
  BOOST_CHECK( theValue.type() == QVariant::String );
  BOOST_CHECK_EQUAL( theValue.toString().toStdString(), std::string() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( signal_emitting )
{
  GraphicalUri * value = new GraphicalUri();
  QLineEdit * lineEdit = find_line_edit(value);
  QSignalSpy spy(value, SIGNAL(value_changed()));

  //
  // 1. through setValue()
  //
  value->set_value("cpath:/");
  value->set_value("file:/etc/fstab");
  value->set_value(12);

  // 2 signals should have been emitted
  BOOST_CHECK_EQUAL( spy.count(), 2 );

  spy.clear();

  //
  // 2. by simulating keyboard events
  //
  lineEdit->clear();
  value->show(); // make the value visible (it ignores keyboard events if not)
  QTest::keyClicks(lineEdit, "cpath:" );
  QTest::keyClicks(lineEdit, "//Path" );
  QTest::keyClicks(lineEdit, "/To/A/Component" );

  // 32 signals should have been emitted (one per character)
  BOOST_CHECK_EQUAL( spy.count(), 28 );

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
  GraphicalUri * value = new GraphicalUri();

  value->set_value("cpath:/");
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("cpath:/") );

  value->set_value("http://coolfluidsrv.vki.ac.be");
  BOOST_CHECK_EQUAL( value->value_string().toStdString(), std::string("http://coolfluidsrv.vki.ac.be") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( is_modified )
{
  GraphicalUri * value = new GraphicalUri();
  QLineEdit* lineEdit = find_line_edit(value);

  // 1. initially, it's not modified
  BOOST_CHECK( !value->is_modified() );

  // 2. change the value
  lineEdit->setText("cpath:/");
  BOOST_CHECK( value->is_modified() );

  // 3. change the value and commit
  lineEdit->setText("cpath:/Component");
  BOOST_CHECK( value->is_modified() );
  value->commit();
  BOOST_CHECK( !value->is_modified() );

  // 4. set the same value
  lineEdit->setText("cpath:/Component");
  BOOST_CHECK( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
