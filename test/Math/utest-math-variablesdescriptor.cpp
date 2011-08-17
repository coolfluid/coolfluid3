// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Math::VariablesDescriptor"

#include <boost/test/unit_test.hpp>

#include "Math/VariablesDescriptor.hpp"


using namespace CF;
using namespace CF::Common;
using namespace CF::Math;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VariablesDescriptorSuite )

//////////////////////////////////////////////////////////////////////////////

// Add a single scalar variable
BOOST_AUTO_TEST_CASE( PushBackScalar )
{
  VariablesDescriptor::Ptr descriptor = allocate_component<VariablesDescriptor>("descriptor");
  descriptor->push_back("a", CF::Math::VariablesDescriptor::Dimensionalities::SCALAR);

  descriptor->configure_option(Common::Tags::dimension(), 3u);

  BOOST_CHECK_EQUAL(descriptor->size(), 1);
  BOOST_CHECK_EQUAL(descriptor->user_variable_name("a"), "a");

  descriptor->configure_option("a_variable_name", std::string("b"));

  BOOST_CHECK_EQUAL(descriptor->user_variable_name("a"), "b");
}

// Test vector and tensor behavior under changing dimensions, as well as description generator
BOOST_AUTO_TEST_CASE( PushBackVectors )
{
  VariablesDescriptor::Ptr descriptor = allocate_component<VariablesDescriptor>("descriptor");

  descriptor->configure_option(Common::Tags::dimension(), 2u);

  descriptor->push_back("v1", CF::Math::VariablesDescriptor::Dimensionalities::VECTOR);
  descriptor->push_back("v2", CF::Math::VariablesDescriptor::Dimensionalities::VECTOR);
  descriptor->push_back("t1", CF::Math::VariablesDescriptor::Dimensionalities::TENSOR);

  BOOST_CHECK_EQUAL(descriptor->size(), 8);
  BOOST_CHECK_EQUAL(descriptor->size("v1"), 2);
  BOOST_CHECK_EQUAL(descriptor->size("v2"), 2);
  BOOST_CHECK_EQUAL(descriptor->size("t1"), 4);
  BOOST_CHECK_EQUAL(descriptor->offset("v1"), 0);
  BOOST_CHECK_EQUAL(descriptor->offset("v2"), 2);
  BOOST_CHECK_EQUAL(descriptor->offset("t1"), 4);

  descriptor->configure_option(Common::Tags::dimension(), 3u);
  BOOST_CHECK_EQUAL(descriptor->size(), 15);
  BOOST_CHECK_EQUAL(descriptor->size("v1"), 3);
  BOOST_CHECK_EQUAL(descriptor->size("v2"), 3);
  BOOST_CHECK_EQUAL(descriptor->size("t1"), 9);
  BOOST_CHECK_EQUAL(descriptor->offset("v1"), 0);
  BOOST_CHECK_EQUAL(descriptor->offset("v2"), 3);
  BOOST_CHECK_EQUAL(descriptor->offset("t1"), 6);

  descriptor->push_back("s", CF::Math::VariablesDescriptor::Dimensionalities::SCALAR);
  BOOST_CHECK_EQUAL(descriptor->description(), "v1[3],v2[3],t1[9],s[1]");
}

// Test parsing of string to batch-add variables
BOOST_AUTO_TEST_CASE( ParseString )
{
  VariablesDescriptor::Ptr descriptor = allocate_component<VariablesDescriptor>("descriptor");

  descriptor->configure_option(Common::Tags::dimension(), 2u);

  // Add a scalar, vector and tensor
  descriptor->set_variables("a, b[v], c[t]");

  BOOST_CHECK_EQUAL(descriptor->size(), 7);
  BOOST_CHECK_EQUAL(descriptor->description(), "a[1],b[2],c[4]");

  BOOST_CHECK_EQUAL(descriptor->dimensionality("a"), VariablesDescriptor::Dimensionalities::SCALAR);
  BOOST_CHECK_EQUAL(descriptor->dimensionality("b"), VariablesDescriptor::Dimensionalities::VECTOR);
  BOOST_CHECK_EQUAL(descriptor->dimensionality("c"), VariablesDescriptor::Dimensionalities::TENSOR);
}

// Test parsing of string to batch-add variables, adding many scalars at a time
BOOST_AUTO_TEST_CASE( ParseStringArray )
{
  VariablesDescriptor::Ptr descriptor = allocate_component<VariablesDescriptor>("descriptor");

  descriptor->configure_option(Common::Tags::dimension(), 2u);

  // Add 5 scalars
  descriptor->set_variables("a[5]");

  BOOST_CHECK_EQUAL(descriptor->size(), 5);
  BOOST_CHECK_EQUAL(descriptor->description(), "a1[1],a2[1],a3[1],a4[1],a5[1]");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
