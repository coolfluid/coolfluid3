// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_Tags_hpp
#define cf3_SFDM_Tags_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/Tags.hpp"
#include "SFDM/LibSFDM.hpp"

namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

struct SFDM_API Tags : public Solver::Tags
{
  static const char * fields();
  static const char * actions();
  static const char * solution_vars();
  static const char * input_vars();
  static const char * solution_order();

  static const char * solution();
  static const char * wave_speed();
  static const char * update_coeff();
  static const char * residual();
  static const char * jacob_det();
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_Tags_hpp
