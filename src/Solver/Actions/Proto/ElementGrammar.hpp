// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementGrammar_hpp
#define CF_Solver_Actions_Proto_ElementGrammar_hpp

#include <boost/proto/proto.hpp>

#include "Solver/Actions/Proto/BlockAccumulator.hpp"
#include "Solver/Actions/Proto/ElementTransforms.hpp"

/// @file 
/// Grammars related to element-wise mesh operations

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {


  
/// Matches and evaluates element-wise expressions
struct ElementGrammar :
  boost::proto::or_
  <
    // Assignment to system matrix
    BlockAccumulation<ElementMath>,
    ElementMath, // Math expressions
    // Stream output
    boost::proto::when
    <
      boost::proto::shift_left< boost::proto::terminal< std::ostream & >, ElementGrammar >,
      boost::proto::_default<ElementGrammar>
    >
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementGrammar_hpp
