// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_solver_CWorker_hpp
#define cf3_Tools_solver_CWorker_hpp

#include "common/Component.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace solver {

////////////////////////////////////////////////////////////////////////////

class CWorker : public common::Component
{
public:

  CWorker( const std::string & name );

  /// Destructor.
  virtual ~CWorker();

  /// Returns the class name.
  static std::string type_name() { return "CWorker"; }

  /// @name SIGNALS
  //@{

  void signal_solve( common::SignalArgs & args );

  //@} END SIGNALS
};


////////////////////////////////////////////////////////////////////////////

} // Tools
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_solver_CWorker_hpp
