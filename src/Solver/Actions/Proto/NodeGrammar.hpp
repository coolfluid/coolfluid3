// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_NodeGrammar_hpp
#define CF_Solver_Actions_Proto_NodeGrammar_hpp

#include "DirichletBC.hpp"
#include "EigenTransforms.hpp"
#include "NeumannBC.hpp"
#include "NodeData.hpp"
#include "Transforms.hpp"
#include "ElementVariables.hpp"

/// @file
/// Grammar for node-based expressions

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Provide access to the geometry coordinates in case of node expressions
struct GetCoordinates :
  boost::proto::transform< GetCoordinates >
{
  template<typename TagT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<TagT, StateT, DataT>
  {
    /// The geometric support is also a functor that adheres to the TR1 result_of protocol, so we can easily determine the result type in a generic way
    typedef const typename boost::remove_reference<DataT>::type::CoordsT& result_type;
  
    result_type operator ()(
                typename impl::expr_param tag
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      return data.coordinates();
    }
  };
};

/// Valid terminals that can represent the current node coordinates
struct CoordsTerminals :
  boost::proto::terminal< SFOp<CoordinatesOp> >
{
};

struct CoordinatesGrammar :
  boost::proto::or_
  <
    boost::proto::when
    <
      CoordsTerminals,
      GetCoordinates
    >
  >
{
};

/// Forward declaration
struct NodeMath;

/// Matches expressions that can be used as terms in math formulas for element expressions
struct NodeMath :
  boost::proto::or_
  <
    MathTerminals, // Scalars and matrices
    EigenMath<NodeMath>, // Special Eigen functions and Eigen multiplication (overrides default product)
    CoordinatesGrammar,
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
    StreamOutput<NodeGrammar>
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_NodeGrammar_hpp
