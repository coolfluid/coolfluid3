// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Physics::VariableManager"

#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Physics/DynamicModel.hpp"
#include "Physics/VariableManager.hpp"


using namespace CF;
using namespace CF::Common;
using namespace CF::Physics;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VariableManagerSuite )

//////////////////////////////////////////////////////////////////////////////

/// Convenient access to the variable manager
PhysModel& physical_model()
{
  static boost::weak_ptr<PhysModel> model;
  if(!model.lock())
    model = Core::instance().root().create_component_ptr<DynamicModel>("PhysicalModel");

  return *model.lock();
}

BOOST_AUTO_TEST_CASE( Constructor )
{
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionAborts = false;
  ExceptionManager::instance().ExceptionOutputs = false;
  
  // Check empty mesh config
  BOOST_CHECK_EQUAL(physical_model().ndim(), 0u);
  BOOST_CHECK_EQUAL(physical_model().neqs(), 0u);
}

BOOST_AUTO_TEST_CASE( RegisterVariable )
{
  // Register state variables
  std::string pressure_symbol = "p";
  std::string pressure_field = "Pressure";
  physical_model().variable_manager().register_variable("Pressure", pressure_symbol, pressure_field, VariableManager::SCALAR, true);
  
  std::string vel_sym = "u";
  std::string vel_fd = "Velocity";
  physical_model().variable_manager().register_variable("Velocity", vel_sym, vel_fd, VariableManager::VECTOR, true);

  // Register a non-state var
  std::string rho_sym = "rho";
  std::string rho_fd = "Density";
  physical_model().variable_manager().register_variable("Density", rho_sym, rho_fd, VariableManager::SCALAR, false);

  // Check if the expected options are there

  BOOST_CHECK_EQUAL(physical_model().variable_manager().option("pressure_field_name").value_str(), "Pressure");
  BOOST_CHECK_EQUAL(physical_model().variable_manager().option("velocity_field_name").value_str(), "Velocity");
  BOOST_CHECK_EQUAL(physical_model().variable_manager().option("density_field_name").value_str(), "Density");
  BOOST_CHECK_EQUAL(physical_model().variable_manager().option("pressure_variable_name").value_str(), "p");
  BOOST_CHECK_EQUAL(physical_model().variable_manager().option("velocity_variable_name").value_str(), "u");
  BOOST_CHECK_EQUAL(physical_model().variable_manager().option("density_variable_name").value_str(), "rho");
  
  // Change the name of the field for the state varialbes
  physical_model().variable_manager().option("pressure_field_name").change_value(std::string("StateField"));
  physical_model().variable_manager().option("velocity_field_name").change_value(std::string("StateField"));
  
  BOOST_CHECK_EQUAL(pressure_field, "StateField");
  BOOST_CHECK_EQUAL(vel_fd, "StateField");
}

BOOST_AUTO_TEST_CASE( Dimensions )
{
  physical_model().variable_manager().configure_option("dimensions", 2u);

  // Check if the statistics are OK
  BOOST_CHECK_EQUAL(physical_model().ndim(), 2);
  BOOST_CHECK_EQUAL(physical_model().neqs(), 3);


  BOOST_CHECK(physical_model().variable_manager().is_state_variable("Velocity"));
  BOOST_CHECK(physical_model().variable_manager().is_state_variable("Pressure"));
  BOOST_CHECK(!physical_model().variable_manager().is_state_variable("Density"));

  BOOST_CHECK_EQUAL(physical_model().variable_manager().offset("Pressure"), 0);
  BOOST_CHECK_EQUAL(physical_model().variable_manager().offset("Velocity"), 1);
  try
  {
    const Uint offset = physical_model().variable_manager().offset("Density");
    BOOST_CHECK(false); // never reached
  }
  catch(const ValueNotFound& e)
  {
    BOOST_CHECK(true);
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
