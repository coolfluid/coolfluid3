// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Solver/Tags.hpp"

#include "RDM/LibRDM.hpp"

#ifndef CF_RDM_Tags_hpp
#define CF_RDM_Tags_hpp

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

/// Class defines the tags for the RDM components
/// @author Tiago Quintino
class RDM_API Tags : public Solver::Tags {
public:

  static const char * fields()         { return "Fields";  }
  static const char * actions()        { return "Actions"; }

  static const char * solution()      { return "solution"; }
  static const char * residual()      { return "residual"; }
  static const char * wave_speed()    { return "wave_speed"; }
  static const char * dual_area()     { return "dual_area"; }

  static const char * update_vars()   { return "update_vars"; }

}; // Tags

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_Tags_hpp
