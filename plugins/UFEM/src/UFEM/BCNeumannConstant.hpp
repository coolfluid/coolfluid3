// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BCNeumannConstant_hpp
#define cf3_UFEM_BCNeumannConstant_hpp

#include "solver/actions/Proto/ProtoAction.hpp"

#include <solver/actions/Proto/BlockAccumulator.hpp>

#include "LibUFEM.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Boundary condition to hold the value of a field at a value given by another (or the same) field
class UFEM_API BCNeumannConstant : public solver::actions::Proto::ProtoAction
{
public:

  /// Contructor
  /// @param name of the component
  BCNeumannConstant ( const std::string& name );
  
  virtual ~BCNeumannConstant();

  /// Get the class name
  static std::string type_name () { return "BCNeumannConstant"; }

  /// Sets the tags and variable names used in the expression
  void set_tags(const std::string& neumann_variable, const std::string& neumann_field );
private:
  void signal_set_tags(common::SignalArgs& node);
  void signature_set_tags(common::SignalArgs& node);
  Real m_flux;
  cf3::solver::actions::Proto::SystemRHS m_rhs;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BCHoldValue_hpp
