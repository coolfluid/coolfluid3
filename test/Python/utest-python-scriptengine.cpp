// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Python::ScriptEngine"

#include <boost/test/unit_test.hpp>

#include "Common/BoostFilesystem.hpp"
#include "Common/CRoot.hpp"
#include "Common/Core.hpp"

#include "Python/ScriptEngine.hpp"

using namespace cf3;
using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VariablesDescriptorSuite )

//////////////////////////////////////////////////////////////////////////////

// Execute a script
BOOST_AUTO_TEST_CASE( ExecuteScript )
{
  BOOST_CHECK(boost::unit_test::framework::master_test_suite().argc == 2);

  CRoot& root = Core::instance().root();

  Python::ScriptEngine& engine = root.create_component<Python::ScriptEngine>("PythonEngine");

  boost::filesystem::fstream file(boost::unit_test::framework::master_test_suite().argv[1]);

  std::stringstream script_stream;
  script_stream << file.rdbuf();

  engine.execute_script(script_stream.str());

  BOOST_CHECK(is_not_null(root.get_child_ptr("group")));
  BOOST_CHECK(is_not_null(root.get_child("group").get_child_ptr("journal")));
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
