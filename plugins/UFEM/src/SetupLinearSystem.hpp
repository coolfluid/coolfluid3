// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_SetupLinearSystem_hpp
#define CF_UFEM_SetupLinearSystem_hpp

#include "Common/Component.hpp"

#include "LibUFEM.hpp"

namespace CF {
namespace UFEM {

/// Wizard to set up steady linear heat conduction
class UFEM_API SetupLinearSystem : public Common::Component
{
public: // typedefs

  typedef boost::shared_ptr<SetupLinearSystem> Ptr;
  typedef boost::shared_ptr<SetupLinearSystem const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SetupLinearSystem ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "SetupLinearSystem"; }

  // functions specific to the HeatConductionLinearSteady component

  /// @name SIGNALS
  //@{

  /// Signal to create a model
  void create_model ( Common::SignalArgs& node );

  void create_model_signature ( Common::SignalArgs& node );

  //@} END SIGNALS
};

} // UFEM
} // CF


#endif // CF_UFEM_SetupLinearSystem_hpp
