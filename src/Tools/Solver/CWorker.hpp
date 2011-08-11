// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_Solver_CWorker_hpp
#define CF_Tools_Solver_CWorker_hpp

#include "Common/Component.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Solver {

////////////////////////////////////////////////////////////////////////////

class CWorker : public Common::Component
{
public: // typedefs

  typedef boost::shared_ptr<CWorker> Ptr;
  typedef boost::shared_ptr<const CWorker> ConstPtr;

public:

  CWorker( const std::string & name );

  /// Destructor.
  virtual ~CWorker();

  /// Returns the class name.
  static std::string type_name() { return "CWorker"; }

  /// @name SIGNALS
  //@{

  void signal_solve( Common::SignalArgs & args );

  //@} END SIGNALS
};


////////////////////////////////////////////////////////////////////////////

} // Tools
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Solver_CWorker_hpp
