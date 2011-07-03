// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Solver::CPhysicalModel"

#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"

#include "Solver/CPhysicalModel.hpp"

#include "Mesh/CSimpleMeshGenerator.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( CPhysicalModelSuite )

//////////////////////////////////////////////////////////////////////////////

/// Convenient access to the physical model
CPhysicalModel& physical_model()
{
  static boost::weak_ptr<CPhysicalModel> model;
  if(!model.lock())
    model = Core::instance().root().create_component_ptr<CPhysicalModel>("PhysicalModel");

  return *model.lock();
}

BOOST_AUTO_TEST_CASE( Constructor )
{
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionAborts = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  CMesh& mesh = Core::instance().root().create_component<CMesh>("Grid2D");
  physical_model().configure_option("mesh", mesh.uri());
  
  // Check empty mesh config
  BOOST_CHECK_EQUAL(physical_model().dimensions(), 0u);
  BOOST_CHECK_EQUAL(physical_model().nb_dof(), 0u);
}

BOOST_AUTO_TEST_CASE( RegisterVariable )
{
  // Register state variables
  std::string pressure_symbol = "p";
  std::string pressure_field = "Pressure";
  physical_model().register_variable("Pressure", pressure_symbol, pressure_field, CPhysicalModel::SCALAR, true);
  
  std::string vel_sym = "u";
  std::string vel_fd = "Velocity";
  physical_model().register_variable("Velocity", vel_sym, vel_fd, CPhysicalModel::VECTOR, true);

  // Register a non-state var
  std::string rho_sym = "rho";
  std::string rho_fd = "Density";
  physical_model().register_variable("Density", rho_sym, rho_fd, CPhysicalModel::SCALAR, false);

  // Check if the expected options are there

  BOOST_CHECK_EQUAL(physical_model().option("PressureFieldName").value_str(), "Pressure");
  BOOST_CHECK_EQUAL(physical_model().option("VelocityFieldName").value_str(), "Velocity");
  BOOST_CHECK_EQUAL(physical_model().option("DensityFieldName").value_str(), "Density");
  BOOST_CHECK_EQUAL(physical_model().option("PressureVariableName").value_str(), "p");
  BOOST_CHECK_EQUAL(physical_model().option("VelocityVariableName").value_str(), "u");
  BOOST_CHECK_EQUAL(physical_model().option("DensityVariableName").value_str(), "rho");
  
  // Change the name of the field for the state varialbes
  physical_model().option("PressureFieldName").change_value(std::string("StateField"));
  physical_model().option("VelocityFieldName").change_value(std::string("StateField"));
  
  BOOST_CHECK_EQUAL(pressure_field, "StateField");
  BOOST_CHECK_EQUAL(vel_fd, "StateField");
}

BOOST_AUTO_TEST_CASE( CreateFields )
{
  // Make a rectangle
  CMesh& mesh = Core::instance().root().get_child("Grid2D").as_type<CMesh>();
  CSimpleMeshGenerator::create_rectangle(mesh, 1., 1., 5, 5);

  // Check if the statistics are OK
  BOOST_CHECK_EQUAL(physical_model().dimensions(), 2);
  BOOST_CHECK_EQUAL(physical_model().nb_dof(), 3);
  BOOST_CHECK_EQUAL(physical_model().nb_nodes(), 36);


  BOOST_CHECK(physical_model().is_state_variable("Velocity"));
  BOOST_CHECK(physical_model().is_state_variable("Pressure"));
  BOOST_CHECK(!physical_model().is_state_variable("Density"));

  BOOST_CHECK_EQUAL(physical_model().offset("Pressure"), 0);
  BOOST_CHECK_EQUAL(physical_model().offset("Velocity"), 1);
  try
  {
    const Uint offset = physical_model().offset("Density");
    BOOST_CHECK(false); // never reached
  }
  catch(const ValueNotFound& e)
  {
    BOOST_CHECK(true);
  }

  // Check if the fields are there
  BOOST_CHECK(mesh.get_child_ptr("StateField"));
  BOOST_CHECK(mesh.get_child_ptr("Density"));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

