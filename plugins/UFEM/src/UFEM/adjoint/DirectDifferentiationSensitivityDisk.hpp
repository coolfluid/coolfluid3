// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_DirectDifferentiationSensitivityDisk_hpp
#define cf3_UFEM_DirectDifferentiationSensitivityDisk_hpp


#include "solver/Action.hpp"
#include <solver/actions/Proto/BlockAccumulator.hpp>

#include "LibUFEMAdjoint.hpp"

#include "../LSSAction.hpp"



namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {
namespace adjoint {

class UFEM_API DirectDifferentiationSensitivityDisk : public solver::Action
{
public:

  /// Contructor
  /// @param name of the component
  DirectDifferentiationSensitivityDisk ( const std::string& name );

  virtual ~DirectDifferentiationSensitivityDisk();

  /// Get the class name
  static std::string type_name () { return "DirectDifferentiationSensitivityDisk"; }

  virtual void execute();

private:
  void trigger_ct();


  Real m_ct;
  Real m_a;
  Real m_th = 0.0;
  Real m_area = 0.0;
  Uint m_nDisk = 1;
};

} // adjointtube
} // UFEM
} // cf3


#endif // cf3_UFEM_ComputeFluxFluid_hpp
