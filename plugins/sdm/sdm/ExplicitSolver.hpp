// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ExplicitSolver_hpp
#define cf3_sdm_ExplicitSolver_hpp

#include "sdm/Solver.hpp"

#include "sdm/LibSDM.hpp"

namespace cf3 {
namespace sdm {

/////////////////////////////////////////////////////////////////////////////////////

class sdm_API ExplicitSolver : public sdm::Solver {

public: // functions

  /// Contructor
  /// @param name of the component
  ExplicitSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~ExplicitSolver() {}

  /// Get the class name
  static std::string type_name () { return "ExplicitSolver"; }

  virtual void setup() ;

  virtual void step() ;

};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_ExplicitSolver_hpp
