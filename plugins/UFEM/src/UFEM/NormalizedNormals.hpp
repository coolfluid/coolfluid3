// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NormalizedNormals_hpp
#define cf3_UFEM_NormalizedNormals_hpp

#include "InitialConditions.hpp"

#include "LibUFEM.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3 {
namespace UFEM {

/// Compute a field containing the velocity gradient tensor and the divergence
class NormalizedNormals : public solver::actions::Proto::ProtoAction
{
public: // functions
  /// Contructor
  /// @param name of the component
  NormalizedNormals( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "NormalizedNormals"; }

  virtual void execute();

private:
  void trigger_initial_conditions();
  virtual void on_regions_set();
  Handle<InitialConditions> m_initial_conditions;
  Handle<solver::actions::Proto::ProtoAction> m_zero_fields;
  Handle<solver::actions::Proto::ProtoAction> m_normalize;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_NormalizedNormals_hpp
