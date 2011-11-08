// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QGridLayout>
#include <QLineEdit>
#include <QValidator>
#include <QtTest>

#include <boost/assign/std/vector.hpp>

#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"

#include "math/Consts.hpp"

#include "ui/graphics/GraphicalArray.hpp"
#include "ui/graphics/GraphicalArrayRestrictedList.hpp"
#include "ui/graphics/GraphicalBool.hpp"
#include "ui/graphics/GraphicalDouble.hpp"
#include "ui/graphics/GraphicalRestrictedList.hpp"
#include "ui/graphics/GraphicalString.hpp"
#include "ui/graphics/GraphicalInt.hpp"
#include "ui/graphics/GraphicalUri.hpp"
#include "ui/graphics/GraphicalUriArray.hpp"

#include "ui/graphics/GraphicalValue.hpp"

#include "test/ui/ExceptionThrowHandler.hpp"
#include "test/ui/Graphics/GraphicalValueTest.hpp"

using namespace boost::assign;
using namespace cf3::common;
using namespace cf3::ui::graphics;
using namespace cf3::math;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace GraphicsTest {

//////////////////////////////////////////////////////////////////////////

void GraphicalValueTest::test_createFromOption()
{
  Option::Ptr option;
  GraphicalValue * value = nullptr;

  //
  // 1. null option, should get a null pointer
  //
  QCOMPARE( GraphicalValue::create_from_option(Option::ConstPtr()), (GraphicalValue*) nullptr );

  //
  // 2. check bool, should get a GraphicalBool
  //
  option = OptionT<bool>::Ptr(new OptionT<bool>("OptBool", true));
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalBool*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 3. check int, should get a GraphicalInt
  //
  option = OptionT<int>::Ptr(new OptionT<int>("OptInt", int(-156754)));
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalInt*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 4. check Uint, should get a GraphicalInt
  //
  option = OptionT<Uint>::Ptr(new OptionT<Uint>("OptUint", Uint(42)));
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalInt*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 5. check Uint, should get a GraphicalReal
  //
  option = OptionT<Real>::Ptr(new OptionT<Real>("OptReal", Real(3.141592)));
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalDouble*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 6. check str::string, should get a GraphicalString
  //
  option = OptionT<std::string>::Ptr(new OptionT<std::string>("OptString", std::string()));
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalString*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 7. check URI, should get a GraphicalUri
  //
  option = OptionURI::Ptr(new OptionURI("OptUri", URI()));
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalUri*>(value) ) );
  delete value;
  value = nullptr;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalValueTest::test_createFromOptionArray()
{
  Option::Ptr option;
  GraphicalValue * value = nullptr;
  const QValidator * validator;

  //
  // 1. null option, should get a null pointer
  //
  QCOMPARE( GraphicalValue::create_from_option(Option::ConstPtr()), (GraphicalValue*) nullptr );

  //
  // 2. check bool, should get a GraphicalArray
  //
  option = OptionArrayT<bool>::Ptr(new OptionArrayT<bool>("OptBool", std::vector<bool>()));
  value = GraphicalValue::create_from_option( option );

  // 2a. check that it is a GraphicalArray
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 2b. check that a validator is present
  validator = arrayValidator(dynamic_cast<GraphicalArray*>(value));
  QVERIFY2( is_not_null(validator), "No validator found. Please check the warning above." );

  // 2c. check its a QRegExpValidator with the correct string
  {
    const QRegExpValidator * val = dynamic_cast<const QRegExpValidator*>(validator);
    QVERIFY( is_not_null(val) );
    QCOMPARE( val->regExp().pattern(), QString("(true)|(false)|(1)|(0)|(on)|(off)") );
  }
  delete value;
  value = nullptr;

  //
  // 3. check int, should get a GraphicalArray
  //
  option = OptionArrayT<int>::Ptr(new OptionArrayT<int>("OptInt", std::vector<int>()));
  value = GraphicalValue::create_from_option( option );

  // 3a. check that it is a GraphicalArray
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 3b. check that a validator is present
  validator = arrayValidator(dynamic_cast<GraphicalArray*>(value));
  QVERIFY2( is_not_null(validator), "No validator found. Please check the warning above." );

  // 2c. check its a QIntValidator
  {
    const QIntValidator * val = dynamic_cast<const QIntValidator*>(validator);
    QVERIFY( is_not_null(val) );
  }

  delete value;
  value = nullptr;

  //
  // 4. check Uint, should get a GraphicalArray
  //
  option = OptionArrayT<Uint>::Ptr(new OptionArrayT<Uint>("OptUint", std::vector<Uint>()));
  value = GraphicalValue::create_from_option( option );

  // 4a. check that it is a GraphicalArray
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 4b. check that a validator is present
  validator = arrayValidator(dynamic_cast<GraphicalArray*>(value));
  QVERIFY2( is_not_null(validator), "No validator found. Please check the warning above." );

  // 4c. check its a QIntValidator and the bottom is 0
  {
    const QIntValidator * val = dynamic_cast<const QIntValidator*>(validator);
    QVERIFY( is_not_null(val) );
    QCOMPARE( val->bottom(), 0);
  }

  delete value;
  value = nullptr;

  //
  // 5. check Real, should get a GraphicalArray
  //
  option = OptionArrayT<Real>::Ptr(new OptionArrayT<Real>("OptReal", std::vector<Real>()));
  value = GraphicalValue::create_from_option( option );

  // 5a. check that it is a GraphicalArray
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 5b. check that a validator is present
  validator = arrayValidator(dynamic_cast<GraphicalArray*>(value));
  QVERIFY2( is_not_null(validator), "No validator found. Please check the warning above." );

  // 5c. check its a QDoubleValidator
  {
    const QDoubleValidator * val = dynamic_cast<const QDoubleValidator*>(validator);
    QVERIFY( is_not_null(val) );
  }

  delete value;
  value = nullptr;

  //
  // 6. check str::string, should get a GraphicalArray
  //
  option = OptionArrayT<std::string>::Ptr(new OptionArrayT<std::string>("OptString", std::vector<std::string>()));
  value = GraphicalValue::create_from_option( option );

  // 6a. check that it is a GraphicalArray
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 6b. check that *no* validator is present
  validator = arrayValidator(dynamic_cast<GraphicalArray*>(value));
  QVERIFY2( is_null(validator), "Validator found. We should not have a validator here.");

  delete value;
  value = nullptr;

  //
  // 7. check URI, should get a GraphicalUriArray
  //
  option = OptionArrayT<URI>::Ptr(new OptionArrayT<URI>("OptString", std::vector<URI>()));
  value = GraphicalValue::create_from_option( option );

  // check that is is a GraphicalUriArray
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalUriArray*>(value) ) );
  delete value;
  value = nullptr;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalValueTest::test_createFromOptionRestrValues()
{
  Option::Ptr option;
  GraphicalValue * value = nullptr;

  //
  // 1. check bool
  //
  option = OptionT<bool>::Ptr(new OptionT<bool>("OptBool", true));
  option->restricted_list() += false;
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 2. check int
  //
  option = OptionT<int>::Ptr(new OptionT<int>("OptInt", int(-156754)));
  option->restricted_list() += int(47687876);
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 3. check Uint
  //
  option = OptionT<Uint>::Ptr(new OptionT<Uint>("OptUint", Uint(42)));
  option->restricted_list() += Uint(314);
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 4. check Real
  //
  option = OptionT<Real>::Ptr(new OptionT<Real>("OptReal", Real(3.141592)));
  option->restricted_list() += Real(2.71);
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 5. check str::string
  //
  option = OptionT<std::string>::Ptr(new OptionT<std::string>("OptString", std::string()));
  option->restricted_list() += std::string("Hello, World!");
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 6. check URI
  //
  option = OptionURI::Ptr(new OptionURI("OptUri", URI("http://www.google.com")));
  option->restricted_list().push_back( URI("cpath:/") );
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;
}

