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

#include "Tools/MeshGeneration/MeshGeneration.hpp"

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
  
  BOOST_CHECK_EQUAL(physical_model().dimensions(), 0u);
  BOOST_CHECK_EQUAL(physical_model().nb_dof(), 0u);
}

BOOST_AUTO_TEST_CASE( RegisterVariable )
{
  // Register state variables
  physical_model().register_variable("Pressure", "p", CPhysicalModel::SCALAR, true);
  physical_model().register_variable("Velocity", "u", CPhysicalModel::VECTOR, true);
  
  // Register a non-state var
  physical_model().register_variable("Density", "rho", CPhysicalModel::SCALAR, false);
  
  // Check if the expected options are there
  BOOST_CHECK_EQUAL(physical_model().property("PressureFieldName").value_str(), "Pressure");
  BOOST_CHECK_EQUAL(physical_model().property("VelocityFieldName").value_str(), "Velocity");
  BOOST_CHECK_EQUAL(physical_model().property("DensityFieldName").value_str(), "Density");
  BOOST_CHECK_EQUAL(physical_model().property("PressureVariableName").value_str(), "p");
  BOOST_CHECK_EQUAL(physical_model().property("VelocityVariableName").value_str(), "u");
  BOOST_CHECK_EQUAL(physical_model().property("DensityVariableName").value_str(), "rho");
  
  // Change the name of the field for the state varialbes
  physical_model().property("PressureFieldName").change_value(std::string("StateField"));
  physical_model().property("VelocityFieldName").change_value(std::string("StateField"));
}

BOOST_AUTO_TEST_CASE( CreateFields )
{
  // Create a 2D test mesh
  CMesh& mesh = Core::instance().root().create_component<CMesh>("Grid2D");
  
  // Make a rectangle
  Tools::MeshGeneration::create_rectangle(mesh, 1., 1., 5, 5);
  BOOST_CHECK_EQUAL(mesh.dimension(), 2);
  
  // Set the mesh
  physical_model().set_mesh(mesh);
  
  // Check if the statistics are OK
  BOOST_CHECK_EQUAL(physical_model().dimensions(), 2);
  BOOST_CHECK_EQUAL(physical_model().nb_dof(), 3);
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

