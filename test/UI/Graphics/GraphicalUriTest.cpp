// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>

#include <QtTest>

#include "UI/Graphics/GraphicalUri.hpp"

#include "test/UI/ExceptionThrowHandler.hpp"

#include "test/UI/Graphics/GraphicalUriTest.hpp"

using namespace cf3::common;
using namespace cf3::UI::Graphics;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

void GraphicalUriTest::initTestCase()
{
  GraphicalUri * value = new GraphicalUri();

  QVERIFY( is_not_null( findLineEdit(value) ) );
  QVERIFY( is_not_null( findComboBox(value) ) );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUriTest::test_constructor()
{
  GraphicalUri * value = new GraphicalUri();
  QLineEdit * lineEdit = findLineEdit(value);

  // 1. value is empty, the line edit should be empty as well
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->text(), QString() );

  delete value;
  OptionURI::Ptr option(new OptionURI("Option",  URI("cpath://Root")));
  value = new GraphicalUri(option);
  lineEdit = findLineEdit(value);

  // 2. value is not empty
  QVERIFY( is_not_null(lineEdit) );
  QCOMPARE( lineEdit->text(), QString("cpath://Root") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUriTest::test_setSchemes()
{
  OptionURI::Ptr option(new OptionURI("Option", URI("cpath://Root")));
  GraphicalUri * value = new GraphicalUri(option);
  QComboBox * comboBox = findComboBox(value);
  std::vector<URI::Scheme::Type> schemes;

  QVERIFY( is_not_null(comboBox) );

  // 1. all protocols are used by default
  QCOMPARE( comboBox->count(), 3);
  QCOMPARE( comboBox->itemText(0), QString("cpath") );
  QCOMPARE( comboBox->itemText(1), QString("file") );
  QCOMPARE( comboBox->itemText(2), QString("http") );

  // 2. some protocols
  schemes.push_back(URI::Scheme::CPATH);
  schemes.push_back(URI::Scheme::FILE);

  value->set_schemes(schemes);
  QCOMPARE( comboBox->count(), 2);
  QCOMPARE( comboBox->itemText(0), QString("cpath") );
  QCOMPARE( comboBox->itemText(1), QString("file") );

  // 3. some protocols where one appears twice
  schemes.clear();
  schemes.push_back(URI::Scheme::CPATH);
  schemes.push_back(URI::Scheme::FILE);
  schemes.push_back(URI::Scheme::CPATH);

  value->set_schemes(schemes);
  QCOMPARE( comboBox->count(), 2);
  QCOMPARE( comboBox->itemText(0), QString("cpath") );
  QCOMPARE( comboBox->itemText(1), QString("file") );

  // 4. giving an empty vector should enable all schemes
  schemes.clear();
  value->set_schemes(schemes);

  QCOMPARE( comboBox->count(), 3);
  QCOMPARE( comboBox->itemText(0), QString("cpath") );
  QCOMPARE( comboBox->itemText(1), QString("file") );
  QCOMPARE( comboBox->itemText(2), QString("http") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUriTest::test_setValue()
{
  OptionURI::Ptr option(new OptionURI("Option", URI("cpath://Root")));
  GraphicalUri * value = new GraphicalUri(option);
  QLineEdit * lineEdit = findLineEdit(value);

  QVERIFY( is_not_null(lineEdit) );

  //
  // 1. check with strings
  //
  QVERIFY( value->set_value("cpath://Root/Component") );
  QCOMPARE( lineEdit->text(), QString("cpath://Root/Component") );

  QVERIFY( value->set_value("coolfluidsrv.vki.ac.be") );
  QCOMPARE( lineEdit->text(), QString("cpath:coolfluidsrv.vki.ac.be") );

  //
  // 2. check with other types
  //
  QVERIFY( !value->set_value(12) );
  QCOMPARE( lineEdit->text(), QString("cpath:coolfluidsrv.vki.ac.be") );

  QVERIFY( !value->set_value(-421) );
  QCOMPARE( lineEdit->text(), QString("cpath:coolfluidsrv.vki.ac.be") );

  QVERIFY( !value->set_value(3.141592) );
  QCOMPARE( lineEdit->text(), QString("cpath:coolfluidsrv.vki.ac.be") );

  QVERIFY( !value->set_value(true) );
  QCOMPARE( lineEdit->text(), QString("cpath:coolfluidsrv.vki.ac.be") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUriTest::test_value()
{
  OptionURI::Ptr option(new OptionURI("Option", URI("cpath://Root")));
  GraphicalUri * value = new GraphicalUri(option);
  QLineEdit * lineEdit = findLineEdit(value);
  QComboBox * comboBox = findComboBox(value);
  QVariant theValue;

  value->show();

  // change the scheme
  comboBox->setCurrentIndex( comboBox->findText("file") );

  // 1. get value when the scheme exists in the line edit
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString(), QString("cpath://Root") );

  // 2. get value when the scheme is determined by the combo box
  lineEdit->setText("/etc/fstab");
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString(), QString("file:/etc/fstab") );

  // change the scheme again
  comboBox->setCurrentIndex( comboBox->findText("http") );

  lineEdit->setText("coolfluidsrv.vki.ac.be");
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString(), QString("http:coolfluidsrv.vki.ac.be") );

  // 3. line edit is empty, should get an empty string
  lineEdit->setText("");
  theValue = value->value();
  QVERIFY( theValue.type() == QVariant::String );
  QCOMPARE( theValue.toString(), QString() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUriTest::test_signalEmmitting()
{
  GraphicalUri * value = new GraphicalUri();
  QLineEdit * lineEdit = findLineEdit(value);
  QSignalSpy spy(value, SIGNAL(valueChanged()));

  //
  // 1. through setValue()
  //
  value->set_value("cpath://Root");
  value->set_value("file:/etc/fstab");
  value->set_value(12);

  // 2 signals should have been emitted
  QCOMPARE( spy.count(), 2 );

  spy.clear();

  //
  // 2. by simulating keyboard events
  //
  lineEdit->clear();
  value->show(); // make the value visible (it ignores keyboard events if not)
  QTest::keyClicks(lineEdit, "cpath:" );
  QTest::keyClicks(lineEdit, "//Root/Path" );
  QTest::keyClicks(lineEdit, "/To/A/Component" );

  // 32 signals should have been emitted (one per character)
  QCOMPARE( spy.count(), 33 );

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

void GraphicalUriTest::test_valueString()
{
  GraphicalUri * value = new GraphicalUri();

  value->set_value("cpath://Root");
  QCOMPARE( value->value_string(), QString("cpath://Root") );

  value->set_value("http://coolfluidsrv.vki.ac.be");
  QCOMPARE( value->value_string(), QString("http://coolfluidsrv.vki.ac.be") );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalUriTest::test_isModified()
{
  GraphicalUri * value = new GraphicalUri();
  QLineEdit* lineEdit = findLineEdit(value);

  // 1. initially, it's not modified
  QVERIFY( !value->is_modified() );

  // 2. change the value
  lineEdit->setText("cpath://Root");
  QVERIFY( value->is_modified() );

  // 3. change the value and commit
  lineEdit->setText("cpath://Root/Component");
  QVERIFY( value->is_modified() );
  value->commit();
  QVERIFY( !value->is_modified() );

  // 4. set the same value
  lineEdit->setText("cpath://Root/Component");
  QVERIFY( !value->is_modified() );

  delete value;
}

//////////////////////////////////////////////////////////////////////////

QLineEdit * GraphicalUriTest::findLineEdit(const GraphicalUri* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalUri

  QLineEdit * lineEdit = dynamic_cast<QLineEdit*>(findWidget(value, 1));

  if( is_null(lineEdit) )
    QWARN("Failed to find the line edit.");

  return lineEdit;
}

//////////////////////////////////////////////////////////////////////////

QComboBox * GraphicalUriTest::findComboBox(const GraphicalUri* value)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalUri

  QComboBox * comboBox = dynamic_cast<QComboBox*>(findWidget(value, 0));

  if( is_null(comboBox) )
    QWARN("Failed to find the combo box.");

  return comboBox;
}

//////////////////////////////////////////////////////////////////////////

QWidget * GraphicalUriTest::findWidget(const GraphicalUri* value, int index)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalUri

  QHBoxLayout * layout = dynamic_cast<QHBoxLayout*>(value->layout());
  QWidget * widget = nullptr;

  if( is_not_null(layout) )
    widget = layout->itemAt(index)->widget();
  else
    QWARN("Failed to find the layout or cast it to QHBoxLayout.");

  return widget;
}

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // UI
} // cf3
