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

//----------------------------------------------------------------------------
// PhysModel.hpp

class Variables;

/// @note would be a component
class PhysModel {
public:

  struct Properties {};

  virtual ~PhysModel() {}

  virtual Uint dim() = 0;
  virtual Uint nb_eqs() = 0;

  virtual PhysModel::Properties* create_properties() = 0;

  virtual Variables* create_variables( const std::string& name ) = 0;

};

//----------------------------------------------------------------------------
// Variables.hpp

/// @note would be a component
class Variables {
public:
  virtual ~Variables() {}
  virtual void compute_properties( PhysModel::Properties& physp ) = 0;
  virtual void flux_jacobian( PhysModel::Properties& physp ) = 0;
};

//----------------------------------------------------------------------------
// VariablesT.hpp

template < typename PHYS >
class VariablesT : public Variables {
public:

  virtual ~VariablesT() {}

  virtual void compute_properties( PhysModel::Properties& physp )
  {
    typename PHYS::PROPS& cphysp = static_cast<typename PHYS::PROPS&>( physp );
    PHYS::compute_properties( cphysp );
  }

  virtual void flux_jacobian( PhysModel::Properties& physp )
  {
    typename PHYS::PROPS& cphysp = static_cast<typename PHYS::PROPS&>( physp );
    PHYS::flux_jacobian( cphysp );
  }

};

//----------------------------------------------------------------------------
// Euler2D.hpp

class Euler2D : public PhysModel {
public:

  enum { dimension = 2 };
  enum { neqs = 4 };

  struct Properties : public PhysModel::Properties
  {
    Real u;
    Real v;
  };

  virtual Uint dim()    { return (Uint) dimension; }
  virtual Uint nb_eqs() { return (Uint) neqs; }

  virtual ~Euler2D() {}

  virtual PhysModel::Properties* create_properties()
  {
    return new Euler2D::Properties();
  }

  virtual Variables* create_variables( const std::string& name );

};

//----------------------------------------------------------------------------
// Euler2DCons.hpp

struct Euler2DCons
{
  typedef Euler2D           MODEL;
  typedef MODEL::Properties PROPS;

  static void compute_properties (PROPS& p )
  {
    p.u =  0.;
    p.v = 10.;
  }

  static void flux_jacobian  ( PROPS& p )
  {
    std::cout << "u " << p.u << std::endl;
    std::cout << "v " << p.v << std::endl;
  }
};

//----------------------------------------------------------------------------
// Euler2D.cpp

Variables* Euler2D::create_variables( const std::string& name )
{
  if (name == "cons")
    return new VariablesT<Euler2DCons>();
  else
    throw std::string("no such variable set available");
}


//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( PhysicsSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dynamic_api )
{
  PhysModel* pmodel = new Euler2D();

  PhysModel::Properties* props = pmodel->create_properties();
  Variables* pv = pmodel->create_variables( "cons");

  pv->compute_properties(*props);
  pv->flux_jacobian(*props);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( static_api )
{
  PhysModel* pmodel = new Euler2D();

  PhysModel::Properties* props = pmodel->create_properties();

  Euler2D::Properties* ep = static_cast<Euler2D::Properties*>(props);

  Euler2DCons::compute_properties(*ep);
  Euler2DCons::flux_jacobian(*ep);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( definition_of_model )
{
  PhysModel* pmodel = new Euler2D();

  BOOST_CHECK_EQUAL( pmodel->dim() ,    (Uint) Euler2D::dimension );
  BOOST_CHECK_EQUAL( pmodel->nb_eqs() , (Uint) Euler2D::neqs );

  BOOST_CHECK_EQUAL( (Uint) Euler2DCons::MODEL::dimension , (Uint) Euler2D::dimension );
  BOOST_CHECK_EQUAL( (Uint) Euler2DCons::MODEL::neqs ,      (Uint) Euler2D::neqs );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

