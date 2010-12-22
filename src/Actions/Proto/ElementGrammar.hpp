// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_Proto_ElementGrammar_hpp
#define CF_Actions_Proto_ElementGrammar_hpp

#include <boost/proto/proto.hpp>

#include "BlockAccumulator.hpp"
#include "ElementTransforms.hpp"

/// @file 
/// Grammars related to element-wise mesh operations

namespace CF {
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
} // namespace CF

#endif // CF_Actions_Proto_ElementGrammar_hpp
