// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_Proto_Functions_hpp
#define cf3_Solver_Actions_Proto_Functions_hpp

#include <boost/proto/core.hpp>

#include "common/CF.hpp"

namespace cf3 {
namespace Solver {
namespace Actions {
namespace Proto {

/// Base class of all functions that can be evaluated using "default" C++ semantics
struct FunctionBase
{
};
  
/// Pow function based on Proto docs example
template<Uint Exp>
struct pow_fun : FunctionBase
{
  typedef Real result_type;
  Real operator()(Real d) const
  {
    for(Uint i = 0; i != (Exp-1); ++i)
      d *= d;
    return d;
  }
};

template<Uint Exp, typename Arg>
typename boost::proto::result_of::make_expr<
    boost::proto::tag::function  // Tag type
  , pow_fun<Exp>          // First child (by value)
  , Arg const &           // Second child (by reference)
>::type const
pow(Arg const &arg)
{
  return boost::proto::make_expr<boost::proto::tag::function>(
      pow_fun<Exp>()    // First child (by value)
    , boost::ref(arg)   // Second child (by reference)
  );
}

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace cf3

#endif // cf3_Solver_Actions_Proto_Functions_hpp
