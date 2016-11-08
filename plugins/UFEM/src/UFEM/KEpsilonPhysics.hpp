// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_KEpsilonPhysics_hpp
#define cf3_UFEM_KEpsilonPhysics_hpp

#include "NavierStokesPhysics.hpp"
#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// Physical model for the Navier Stokes equations
class UFEM_API KEpsilonPhysics : public NavierStokesPhysics
{
public:
  KEpsilonPhysics(const std::string& name);

  static std::string type_name () { return "KEpsilonPhysics"; }

private:
  void trigger_recompute();
  void trigger_yplus();
  void trigger_utau();
  bool m_recursing = false;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_KEpsilonPhysics_hpp
