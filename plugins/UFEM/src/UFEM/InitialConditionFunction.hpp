// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_InitialConditionFunction_hpp
#define cf3_UFEM_InitialConditionFunction_hpp


#include "ParsedFunctionExpression.hpp"

#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// InitialConditionFunction for UFEM problems, setting variables to a constant value
class UFEM_API InitialConditionFunction : public ParsedFunctionExpression
{

public: // functions

  /// Contructor
  /// @param name of the component
  InitialConditionFunction ( const std::string& name );

  virtual ~InitialConditionFunction();

  /// Get the class name
  static std::string type_name () { return "InitialConditionFunction"; }

private:
  /// Triggered when the tag or variable is changed
  void trigger();
};

} // UFEM
} // cf3


#endif // cf3_UFEM_InitialConditionFunction_hpp
