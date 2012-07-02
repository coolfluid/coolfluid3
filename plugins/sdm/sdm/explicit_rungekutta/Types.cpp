// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/std/vector.hpp>

#include "common/Builder.hpp"

#include "sdm/explicit_rungekutta/Types.hpp"

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {

using namespace cf3::common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

// Register the component types in the common::Action factory, so they can be
// dynamically created
ComponentBuilder < ForwardEuler,  common::Action, LibExplicitRungeKutta > ForwardEuler_Builder;
ComponentBuilder < Heun2,         common::Action, LibExplicitRungeKutta > Heun2_Builder;
ComponentBuilder < MidPoint,      common::Action, LibExplicitRungeKutta > MidPoint_Builder;
ComponentBuilder < ClassicRK33,   common::Action, LibExplicitRungeKutta > ClassicRK33_Builder;
ComponentBuilder < Heun3,         common::Action, LibExplicitRungeKutta > Heun3_Builder;
ComponentBuilder < ClassicRK44,   common::Action, LibExplicitRungeKutta > ClassicRK44_Builder;
ComponentBuilder < SSPRK54,       common::Action, LibExplicitRungeKutta > SSPRK54_Builder;
ComponentBuilder < RK65,          common::Action, LibExplicitRungeKutta > RK65_Builder;
ComponentBuilder < RKF65,    common::Action, LibExplicitRungeKutta > RKF65_Builder;

////////////////////////////////////////////////////////////////////////////////

// Definitions of the Butcher tableaux.

namespace butcher_tableau {



// 1st-order methods
ForwardEuler::ForwardEuler()
{
  order     = 1;
  nb_stages = 1;

  a += 0.;

  b += 1.;
}




// 2nd-order methods
Heun2::Heun2()
{
  order     = 2;
  nb_stages = 2;

  a +=  0.,    0.,
        2./3., 0.;

  b +=  1./3., 3./4.;
}





MidPoint::MidPoint()
{
  order     = 2;
  nb_stages = 2;

  a +=  0.,    0.,
        1./2., 0.;

  b +=  0., 1.;
}




// 3rd-order methods
ClassicRK33::ClassicRK33()
{
  order     = 3;
  nb_stages = 3;

  a +=  0.,    0.,    0.,
        1./2., 0.,    0.,
       -1.,    2.,    0.;

  b +=  1./6., 2./3., 1./6.;
}





Heun3::Heun3()
{
  order     = 3;
  nb_stages = 3;

  a +=  0.,    0.,    0.,
        1./3., 0.,    0.,
        0.,    2./3., 0.; 

  b +=  1./4., 0., 3./4.;
}




// 4th-order methods
ClassicRK44::ClassicRK44()
{
  order     = 4;
  nb_stages = 4;

  a +=  0.,    0.,    0.,    0.,
        1./2., 0.,    0.,    0.,
        0.,    1./2., 0.,    0.,
        0.,    0.,    1.,    0.;

  b +=  1./6., 1./3., 1./3., 1./6.;
}





SSPRK54::SSPRK54()
{
  order     = 4;
  nb_stages = 4;

  a +=  0.,               0.,               0.,               0.,               0.,
        0.39175222700392, 0.,               0.,               0.,               0.,
        0.21766909633821, 0.36841059262959, 0.,               0.,               0.,
        0.08269208670950, 0.13995850206999, 0.25189177424738, 0.,               0.,
        0.06796628370320, 0.11503469844438, 0.20703489864929, 0.54497475021237, 0.;

  b +=  0.14681187618661, 0.24848290924556, 0.10425883036650, 0.27443890091960, 0.22600748319395;
}





// 5th-order methods
RK65::RK65()
{
  order     = 5;
  nb_stages = 6;

  a +=  0.,     0.,      0.,    0.,      0.,    0.,
        1./4.,  0.,      0.,    0.,      0.,    0.,
        1./8,   1./8.,   0.,    0.,      0.,    0.,
        0.,     0.,      1./2., 0.,      0.,    0.,
        3./16., -3./8.,  3./8., 9./16.,  0.,    0.,
        -3./7., 8./7.,   6./7., -12./7., 8./7., 0.;

  b +=  7./90., 0., 32./90., 12./90., 32./90., 7./9.;
}





RKF65::RKF65()
{
  order     = 5;
  nb_stages = 6;

  a +=  0.,          0.,           0.,           0.,           0.,      0.,
        1./4.,       0.,           0.,           0.,           0.,      0.,
        3./32.,      9./32.,       0.,           0.,           0.,      0.,
        1932./2197., -7200./2197., 7296./2197.,  0.,           0.,      0.,
        439./216.,   -8.,          3680./513.,   -845./4104.,  0.,      0.,
        -8./27.,     2.,           -3544./2565., 1859./4104., -11./40., 0.;

  b +=  16./135., 0., 6656./12825., 28561./56430.,-9./50.,2./55.;
}





} // butcher_tableau

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3
