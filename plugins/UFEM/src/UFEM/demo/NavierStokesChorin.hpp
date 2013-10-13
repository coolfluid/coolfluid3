// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesChorin_hpp
#define cf3_UFEM_NavierStokesChorin_hpp

#include <boost/scoped_ptr.hpp>

#include "solver/Action.hpp"

#include "LibUFEMDemo.hpp"

namespace cf3 {
namespace UFEM {
namespace demo {

/// Us Chorin's method to solve the Navier-Stokes equations
class UFEM_API NavierStokesChorin : public solver::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  NavierStokesChorin ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "NavierStokesChorin"; }

  virtual void execute();

private:
  void on_regions_set();
};

} // demo
} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesChorin_hpp
