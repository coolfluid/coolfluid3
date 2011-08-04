// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Physics::VariablesDescriptor"

#include <boost/test/unit_test.hpp>

#include "Physics/VariablesDescriptor.hpp"


using namespace CF;
using namespace CF::Common;
using namespace CF::Physics;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VariablesDescriptorSuite )

//////////////////////////////////////////////////////////////////////////////

// Add a single scalar variable
BOOST_AUTO_TEST_CASE( PushBackScalar )
{
  VariablesDescriptor::Ptr descriptor = allocate_component<VariablesDescriptor>("descriptor");
  descriptor->push_back("a", CF::Physics::VariablesDescriptor::Dimensionalities::SCALAR);
  
  descriptor->configure_option("dimensions", 3u);
  
  BOOST_CHECK_EQUAL(descriptor->size(), 1);
  BOOST_CHECK_EQUAL(descriptor->field_name(), "descriptor");
  BOOST_CHECK_EQUAL(descriptor->user_variable_name("a"), "a");
  
  // change options
  descriptor->set_field_name("testfield");
  descriptor->configure_option("a_variable_name", std::string("b"));
  
  BOOST_CHECK_EQUAL(descriptor->field_name(), "testfield");
  BOOST_CHECK_EQUAL(descriptor->user_variable_name("a"), "b");
}

// Test vector and tensor behavior under changing dimensions, as well as description generator
BOOST_AUTO_TEST_CASE( PushBackVectors )
{
  VariablesDescriptor::Ptr descriptor = allocate_component<VariablesDescriptor>("descriptor");
  
  descriptor->configure_option("dimensions", 2u);
  
  descriptor->push_back("v1", CF::Physics::VariablesDescriptor::Dimensionalities::VECTOR);
  descriptor->push_back("v2", CF::Physics::VariablesDescriptor::Dimensionalities::VECTOR);
  descriptor->push_back("t1", CF::Physics::VariablesDescriptor::Dimensionalities::TENSOR);
  
  BOOST_CHECK_EQUAL(descriptor->size(), 8);
  BOOST_CHECK_EQUAL(descriptor->size("v1"), 2);
  BOOST_CHECK_EQUAL(descriptor->size("v2"), 2);
  BOOST_CHECK_EQUAL(descriptor->size("t1"), 4);
  BOOST_CHECK_EQUAL(descriptor->offset("v1"), 0);
  BOOST_CHECK_EQUAL(descriptor->offset("v2"), 2);
  BOOST_CHECK_EQUAL(descriptor->offset("t1"), 4);
  
  descriptor->configure_option("dimensions", 3u);
  BOOST_CHECK_EQUAL(descriptor->size(), 15);
  BOOST_CHECK_EQUAL(descriptor->size("v1"), 3);
  BOOST_CHECK_EQUAL(descriptor->size("v2"), 3);
  BOOST_CHECK_EQUAL(descriptor->size("t1"), 9);
  BOOST_CHECK_EQUAL(descriptor->offset("v1"), 0);
  BOOST_CHECK_EQUAL(descriptor->offset("v2"), 3);
  BOOST_CHECK_EQUAL(descriptor->offset("t1"), 6);
  
  descriptor->push_back("s", CF::Physics::VariablesDescriptor::Dimensionalities::SCALAR);
  BOOST_CHECK_EQUAL(descriptor->description(), "v1[3],v2[3],t1[9],s[1]");
}

// Test parsing of string to batch-add variables
BOOST_AUTO_TEST_CASE( ParseString )
{
  VariablesDescriptor::Ptr descriptor = allocate_component<VariablesDescriptor>("descriptor");
  
  descriptor->configure_option("dimensions", 2u);
  
  // Add a scalar, vector and tensor
  descriptor->set_variables("a, b[v], c[t]");
  
  BOOST_CHECK_EQUAL(descriptor->size(), 7);
  BOOST_CHECK_EQUAL(descriptor->description(), "a[1],b[2],c[4]");
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
