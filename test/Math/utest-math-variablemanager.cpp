// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Math::VariableManager"

#include <boost/test/unit_test.hpp>

#include "Math/VariableManager.hpp"
#include "Math/VariablesDescriptor.hpp"


using namespace CF;
using namespace CF::Common;
using namespace CF::Math;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VariableManagerSuite )

//////////////////////////////////////////////////////////////////////////////

// Add a single scalar variable
BOOST_AUTO_TEST_CASE( CreateVariableDescriptor )
{
  VariableManager::Ptr manager = allocate_component<VariableManager>("manager");
  VariablesDescriptor& descriptor = manager->create_descriptor("solution", "a, b[v], c[t]");

  descriptor.configure_option("dimensions", 2u);

  BOOST_CHECK(descriptor.has_tag("solution"));
  BOOST_CHECK_EQUAL(descriptor.size(), 7);
  BOOST_CHECK_EQUAL(descriptor.description(), "a[1],b[2],c[4]");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
