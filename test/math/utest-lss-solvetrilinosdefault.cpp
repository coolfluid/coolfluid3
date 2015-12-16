// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the SolveLSS action"

#include <boost/assign/std/vector.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"

#include "common/PE/CommPattern.hpp"
#include "common/PE/CommWrapper.hpp"

#include "math/LSS/System.hpp"

using namespace boost::assign;

using namespace cf3;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::math;

BOOST_AUTO_TEST_SUITE( SolveSystemTrilinosSuite )

////////////////////////////////////////////////////////////////////////////////

// Solve a linear system using the trilinos default settings
BOOST_AUTO_TEST_CASE( TestSolveTrilinosDefault )
{
  Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);

  Component& root = Core::instance().root();
  Handle<LSS::System> lss = root.create_component<LSS::System>("LSS");
  lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  lss->read_native(URI(boost::unit_test::framework::master_test_suite().argv[1]));
  lss->solution_strategy()->options().set("compute_residual", true);
  lss->solve();
  
  Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
