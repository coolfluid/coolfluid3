// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the GUI GraphicalValue class"

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

#include "test/ui/Application.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace boost::assign; // for operator += ()
using namespace cf3;
using namespace cf3::common;
using namespace cf3::ui::graphics;

//////////////////////////////////////////////////////////////////////////

const QValidator * find_array_validator(const GraphicalArray* array)
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
      std::cerr << "Failed to find the line edit." << std::endl;
    else
      validator = lineEdit->validator();
  }
  else
    std::cerr << "Failed to find the layout or cast it to QGridLayout." << std::endl;

  return validator;
}

//////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiGraphicsGraphicalValueSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_from_option )
{
  boost::shared_ptr<Option> option;
  GraphicalValue * value = nullptr;

  //
  // 1. null option, should get a null pointer
  //
  BOOST_CHECK_EQUAL( GraphicalValue::create_from_option(boost::shared_ptr< Option >()), (GraphicalValue*) nullptr );

  //
  // 2. check bool, should get a GraphicalBool
  //
  option = boost::shared_ptr< OptionT<bool> >(new OptionT<bool>("OptBool", true));
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalBool*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 3. check int, should get a GraphicalInt
  //
  option = boost::shared_ptr< OptionT<int> >(new OptionT<int>("OptInt", int(-156754)));
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalInt*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 4. check Uint, should get a GraphicalInt
  //
  option = boost::shared_ptr< OptionT<Uint> >(new OptionT<Uint>("OptUint", Uint(42)));
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalInt*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 5. check Uint, should get a GraphicalReal
  //
  option = boost::shared_ptr< OptionT<Real> >(new OptionT<Real>("OptReal", Real(3.141592)));
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalDouble*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 6. check str::string, should get a GraphicalString
  //
  option = boost::shared_ptr< OptionT<std::string> >(new OptionT<std::string>("OptString", std::string()));
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalString*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 7. check URI, should get a GraphicalUri
  //
  option = boost::shared_ptr< OptionURI >(new OptionURI("OptUri", URI()));
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalUri*>(value) ) );
  delete value;
  value = nullptr;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_from_option_array )
{
  boost::shared_ptr<Option> option;
  GraphicalValue * value = nullptr;
  const QValidator * validator;

  //
  // 1. null option, should get a null pointer
  //
  BOOST_CHECK_EQUAL( GraphicalValue::create_from_option(boost::shared_ptr< Option >()), (GraphicalValue*) nullptr );

  //
  // 2. check bool, should get a GraphicalArray
  //
  option = boost::shared_ptr< OptionArray<bool> >(new OptionArray<bool>("OptBool", std::vector<bool>()));
  value = GraphicalValue::create_from_option( option );

  // 2a. check that it is a GraphicalArray
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 2b. check that a validator is present
  validator = find_array_validator(dynamic_cast<GraphicalArray*>(value));
  BOOST_CHECK_MESSAGE( is_not_null(validator),
                       "No validator found. Please check the warning above." );

  // 2c. check its a QRegExpValidator with the correct string
  {
    const QRegExpValidator * val = dynamic_cast<const QRegExpValidator*>(validator);
    BOOST_CHECK( is_not_null(val) );
    BOOST_CHECK_EQUAL( val->regExp().pattern().toStdString(),
                        std::string("(true)|(false)|(1)|(0)|(on)|(off)") );
  }
  delete value;
  value = nullptr;

  //
  // 3. check int, should get a GraphicalArray
  //
  option = boost::shared_ptr< OptionArray<int> >(new OptionArray<int>("OptInt", std::vector<int>()));
  value = GraphicalValue::create_from_option( option );

  // 3a. check that it is a GraphicalArray
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 3b. check that a validator is present
  validator = find_array_validator(dynamic_cast<GraphicalArray*>(value));
  BOOST_CHECK_MESSAGE( is_not_null(validator), "No validator found. Please check the warning above." );

  // 2c. check its a QIntValidator
  {
    const QIntValidator * val = dynamic_cast<const QIntValidator*>(validator);
    BOOST_CHECK( is_not_null(val) );
  }

  delete value;
  value = nullptr;

  //
  // 4. check Uint, should get a GraphicalArray
  //
  option = boost::shared_ptr< OptionArray<Uint> >(new OptionArray<Uint>("OptUint", std::vector<Uint>()));
  value = GraphicalValue::create_from_option( option );

  // 4a. check that it is a GraphicalArray
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 4b. check that a validator is present
  validator = find_array_validator(dynamic_cast<GraphicalArray*>(value));
  BOOST_CHECK_MESSAGE( is_not_null(validator), "No validator found. Please check the warning above." );

  // 4c. check its a QIntValidator and the bottom is 0
  {
    const QIntValidator * val = dynamic_cast<const QIntValidator*>(validator);
    BOOST_CHECK( is_not_null(val) );
    BOOST_CHECK_EQUAL( val->bottom(), 0);
  }

  delete value;
  value = nullptr;

  //
  // 5. check Real, should get a GraphicalArray
  //
  option = boost::shared_ptr< OptionArray<Real> >(new OptionArray<Real>("OptReal", std::vector<Real>()));
  value = GraphicalValue::create_from_option( option );

  // 5a. check that it is a GraphicalArray
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 5b. check that a validator is present
  validator = find_array_validator(dynamic_cast<GraphicalArray*>(value));
  BOOST_CHECK_MESSAGE( is_not_null(validator), "No validator found. Please check the warning above." );

  // 5c. check its a QDoubleValidator
  {
    const QDoubleValidator * val = dynamic_cast<const QDoubleValidator*>(validator);
    BOOST_CHECK( is_not_null(val) );
  }

  delete value;
  value = nullptr;

  //
  // 6. check str::string, should get a GraphicalArray
  //
  option = boost::shared_ptr< OptionArray<std::string> >(new OptionArray<std::string>("OptString", std::vector<std::string>()));
  value = GraphicalValue::create_from_option( option );

  // 6a. check that it is a GraphicalArray
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null( dynamic_cast<GraphicalArray*>(value) ) );

  // 6b. check that *no* validator is present
  validator = find_array_validator(dynamic_cast<GraphicalArray*>(value));
  BOOST_CHECK_MESSAGE( is_null(validator), "Validator found. We should not have a validator here.");

  delete value;
  value = nullptr;

  //
  // 7. check URI, should get a GraphicalUriArray
  //
  option = boost::shared_ptr< OptionArray<URI> >(new OptionArray<URI>("OptString", std::vector<URI>()));
  value = GraphicalValue::create_from_option( option );

  // check that is is a GraphicalUriArray
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalUriArray*>(value) ) );
  delete value;
  value = nullptr;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_from_option_restr_values )
{
  boost::shared_ptr<Option> option;
  GraphicalValue * value = nullptr;

  //
  // 1. check bool
  //
  option = boost::shared_ptr< OptionT<bool> >(new OptionT<bool>("OptBool", true));
  option->restricted_list() += false;
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 2. check int
  //
  option = boost::shared_ptr< OptionT<int> >(new OptionT<int>("OptInt", int(-156754)));
  option->restricted_list() += int(47687876);
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 3. check Uint
  //
  option = boost::shared_ptr< OptionT<Uint> >(new OptionT<Uint>("OptUint", Uint(42)));
  option->restricted_list() += Uint(314);
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 4. check Real
  //
  option = boost::shared_ptr< OptionT<Real> >(new OptionT<Real>("OptReal", Real(3.141592)));
  option->restricted_list() += Real(2.71);
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 5. check str::string
  //
  option = boost::shared_ptr< OptionT<std::string> >(new OptionT<std::string>("OptString", std::string()));
  option->restricted_list() += std::string("Hello, World!");
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 6. check URI
  //
  option = boost::shared_ptr< OptionURI >(new OptionURI("OptUri", URI("http://www.google.com")));
  option->restricted_list().push_back( URI("cpath:/") );
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalRestrictedList*>(value) ) );
  delete value;
  value = nullptr;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_from_option_array_restr_values )
{
  boost::shared_ptr<Option> option;
  GraphicalValue * value = nullptr;

  //
  // 1. check bool
  //
  std::vector<bool> vectBool;
  vectBool.push_back(true);
  option = boost::shared_ptr< OptionArray<bool> >(new OptionArray<bool>("OptBool", vectBool));
  option->restricted_list() += false;
  BOOST_CHECK_NO_THROW(value = GraphicalValue::create_from_option( option ));
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 2. check int
  //
  std::vector<int> vectInt;
  vectInt.push_back(-154786);
  option = boost::shared_ptr< OptionArray<int> >(new OptionArray<int>("OptInt", vectInt));
  option->restricted_list() += int(47687876);
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 3. check Uint
  //
  std::vector<Uint> vectUint;
  vectUint.push_back(42);
  option = boost::shared_ptr< OptionArray<Uint> >(new OptionArray<Uint>("OptUint", vectUint));
  option->restricted_list() += Uint(3654614);
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 4. check Real
  //
  std::vector<Real> vectReal;
  vectReal.push_back(3.141592);
  option = boost::shared_ptr< OptionArray<Real> >(new OptionArray<Real>("OptReal", vectReal));
  option->restricted_list() += Real(2.71);
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 5. check str::string
  //
  std::vector<std::string> vectString;
  vectString.push_back("Hello");
  option = boost::shared_ptr< OptionArray<std::string> >(new OptionArray<std::string>("OptString", vectString));
  option->restricted_list() += std::string(", World!");
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;

  //
  // 6. check URI
  //
  std::vector<URI> vectUri;
  vectUri.push_back( URI("http://coolfluidsrv.vki.ac.be") );
  option = boost::shared_ptr< OptionArray<URI> >(new OptionArray<URI>("OptUri", vectUri));
  option->restricted_list() += URI("cpath:/");
  value = GraphicalValue::create_from_option( option );
  BOOST_CHECK( is_not_null(value) );
  BOOST_CHECK( is_not_null(dynamic_cast<GraphicalArrayRestrictedList*>(value) ) );
  delete value;
  value = nullptr;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
