// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_NodeGrammar_hpp
#define CF_Solver_Actions_Proto_NodeGrammar_hpp

#include "Solver/Actions/Proto/DirichletBC.hpp"
#include "Solver/Actions/Proto/EigenTransforms.hpp"
#include "Solver/Actions/Proto/NeumannBC.hpp"
#include "Solver/Actions/Proto/NodeData.hpp"
#include "Solver/Actions/Proto/Transforms.hpp"

/// @file
/// Grammar for node-based expressions

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Forward declaration
struct NodeMath;
  
/// Matches expressions that can be used as terms in math formulas for element expressions
struct NodeMath :
  boost::proto::or_
  <
    MathTerminals, // Scalars and matrices
    EigenMath<NodeMath>, // Special Eigen functions and Eigen multiplication (overrides default product)
    // Default evaluation of certain math expressions
    MathOpDefault<NodeMath>
  >
{
};
  
/// Matches and evaluates element-wise expressions
struct NodeGrammar :
  boost::proto::or_
  <
    // Dirichlet BC
    boost::proto::when
    <
      DirichletBCGrammar,
      DirichletBCSetter(boost::proto::_child_c<1>(boost::proto::_left), NodeMath(boost::proto::_right))
    >,
    // Neumann BC
    boost::proto::when
    <
      NeumannBCGrammar,
      NeumannBCSetter(boost::proto::_child_c<1>(boost::proto::_left), NodeMath(boost::proto::_right))
    >,
    NodeMath, // Math expressions
    boost::proto::when
    <
      boost::proto::shift_left< boost::proto::terminal< std::ostream & >, NodeGrammar >,
      boost::proto::_default<NodeGrammar>
    >
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_NodeGrammar_hpp
