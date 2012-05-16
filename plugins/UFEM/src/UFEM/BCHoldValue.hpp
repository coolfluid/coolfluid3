// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BCHoldValue_hpp
#define cf3_UFEM_BCHoldValue_hpp


#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"


#include "LibUFEM.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Boundary condition to hold the value of a field at a value given by another (or the same) field
class UFEM_API BCHoldValue : public solver::actions::Proto::ProtoAction
{
public:

  /// Contructor
  /// @param name of the component
  BCHoldValue ( const std::string& name );
  
  virtual ~BCHoldValue();

  /// Get the class name
  static std::string type_name () { return "BCHoldValue"; }

  /// Sets the tags and variable names used in the expression
  void set_tags(const std::string& from_field_tag, const std::string& to_field_tag, const std::string& from_variable, const std::string& to_variable);
private:
  void signal_set_tags(common::SignalArgs& node);
  void signature_set_tags(common::SignalArgs& node);
  
  /// Placeholder for the dirichlet BC
  cf3::solver::actions::Proto::DirichletBC m_dirichlet;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BCHoldValue_hpp
