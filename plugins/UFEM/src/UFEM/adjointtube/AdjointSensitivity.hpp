// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_AdjointSensitivity_hpp
#define cf3_UFEM_AdjointSensitivity_hpp


#include "solver/Action.hpp"

#include "LibUFEMAdjointTube.hpp"

#include "../LSSAction.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {
namespace adjointtube {

/// Calculate the heat flux from the one domain and apply is as a Neumann
/// boundary condition to problem on the adjacent domain
/// The "regions" option determines the boundary on which to set the condition
/// The "gradient_region" option determines the region in which the temperature gradient
/// is calculated
/// The "lss" option determines the linear system to which the boundary condition is applied
/// The "temperature_field_tag" option determines the tag to use when looking for the temperature field
class UFEM_API AdjointSensitivity : public solver::Action
{
public:

  /// Contructor
  /// @param name of the component
  AdjointSensitivity ( const std::string& name );

  virtual ~AdjointSensitivity();

  /// Get the class name
  static std::string type_name () { return "AdjointSensitivity"; }

  virtual void execute();

protected:
  virtual void on_physical_model_changed();
};

} // adjointtube
} // UFEM
} // cf3


#endif // cf3_UFEM_ComputeFluxFluid_hpp
