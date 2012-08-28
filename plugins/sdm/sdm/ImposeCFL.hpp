// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ImposeCFL_hpp
#define cf3_sdm_ImposeCFL_hpp

#include "sdm/TimeIntegrationStepComputer.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {


class sdm_API ImposeCFL : public sdm::TimeIntegrationStepComputer
{
public: // functions
  /// Contructor
  /// @param name of the component
  ImposeCFL ( const std::string& name );

  /// Virtual destructor
  virtual ~ImposeCFL() {};

  /// Get the class name
  static std::string type_name () { return "ImposeCFL"; }

  /// execute the action
  virtual void execute ();

};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_ImposeCFL_hpp
