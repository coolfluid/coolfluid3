// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CPlotter_hpp
#define CF_Common_CPlotter_hpp

#include "Common/Component.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

class Common_API CPlotter : public Component
{
public: // typedefs

  typedef boost::shared_ptr<CPlotter> Ptr;
  typedef boost::shared_ptr<CPlotter const> ConstPtr;

public:

  CPlotter(const std::string & name);

  static std::string type_name() { return "CPlotter"; }

  /// @name SIGNALS
  //@{

  void signal_create_xyplot( Signal::arg_t & args);

  void signature_create_xyplot( Signal::arg_t & args);

  //@} END SIGNALS


}; // CPlotter

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CPlotter_hpp
