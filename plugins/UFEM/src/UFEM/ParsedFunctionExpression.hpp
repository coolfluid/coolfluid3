// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ParsedFunctionExpression_hpp
#define cf3_UFEM_ParsedFunctionExpression_hpp

#include "UFEM/LibUFEM.hpp"

#include "math/VectorialFunction.hpp"

#include "solver/actions/Proto/Functions.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

namespace cf3 {
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////////////////

class UFEM_API ParsedFunctionExpression : public solver::actions::Proto::ProtoAction
{
public:
  ParsedFunctionExpression(const std::string& name);

  static std::string type_name() { return "ParsedFunctionExpression"; }

  /// Get the held function as a vector
  const math::VectorialFunction& vector_function()
  {
    return m_function;
  }

  /// Get the stored function as a scalar. This requires that the values option has exactly one element
  const solver::actions::Proto::ScalarFunction& scalar_function();

private:
  void trigger_value();

  // Can also represent a vector function
  solver::actions::Proto::ScalarFunction m_function;
};

////////////////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3

#endif // cf3_UFEM_ParsedFunctionExpression_hpp
