// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CPlotter_hpp
#define cf3_solver_CPlotter_hpp

#include "common/Component.hpp"

#include "solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

class solver_API CPlotter : public common::Component
{
public: // typedefs

  
  

public:

  CPlotter(const std::string & name);

  static std::string type_name() { return "CPlotter"; }

  void set_data_set(const common::URI & uri);

  /// @name SIGNALS
  //@{

  void signal_create_xyplot( common::SignalArgs & args);

  void signature_create_xyplot( common::SignalArgs & args);

  //@} END SIGNALS

private: // data

  common::URI m_data;

}; // CPlotter

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CPlotter_hpp
