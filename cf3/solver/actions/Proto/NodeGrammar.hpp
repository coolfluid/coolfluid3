// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_NodeGrammar_hpp
#define cf3_solver_actions_Proto_NodeGrammar_hpp

#include "DirichletBC.hpp"
#include "EigenTransforms.hpp"
#include "ElementOperations.hpp"
#include "ExpressionGroup.hpp"
#include "Functions.hpp"
#include "NeumannBC.hpp"
#include "NodeData.hpp"
#include "SolutionVector.hpp"
#include "Transforms.hpp"

/// @file
/// Grammar for node-based expressions

namespace cf3 {
namespace solver {
namespace actions {
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

/// Modify only a single component of a field
template<typename IndexGrammarT>
struct IndexedNodeAssign :
boost::proto::transform< IndexedNodeAssign<IndexGrammarT> >
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
      data.var_data( boost::proto::value( boost::proto::left(boost::proto::left(expr)) ) ).set_value_component( typename boost::proto::tag_of<ExprT>::type(), state, IndexGrammarT()(boost::proto::right(boost::proto::left(expr)), state, data) );
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

template<typename GrammarT, typename IndexGrammarT>
struct IndexedNodeAssignmentCases
{
  template<typename Tag, int Dummy = 0> struct case_ : boost::proto::not_<boost::proto::_> {};

  template<int Dummy> struct case_<boost::proto::tag::assign, Dummy> : boost::proto::assign< boost::proto::subscript<FieldTypes, IndexGrammarT>, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::plus_assign, Dummy> : boost::proto::plus_assign<boost::proto::subscript<FieldTypes, IndexGrammarT>, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::minus_assign, Dummy> : boost::proto::minus_assign<boost::proto::subscript<FieldTypes, IndexGrammarT>, GrammarT> {};
};


template<typename GrammarT, typename IndexGrammarT>
struct NodeAssignGrammar :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::switch_< NodeAssignmentCases<GrammarT> >,
      NodeAssign(boost::proto::_expr, GrammarT(boost::proto::_right))
    >,
    boost::proto::when
    <
      boost::proto::switch_< IndexedNodeAssignmentCases<GrammarT, IndexGrammarT> >,
      boost::proto::call< IndexedNodeAssign<IndexGrammarT> >(boost::proto::_expr, GrammarT(boost::proto::_right))
    >
  >
{
};

/// Forward declaration
struct NodeMath;

/// Matches expressions that can be used as terms in math formulas for element expressions
struct NodeMathBase :
  boost::proto::or_
  <
    boost::proto::or_<MathTerminals, ParsedFunctionGrammar, boost::proto::terminal< IndexTag<boost::proto::_> > >, // Scalars and matrices
    // Value of numbered variables
    boost::proto::when
    <
      boost::proto::terminal< Var< boost::proto::_, boost::proto::_ > >,
      VarValue(boost::proto::_value)
    >,
    CoordinatesGrammar,
    SolutionVectorGrammar
  >
{
};

struct NodeMath :
  boost::proto::or_
  <
    NodeMathBase,
    NodeAssignGrammar<NodeMath, boost::proto::or_<Integers, boost::proto::terminal< IndexTag<boost::proto::_> > > >,
    EigenMath<NodeMath, boost::proto::or_<Integers, boost::proto::terminal< IndexTag<boost::proto::_> > > >
    
  >
{
};

template<typename I, typename J>
struct NodeMathIndexed :
boost::proto::or_
<
  IndexValues<I, J>,
  NodeMathBase,
  NodeAssignGrammar<boost::proto::call< NodeMathIndexed<I,J> >, boost::proto::or_<Integers, IndexValues<I, J> > >,
  EigenMath<boost::proto::call< NodeMathIndexed<I,J> >, boost::proto::or_<Integers, IndexValues<I, J> > >
  
>
{
};

template<typename I, typename J>
struct NodeStreamOutputIndexed : StreamOutput< NodeMathIndexed<I, J> >
{
};

/// Matches and evaluates element-wise expressions
struct SingleExprNodeGrammar :
  boost::proto::or_
  <
    DirichletBCGrammar<NodeMath>,
    NeumannBCGrammar<NodeMath>,
    boost::proto::when
    <
      NodeMath,
      IndexLooper<NodeMathIndexed>
    >,
    boost::proto::when
    <
      StreamOutput<NodeMath>,
      IndexLooper<NodeStreamOutputIndexed>
    >
  >
{
};

struct NodeGrammar :
  boost::proto::or_
  <
    SingleExprNodeGrammar,
    GroupGrammar< SingleExprNodeGrammar >
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_NodeGrammar_hpp
