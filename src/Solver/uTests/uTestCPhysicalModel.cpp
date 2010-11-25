// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Solver::CPhysicalModel"

#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Solver/CPhysicalModel.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Solver;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( CPhysicalModelSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ElementNodeTest )
{
  Component::Ptr comp = create_component_abstract_type<Component>("CF.Solver.CPhysicalModel", "CPhysicalModel");
  CPhysicalModel::Ptr model = boost::dynamic_pointer_cast<CPhysicalModel>(comp);
  BOOST_CHECK_EQUAL(model->dimensions(), 0u);
  BOOST_CHECK_EQUAL(model->nb_dof(), 0u);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

