// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::math::VariableManager"

#include <boost/test/unit_test.hpp>

#include "common/OptionList.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"


using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VariableManagerSuite )

//////////////////////////////////////////////////////////////////////////////

// Add a single scalar variable
BOOST_AUTO_TEST_CASE( CreateVariableDescriptor )
{
  boost::shared_ptr<VariableManager> manager = allocate_component<VariableManager>("manager");
  VariablesDescriptor& descriptor = manager->create_descriptor("solution", "a, b[v], c[t]");

  descriptor.options().set(common::Tags::dimension(), 2u);

  BOOST_CHECK(descriptor.has_tag("solution"));
  BOOST_CHECK_EQUAL(descriptor.size(), 7);
  BOOST_CHECK_EQUAL(descriptor.description(), "a[scalar],b[vector],c[tensor]");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
