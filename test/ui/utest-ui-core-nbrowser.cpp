// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ui NBrowser class"

#include "ui/core/NBrowser.hpp"

#include "test/ui/CoreApplication.hpp"

using namespace cf3::common;
using namespace cf3::ui::core;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiCoreNBrowserSuite )

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

BOOST_AUTO_TEST_CASE( generate_name )
{
  NBrowser b;

  BOOST_CHECK_EQUAL(b.generate_name().toStdString(), std::string("Browser_0"));
  BOOST_CHECK_EQUAL(b.generate_name().toStdString(), std::string("Browser_1"));
  BOOST_CHECK_EQUAL(b.generate_name().toStdString(), std::string("Browser_2"));

  for(int i = 0 ; i < 15 ; i++)
    b.generate_name();

  BOOST_CHECK_EQUAL(b.generate_name().toStdString(), std::string("Browser_18"));
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
