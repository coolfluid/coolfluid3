// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ForwardEuler, common::Action, LibExplicitRungeKutta > ForwardEuler_Builder;
common::ComponentBuilder < Heun,         common::Action, LibExplicitRungeKutta > Heun_Builder;
common::ComponentBuilder < RK3,          common::Action, LibExplicitRungeKutta > RK3_Builder;
common::ComponentBuilder < ClassicRK4,   common::Action, LibExplicitRungeKutta > ClassicRK4_Builder;

////////////////////////////////////////////////////////////////////////////////

namespace butcher_tableau {

using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

ForwardEuler::ForwardEuler()
{
  order     = 1;
  nb_stages = 1;

  a += 0.;

  b += 1.;
}

////////////////////////////////////////////////////////////////////////////////

Heun::Heun()
{
  order     = 2;
  nb_stages = 2;

  a +=  0.,    0.,
        2./3., 0.;

  b +=  1./3., 3./4.;
}

////////////////////////////////////////////////////////////////////////////////

RK3::RK3()
{
  order     = 3;
  nb_stages = 3;

  a +=  0.,    0.,    0.,
        1./2., 0.,    0.,
       -1.,    2.,    0.;

  b +=  1./6., 2./3., 1./6.;
}

////////////////////////////////////////////////////////////////////////////////

ClassicRK4::ClassicRK4()
{
  order     = 4;
  nb_stages = 4;

  a +=  0.,    0.,    0.,    0.,
        1./2., 0.,    0.,    0.,
        0.,    1./2., 0.,    0.,
        0.,    0.,    1.,    0.;

  b +=  1./6., 1./3., 1./3., 1./6.;
}

////////////////////////////////////////////////////////////////////////////////

} // butcher_tableau

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3
