// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_InitialConditions_hpp
#define CF_RDM_InitialConditions_hpp

#include "Common/CActionDirector.hpp"

#include "RDM/Core/LibCore.hpp"

namespace CF {
namespace RDM {
namespace Core {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_Core_API InitialConditions : public Common::CActionDirector {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<InitialConditions> Ptr;
  typedef boost::shared_ptr<InitialConditions const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  InitialConditions ( const std::string& name );

  /// Virtual destructor
  virtual ~InitialConditions() {}

  /// Get the class name
  static std::string type_name () { return "InitialConditions"; }

  /// execute the action
  virtual void execute ();

  /// @name SIGNALS
  //@{

  /// adds an initialization
  void signal_create_initial_condition( Common::SignalArgs& args );
  /// signature for @see signal_create_initial_condition
  void signature_signal_create_initial_condition( Common::SignalArgs& node );

  //@} END SIGNALS

};

/////////////////////////////////////////////////////////////////////////////////////

} // Core
} // RDM
} // CF

#endif // CF_RDM_InitialConditions_hpp
