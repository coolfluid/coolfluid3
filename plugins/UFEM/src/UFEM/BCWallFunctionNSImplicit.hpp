// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BCWallFunctionNSImplicit_hpp
#define cf3_UFEM_BCWallFunctionNSImplicit_hpp


#include "solver/Action.hpp"
#include <solver/actions/Proto/BlockAccumulator.hpp>

#include "LibUFEM.hpp"

#include "LSSAction.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Calculate the heat flux from the one domain using the wall heat transfer equation
/// and apply it as a Robin boundary condition to the problem on the adjacent domain
/// The "regions" option determines the boundary on which to set the condition
/// The "gradient_region" option determines the region in which the temperature gradient is calculated
/// The "lss" option determines the linear system to which the boundary condition is applied
/// The "temperature_field_tag" option determines the tag to use when looking for the temperature field
class UFEM_API BCWallFunctionNSImplicit : public solver::Action
{
public:

  /// Contructor
  /// @param name of the component
  BCWallFunctionNSImplicit ( const std::string& name );

  virtual ~BCWallFunctionNSImplicit();

  /// Get the class name
  static std::string type_name () { return "BCWallFunctionNSImplicit"; }

  /// Execute the control of heat transfer coefficient usage (dynamic or static)
  virtual void execute();

private:
  /// Called when the boundary regions are set
  virtual void on_regions_set();

  /// Called when the "lss" or "temperature_field_tag" options are changed
  void trigger_setup();

  cf3::solver::actions::Proto::SystemRHS rhs;
  cf3::solver::actions::Proto::SystemMatrix system_matrix;

  Real m_theta = 0.5;

  Real m_c_mu = 0;
  Real m_yplus = 0;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BCWallFunctionNSImplicit_hpp
