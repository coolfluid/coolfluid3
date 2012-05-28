// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::math::VariablesDescriptor"

#include <boost/test/unit_test.hpp>

#include "common/OptionList.hpp"

#include "math/VariablesDescriptor.hpp"


using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VariablesDescriptorSuite )

//////////////////////////////////////////////////////////////////////////////

// Add a single scalar variable
BOOST_AUTO_TEST_CASE( PushBackScalar )
{
  boost::shared_ptr<VariablesDescriptor> descriptor = allocate_component<VariablesDescriptor>("descriptor");
  descriptor->push_back("a", cf3::math::VariablesDescriptor::Dimensionalities::SCALAR);

  descriptor->options().set(common::Tags::dimension(), 3u);

  BOOST_CHECK_EQUAL(descriptor->size(), 1);
  BOOST_CHECK_EQUAL(descriptor->user_variable_name("a"), "a");

  descriptor->options().set("a_variable_name", std::string("b"));

  BOOST_CHECK_EQUAL(descriptor->user_variable_name("a"), "b");
}

// Test vector and tensor behavior under changing dimensions, as well as description generator
BOOST_AUTO_TEST_CASE( PushBackVectors )
{
  boost::shared_ptr<VariablesDescriptor> descriptor = allocate_component<VariablesDescriptor>("descriptor");

  descriptor->options().set(common::Tags::dimension(), 2u);

  descriptor->push_back("v1", cf3::math::VariablesDescriptor::Dimensionalities::VECTOR);
  descriptor->push_back("v2", cf3::math::VariablesDescriptor::Dimensionalities::VECTOR);
  descriptor->push_back("t1", cf3::math::VariablesDescriptor::Dimensionalities::TENSOR);

  BOOST_CHECK_EQUAL(descriptor->size(), 8);
  BOOST_CHECK_EQUAL(descriptor->size("v1"), 2);
  BOOST_CHECK_EQUAL(descriptor->size("v2"), 2);
  BOOST_CHECK_EQUAL(descriptor->size("t1"), 4);
  BOOST_CHECK_EQUAL(descriptor->offset("v1"), 0);
  BOOST_CHECK_EQUAL(descriptor->offset("v2"), 2);
  BOOST_CHECK_EQUAL(descriptor->offset("t1"), 4);

  descriptor->options().set(common::Tags::dimension(), 3u);
  BOOST_CHECK_EQUAL(descriptor->size(), 15);
  BOOST_CHECK_EQUAL(descriptor->size("v1"), 3);
  BOOST_CHECK_EQUAL(descriptor->size("v2"), 3);
  BOOST_CHECK_EQUAL(descriptor->size("t1"), 9);
  BOOST_CHECK_EQUAL(descriptor->offset("v1"), 0);
  BOOST_CHECK_EQUAL(descriptor->offset("v2"), 3);
  BOOST_CHECK_EQUAL(descriptor->offset("t1"), 6);

  descriptor->push_back("s", cf3::math::VariablesDescriptor::Dimensionalities::SCALAR);
  BOOST_CHECK_EQUAL(descriptor->description(), "v1[vector],v2[vector],t1[tensor],s[scalar]");
}

// Test parsing of string to batch-add variables
BOOST_AUTO_TEST_CASE( ParseString )
{
  boost::shared_ptr<VariablesDescriptor> descriptor = allocate_component<VariablesDescriptor>("descriptor");

  descriptor->options().set(common::Tags::dimension(), 2u);

  // Add a scalar, vector and tensor
  descriptor->set_variables("a, b[v], c[t]");

  BOOST_CHECK_EQUAL(descriptor->size(), 7);
  BOOST_CHECK_EQUAL(descriptor->description(), "a[scalar],b[vector],c[tensor]");

  BOOST_CHECK_EQUAL(descriptor->dimensionality("a"), VariablesDescriptor::Dimensionalities::SCALAR);
  BOOST_CHECK_EQUAL(descriptor->dimensionality("b"), VariablesDescriptor::Dimensionalities::VECTOR);
  BOOST_CHECK_EQUAL(descriptor->dimensionality("c"), VariablesDescriptor::Dimensionalities::TENSOR);
}

// Test parsing of string to batch-add variables, adding many scalars at a time
BOOST_AUTO_TEST_CASE( ParseStringArray )
{
  boost::shared_ptr<VariablesDescriptor> descriptor = allocate_component<VariablesDescriptor>("descriptor");

  descriptor->options().set(common::Tags::dimension(), 2u);

  // Add 5 scalars
  descriptor->set_variables("a[5]");

  BOOST_CHECK_EQUAL(descriptor->size(), 5);
  BOOST_CHECK_EQUAL(descriptor->description(), "a1[scalar],a2[scalar],a3[scalar],a4[scalar],a5[scalar]");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
