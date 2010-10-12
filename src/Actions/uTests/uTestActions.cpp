// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Actions"

#include <boost/test/unit_test.hpp>

#include "Common/LibCommon.hpp"

#include "Actions/LibActions.hpp"
#include "Actions/CForAllElementsT.hpp"
#include "Actions/CElementOperation.hpp"
#include "Actions/CSchemeLDA.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Actions;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( TestActionsSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ConstructorTest )
{
  CElementOperation* elem_op = new CSchemeLDA("LDA");
  CForAllElementsT<CSchemeLDA> elem_loop ("loop_LDA");
}

// ////////////////////////////////////////////////////////////////////////////////
// 
// BOOST_AUTO_TEST_CASE( ConstructorTest )
// {
//   CElementOperation* elem_op = new CSchemeLDA("LDA");
//   CForAllElementsT<CSchemeLDA> elem_loop ("loop_LDA");
// }

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////