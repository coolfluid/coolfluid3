// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ParsedFunctionExpression_hpp
#define cf3_UFEM_ParsedFunctionExpression_hpp

#include "UFEM/LibUFEM.hpp"

#include "common/Option.hpp"

#include "math/VectorialFunction.hpp"

#include "solver/Time.hpp"
#include "solver/actions/Proto/Functions.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

namespace cf3 {
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////////////////

class UFEM_API ParsedFunctionExpression : public solver::actions::Proto::ProtoAction
{
public:
  ParsedFunctionExpression(const std::string& name);
  ~ParsedFunctionExpression();

  static std::string type_name() { return "ParsedFunctionExpression"; }

  /// Get the held function as a vector
  const solver::actions::Proto::VectorFunction& vector_function()
  {
    return m_function;
  }

  /// Get the stored function as a scalar. This requires that the values option has exactly one element
  const solver::actions::Proto::ScalarFunction& scalar_function();

private:
  void trigger_value();
  void trigger_time_component();
  void trigger_time();
  common::Option::TriggerID m_time_trigger_id;

  // Can also represent a vector function
  solver::actions::Proto::VectorFunction m_function;

  Handle<solver::Time> m_time;
};

////////////////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3

#endif // cf3_UFEM_ParsedFunctionExpression_hpp
