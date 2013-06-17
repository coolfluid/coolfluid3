// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_PressureSystem_hpp
#define cf3_UFEM_PressureSystem_hpp

#include "solver/Time.hpp"

#include "../LibUFEM.hpp"
#include "../LSSActionUnsteady.hpp"

namespace cf3 {
namespace solver { class Time; }
namespace UFEM {

/// Builds the pressure system matrix
class UFEM_API PressureSystem : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  PressureSystem ( const std::string& name );

  virtual ~PressureSystem();

  /// Get the class name
  static std::string type_name () { return "PressureSystem"; }

private:
  virtual void do_create_lss(common::PE::CommPattern &cp, const math::VariablesDescriptor &vars, std::vector<Uint> &node_connectivity, std::vector<Uint> &starting_indices, const std::vector<Uint> &periodic_links_nodes, const std::vector<bool> &periodic_links_active);
};

} // UFEM
} // cf3


#endif // cf3_UFEM_PressureSystem_hpp
