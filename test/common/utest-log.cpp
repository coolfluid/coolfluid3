// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF log facility"

#include <boost/test/unit_test.hpp>

#include <boost/iostreams/device/back_inserter.hpp>

#include <iostream>

#include "common/Log.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_SUITE( LogTestSuite )

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( openFiles )
{
  Logger::instance().openFiles();

  // files are not created if PE is not initializaed
  if(common::PE::Comm::instance().is_active())
  {
    BOOST_CHECK(Logger::instance().getStream(INFO).isFileOpen());
    BOOST_CHECK(Logger::instance().getStream(ERROR).isFileOpen());
    BOOST_CHECK(Logger::instance().getStream(WARNING).isFileOpen());
    BOOST_CHECK(Logger::instance().getStream(DEBUG).isFileOpen());
  }
  else
  {
    BOOST_CHECK(!Logger::instance().getStream(INFO).isFileOpen());
    BOOST_CHECK(!Logger::instance().getStream(ERROR).isFileOpen());
    BOOST_CHECK(!Logger::instance().getStream(WARNING).isFileOpen());
    BOOST_CHECK(!Logger::instance().getStream(DEBUG).isFileOpen());
  }
}

/// Demonstrate the need to flush after writing an atomic log message
BOOST_AUTO_TEST_CASE( FlushBuffer )
{
  // Output order is not what we expect
  CFinfo << "1. this is unflused CFlog line 1" << "\n";
  std::cout << "2. This is flused std::cout" << std::endl;
  CFinfo << "3. this is flushed CFlog line 2" << CFendl;
  CFinfo << "---------------------------------" << CFendl;
  // Output order is correct
  CFinfo << "1. this is flused CFlog line 1" << CFendl;
  std::cout << "2. This is flused std::cout" << std::endl;
  CFinfo << "3. this is flushed CFlog line 2" << CFendl;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_SUITE_END()
