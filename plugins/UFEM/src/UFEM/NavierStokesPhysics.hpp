// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesPhysics_hpp
#define cf3_UFEM_NavierStokesPhysics_hpp

#include "physics/DynamicModel.hpp"
#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// Physical model for the Navier Stokes equations
class UFEM_API NavierStokesPhysics : public physics::DynamicModel
{
public:
  NavierStokesPhysics(const std::string& name);

  static std::string type_name () { return "NavierStokesPhysics"; }

private:
  void trigger_rho();
  void trigger_mu();
  void trigger_nu();
  bool m_recursing;
  Real m_rho, m_mu;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesPhysics_hpp
