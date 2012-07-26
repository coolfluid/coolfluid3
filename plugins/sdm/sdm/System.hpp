// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/System.hpp
/// @author Willem Deconinck
///
/// This file includes the base class for Systems to be solved.

#ifndef cf3_sdm_System_hpp
#define cf3_sdm_System_hpp

#include "math/MatrixTypes.hpp"
#include "sdm/LibSDM.hpp"

namespace cf3 {

namespace mesh{ class Cells; }

namespace sdm {

/// @brief Base class for System definitions
///
/// Systems such as Backward Euler, BDF2, ESDIRK, ...
/// can be derived from this component. 
/// Inherited classes need to implement the functions
/// - compute_lhs()
/// - compute_rhs()
class sdm_API System: public common::Component
{
public:

  // Unfortunately, a factory called System already exists in the math library, so give it different name!
  static std::string type_name() { return "ImplicitSystem"; }
  System(const std::string& name) : common::Component(name) {}
  virtual ~System() {}

  virtual bool loop_cells(const Handle<mesh::Cells const>& cells) = 0;
  virtual void compute_lhs(const Uint elem, RealMatrix& lhs) = 0;
  virtual void compute_rhs(const Uint elem, RealVector& rhs) = 0;
  virtual void prepare() = 0;
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

#endif // cf3_sdm_System_hpp
