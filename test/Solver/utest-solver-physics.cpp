// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Solver::Physics"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/CreateComponent.hpp"
#include "Solver/Physics.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Solver;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( PhysicsSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Physics )
{
   Physics my_physics;
   my_physics.resize(2);

   ExceptionManager::instance().ExceptionDumps=false;
   ExceptionManager::instance().ExceptionOutputs=false;
   BOOST_CHECK_THROW(my_physics.compute_var(0) , ValueNotFound);
   ExceptionManager::instance().ExceptionDumps=true;
   ExceptionManager::instance().ExceptionOutputs=true;

   my_physics.set_var(0,1.);

   BOOST_CHECK_EQUAL(my_physics.var(0) , 1.);
   BOOST_CHECK_EQUAL(my_physics.compute_var(0) , 1.);
}

////////////////////////////////////////////////////////////////////////////////

class Solver_API Gravity : public Physics
{
  // problem:   Find the moment and the force, given a length and mass
  // --------
  //
  // |
  // |         LENGTH
  // |=============================.  FORCE
  // | MOMENT                     ::
  // |                            ::
  // |                            /\   MASS
  // |                           ````
  // |

public:

  enum Vars { MOMENT=0, FORCE=1, MASS=2, GRAV=3, LENGTH=4 };

  Gravity() : Physics()
  {
    resize(5);

    set_compute_function(FORCE, boost::bind( &Gravity::force , this ) );

    var_deps(FORCE) = boost::assign::list_of(MASS)(GRAV);

    set_compute_function(MOMENT, boost::bind( &Gravity::moment , this ) );
    var_deps(MOMENT) = boost::assign::list_of(FORCE)(LENGTH);

    set_var(GRAV , 9.81);
  }

  Real force()
  {
    return var(MASS) * var(GRAV);
  }

  Real moment()
  {
    return var(FORCE) * var(LENGTH);
  }
};

BOOST_AUTO_TEST_CASE( test_Gravity )
{
   Gravity my_physics;

   // Force calculation
   // -----------------
   my_physics.set_var(Gravity::MASS, 10.);

   // now MOMENT will NOT be computed, as it is not required.
   BOOST_CHECK_CLOSE(my_physics.compute_var(Gravity::FORCE), 98.1 , 0.0000000001);
   BOOST_CHECK_EQUAL(my_physics.var(Gravity::MASS) , 10.);
   BOOST_CHECK_EQUAL(my_physics.var(Gravity::GRAV) , 9.81);


   // Moment calculation
   // ------------------
   my_physics.set_var(Gravity::LENGTH, 10.);

   // now FORCE will not be recomputed, it is already stored.
   BOOST_CHECK_CLOSE(my_physics.compute_var(Gravity::MOMENT), 981 , 0.0000000001);
   BOOST_CHECK_EQUAL(my_physics.var(Gravity::LENGTH), 10.);

}

////////////////////////////////////////////////////////////////////////////////

// OTHER NOT RECOMMENDED METHOD BUT POSSIBLE:
// Fully scriptable physics

///////////////////////////////////////////////////////////////////////////////


Real compute_my_var0(Physics* p)
{
    return 1.+p->var(1);
}

BOOST_AUTO_TEST_CASE( test_CreatePhysics )
{
   Physics my_physics;

   // create 2 variables
   #define VAR0 0
   #define VAR1 1
   my_physics.resize(2);

   // Setup computation function of VAR0 as function of VAR1
   my_physics.var_deps(VAR0).push_back(VAR1);
   my_physics.set_compute_function(0, boost::bind (&compute_my_var0, &my_physics));

   // Set required variable var(1)
   my_physics.set_var(VAR1 , 2.);


   // apply physics
   my_physics.compute_var(VAR0);

   Real result = my_physics.var(VAR0);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