//////////////////////////////////////////////////////////////////////////

void GraphicalValueTest::test_createFromOptionArrayRestrValues()
{
  Option::Ptr option;
  GraphicalValue * value = nullptr;

  //
  // 1. check bool
  //
  std::vector<bool> vectBool;
  vectBool.push_back(true);
  option = OptionArrayT<bool>::Ptr(new OptionArrayT<bool>("OptBool", vectBool));
  option->restricted_list() += false;
  Gui_CHECK_NO_THROW(value = GraphicalValue::create_from_option( option ));
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 2. check int
  //
  std::vector<int> vectInt;
  vectInt.push_back(-154786);
  option = OptionArrayT<int>::Ptr(new OptionArrayT<int>("OptInt", vectInt));
  option->restricted_list() += int(47687876);
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 3. check Uint
  //
  std::vector<Uint> vectUint;
  vectUint.push_back(42);
  option = OptionArrayT<Uint>::Ptr(new OptionArrayT<Uint>("OptUint", vectUint));
  option->restricted_list() += Uint(3654614);
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 4. check Real
  //
  std::vector<Real> vectReal;
  vectReal.push_back(3.141592);
  option = OptionArrayT<Real>::Ptr(new OptionArrayT<Real>("OptReal", vectReal));
  option->restricted_list() += Real(2.71);
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 5. check str::string
  //
  std::vector<std::string> vectString;
  vectString.push_back("Hello");
  option = OptionArrayT<std::string>::Ptr(new OptionArrayT<std::string>("OptString", vectString));
  option->restricted_list() += std::string(", World!");
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 6. check URI
  //
  std::vector<URI> vectUri;
  vectUri.push_back( URI("http://coolfluidsrv.vki.ac.be") );
  option = OptionArrayT<URI>::Ptr(new OptionArrayT<URI>("OptUri", vectUri));
  option->restricted_list() += URI("cpath:/");
  value = GraphicalValue::create_from_option( option );
  QVERIFY( is_not_null(value) );
  QVERIFY( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;
}

//////////////////////////////////////////////////////////////////////////

const QValidator * GraphicalValueTest::arrayValidator(const GraphicalArray* array)
{
  // /!\ WARNING /!\
  // THE FOLLOWING CODE IS EXTREMELY SENSITIVE TO ANY MODIFICATION
  // OF THE GRAPHICAL LOOK OF GraphicalArray

  // array->layout()->itemAt(0)->widget() is the QGroupBox of the GraphicalArray
  // we know it has a QGridLayout
  QGridLayout * layout = dynamic_cast<QGridLayout*>(array->layout()->itemAt(0)->widget()->layout());
  const QValidator * validator = nullptr;

  if( is_not_null(layout) )
  {
    QLineEdit * lineEdit = dynamic_cast<QLineEdit*>(layout->itemAtPosition(0, 0)->widget());

    if( is_null(lineEdit) )
      QWARN("Failed to find the line edit.");
    else
      validator = lineEdit->validator();
  }
  else
    QWARN("Failed to find the layout or cast it to QGridLayout.");

  return validator;
}

//////////////////////////////////////////////////////////////////////////

} // GraphicsTest
} // ui
} // cf3
