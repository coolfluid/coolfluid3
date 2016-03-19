//#ifndef SCALARADVECTION_H
//#define SCALARADVECTION_H

//#endif // SCALARADVECTION_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BoussinesqConcentration_hpp
#define cf3_UFEM_BoussinesqConcentration_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LSSActionUnsteady.hpp"

#include "CrossWindDiffusion.hpp"
#include "SUPG.hpp"

namespace cf3 {

namespace UFEM {

/// solver for scalar transport
class UFEM_API BoussinesqConcentration : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  BoussinesqConcentration ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "BoussinesqConcentration"; }

private:
  void on_initial_conditions_set(InitialConditions& initial_conditions);
  void trigger_assembly();

  /// Parameter for the theta scheme
  Real m_theta = 0.5;

  /// Stabilization coefficient
  Real tau_su;

  /// Density of the dispersed gas
  Real density = 1.;

  ComputeTau compute_tau;

  CrosswindDiffusion cw;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BoussinesqConcentration_hpp
