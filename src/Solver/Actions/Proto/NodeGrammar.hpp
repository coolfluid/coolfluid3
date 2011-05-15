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
#include "ElementOperations.hpp"

/// @file
/// Grammar for node-based expressions

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Provide access to the geometry coordinates in case of node expressions
struct GetCoordinates :
  boost::proto::transform<GetCoordinates>
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

/// Handle modification of a field
struct NodeAssign :
  boost::proto::transform<NodeAssign>
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;
  
    result_type operator ()(
                typename impl::expr_param expr // The expression
              , typename impl::state_param state // The evaluated RHS
              , typename impl::data_param data
    ) const
    {
      data.var_data( boost::proto::value( boost::proto::left(expr) ) ).set_value( typename boost::proto::tag_of<ExprT>::type(), state );
    }
  };
};


template<typename GrammarT>
struct NodeAssignmentCases
{
  template<typename Tag, int Dummy = 0> struct case_ : boost::proto::not_<boost::proto::_> {};
  
  template<int Dummy> struct case_<boost::proto::tag::assign, Dummy> : boost::proto::assign<FieldTypes, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::plus_assign, Dummy> : boost::proto::plus_assign<FieldTypes, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::minus_assign, Dummy> : boost::proto::minus_assign<FieldTypes, GrammarT> {};
};


template<typename GrammarT>
struct NodeAssignGrammar :
  boost::proto::when
  <
    boost::proto::switch_< NodeAssignmentCases<GrammarT> >,
    NodeAssign(boost::proto::_expr, GrammarT(boost::proto::_right))
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
    // Value of numbered variables
    boost::proto::when
    <
      boost::proto::terminal< Var< boost::proto::_, boost::proto::_ > >,
      VarValue(boost::proto::_value)
    >,
    CoordinatesGrammar,
    NodeAssignGrammar<NodeMath>,
    EigenMath<NodeMath, Integers> // Special Eigen functions and Eigen multiplication (overrides default product)
  >
{
};
  
/// Matches and evaluates element-wise expressions
struct NodeGrammar :
  boost::proto::or_
  <
    DirichletBCGrammar<NodeMath>,
    NeumannBCGrammar<NodeMath>,
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
