// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_RobinUt_hpp
#define cf3_UFEM_RobinUt_hpp


#include "solver/Action.hpp"
#include <solver/actions/Proto/BlockAccumulator.hpp>

#include "LibUFEMAdjoint.hpp"

#include "../LSSAction.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {
namespace adjoint{
/// Calculate the heat flux from the one domain using the wall heat transfer equation
/// and apply it as a Robin boundary condition to the problem on the adjacent domain
/// The "regions" option determines the boundary on which to set the condition
/// The "gradient_region" option determines the region in which the temperature gradient is calculated
/// The "lss" option determines the linear system to which the boundary condition is applied
/// The "temperature_field_tag" option determines the tag to use when looking for the temperature field
class UFEM_API RobinUt : public solver::Action
{
public:

  /// Contructor
  /// @param name of the component
  RobinUt ( const std::string& name );
  
  virtual ~RobinUt();

  /// Get the class name
  static std::string type_name () { return "RobinUt"; }

  /// Execute the control of heat transfer coefficient usage (dynamic or static)
  virtual void execute();

private:
  /// Called when the boundary regions are set
  virtual void on_regions_set();


  Handle<math::LSS::System> m_lss;
  cf3::solver::actions::Proto::SystemRHS system_rhs;
  cf3::solver::actions::Proto::SystemMatrix system_matrix;
  Uint m_u_index1 = 0;
  Uint m_u_index2 = 0;
};
} // adjoint
} // UFEM
} // cf3


#endif // cf3_UFEM_RobinUt_hpp
