// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RDM::Schemes::LDA"

#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"

#include "RDM/Schemes/LDA.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Common;

/// @todo create a library for support of the utests
/// @todo move this to a class that all utests global fixtures must inherit from
struct CoreInit {

  /// global initiate
  CoreInit()
  {
    using namespace boost::unit_test::framework;
    Core::instance().initiate( master_test_suite().argc, master_test_suite().argv);
  }

  /// global tear-down
  ~CoreInit()
  {
    Core::instance().terminate();
  }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( CoreInit )

BOOST_AUTO_TEST_SUITE( lda_test_suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( const_solution )
{
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
