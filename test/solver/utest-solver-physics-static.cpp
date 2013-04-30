// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::physics::PhysModel"

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/mpl/void.hpp>

#include "common/CF.hpp"

using namespace cf3;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( PhysicsStaticSuite )

/// Prototype implementation of a static physics model. Compose up to 5 physical models
template<typename T1=boost::mpl::void_, typename T2=boost::mpl::void_, typename T3=boost::mpl::void_, typename T4=boost::mpl::void_, typename T5=boost::mpl::void_>
struct ComposedPhysics;

/// Specialization for 2 models
template<typename T1, typename T2>
struct ComposedPhysics<T1, T2, boost::mpl::void_, boost::mpl::void_, boost::mpl::void_> : T1, T2
{
  /// Compute the physics in the supplied argument (used mainly for recursion)
  template<typename PhysicsT>
  void compute(PhysicsT& physics)
  {
    T1::compute(physics);
    T2::compute(physics);
  };
  
  /// Compute physics properties
  void compute()
  {
    T1::compute(*this);
    T2::compute(*this);
  }
};

/// Specialization for 3 models
template<typename T1, typename T2, typename T3>
struct ComposedPhysics<T1, T2, T3, boost::mpl::void_, boost::mpl::void_> : T1, T2, T3
{
  /// Compute the physics in the supplied argument (used mainly for recursion)
  template<typename PhysicsT>
  void compute(PhysicsT& physics)
  {
    T1::compute(physics);
    T2::compute(physics);
    T3::compute(physics);
  };
  
  /// Compute physics properties
  void compute()
  {
    T1::compute(*this);
    T2::compute(*this);
    T3::compute(*this);
  }
};

/// Specialization for 4 models
template<typename T1, typename T2, typename T3, typename T4>
struct ComposedPhysics<T1, T2, T3, T4, boost::mpl::void_> : T1, T2, T3, T4
{
  /// Compute the physics in the supplied argument (used mainly for recursion)
  template<typename PhysicsT>
  void compute(PhysicsT& physics)
  {
    T1::compute(physics);
    T2::compute(physics);
    T3::compute(physics);
    T4::compute(physics);
  };
  
  /// Compute physics properties
  void compute()
  {
    T1::compute(*this);
    T2::compute(*this);
    T3::compute(*this);
    T4::compute(*this);
  }
};

/// Store local fluid properties
struct IsotermalFluid2D
{
  /// Pressure
  Real p;
  
  /// X Velocity
  Real u;
  
  /// Y velocity
  Real v;
  
  /// Empty computation, properties come from the solution
  template<typename T>
  void compute(T&)
  {
  }
};

/// Non-isothermal fluid (assume we get the temperature from somewhere else)
struct ThermalFluid
{
  Real temperature;
  
  template<typename T>
  void compute(T&)
  {
  }
};

/// Properties of air that depend on temperature
struct Air
{
  /// Heat capacity
  Real cp;
  
  /// Isentropic coefficient
  Real gamma;
  
  /// Ideal gas constant
  Real r;
  
  /// Compute the properties from the temperature
  template<typename T>
  void compute(T& physics)
  {
    physics.cp = 1010. * (physics.temperature + 1.) / physics.temperature; // dummy dependency on temperature
    physics.gamma = 1.4;
    physics.r = physics.cp - physics.cp / physics.gamma;
  }
};

/// Use the perfect gas law to get density
struct PerfectGas
{
  /// Density
  Real rho;
  
  template<typename T>
  void compute(T& physics)
  {
    physics.rho = physics.p / (physics.r * physics.temperature);
  }
};


//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( PhysicsStatic )
{
  ComposedPhysics<IsotermalFluid2D, ThermalFluid, Air, PerfectGas> physics;
  physics.p = 101325.;
  physics.temperature = 288.;
  physics.compute();
  BOOST_CHECK_CLOSE(physics.rho, 1.21497, 0.01);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

