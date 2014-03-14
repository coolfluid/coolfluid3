// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_RelaxationTime_hpp
#define cf3_UFEM_RelaxationTime_hpp

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "LibUFEMParticles.hpp"

namespace cf3 {
namespace UFEM {
namespace particles {

/// Compute the particle relaxation time
class RelaxationTime : public solver::actions::Proto::ProtoAction
{
public: // functions

  /// Contructor
  /// @param name of the component
  RelaxationTime ( const std::string& name );
  
  virtual ~RelaxationTime();

  /// Get the class name
  static std::string type_name () { return "RelaxationTime"; }

private:
  virtual void on_regions_set();
};

} // particles
} // UFEM
} // cf3


#endif // cf3_UFEM_RelaxationTime_hpp
