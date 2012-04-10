// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_test_NavierStokes_hpp
#define cf3_UFEM_test_NavierStokes_hpp

#include "solver/actions/Proto/Expression.hpp"

/// @file Precompiled expressions for the different steps in the Navier-Stokes solver development

namespace cf3 {
namespace UFEM {

class LSSActionUnsteady;
class SUPGCoeffs;

/// Assembly for the Stokes equations, stabilized with artificial dissipation
boost::shared_ptr< solver::actions::Proto::Expression > stokes_artifdiss(LSSActionUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Stokes equations, stabilized with PSPG
boost::shared_ptr< solver::actions::Proto::Expression > stokes_pspg(LSSActionUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Navier-Stokes equations, stabilized with PSPG
boost::shared_ptr< solver::actions::Proto::Expression > navier_stokes_pspg(LSSActionUnsteady& solver, SUPGCoeffs& coefs);

/// Assembly for the Navier-Stokes equations, stabilized with SUPG
boost::shared_ptr< solver::actions::Proto::Expression > navier_stokes_supg(LSSActionUnsteady& solver, SUPGCoeffs& coefs);

}
}

#endif // cf3_UFEM_test_NavierStokes_hpp
