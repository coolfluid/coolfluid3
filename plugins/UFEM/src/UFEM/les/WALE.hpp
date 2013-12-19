// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_EulerDNS_hpp
#define cf3_UFEM_EulerDNS_hpp

#include "../InitialConditions.hpp"

#include "LibUFEMLES.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

namespace cf3 {
namespace UFEM {
namespace les {

/// WALE LES model
class WALE : public solver::actions::Proto::ProtoAction
{
public: // functions
  /// Contructor
  /// @param name of the component
  WALE( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "WALE"; }

private:
  void trigger_set_expression();
  void trigger_initial_conditions();
  virtual void on_regions_set();
  Real m_cw;
  Handle<InitialConditions> m_initial_conditions;
  Handle<common::Component> m_node_valence;
};

} // les
} // UFEM
} // cf3


#endif // cf3_UFEM_EulerDNS_hpp
