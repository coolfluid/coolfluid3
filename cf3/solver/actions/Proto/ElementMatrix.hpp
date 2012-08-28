// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_solver_actions_Proto_ElementMatrix_hpp
#define cf3_solver_actions_Proto_ElementMatrix_hpp

// Default maximum number of different element matrices that can appear in an expression
#ifndef CF3_PROTO_MAX_ELEMENT_MATRICES
  #define CF3_PROTO_MAX_ELEMENT_MATRICES 5
#endif

#include<iostream>

#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>

#include <boost/mpl/max.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>

#include <boost/proto/core.hpp>

#include "math/MatrixTypes.hpp"

#include "IndexLooping.hpp"
#include "Terminals.hpp"
#include "Transforms.hpp"
#include "BlockAccumulator.hpp"

/// @file
/// System matrix block accumultation. Current prototype uses dense a dense Eigen matrix and is purely for proof-of-concept

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Represents an element matrix that is used as part of the linear system
template<typename T>
struct ElementSystemMatrix : T
{
};

/// Represents an element matrix
template<typename T>
struct ElementMatrix : T
{
};

/// Represents an element vector
template<typename T>
struct ElementVector : T
{
};

/// Some predefined element matrices (more can be user-defined, but you have to change the number in the MPL int_ so the type is long and tedious)
static boost::proto::terminal< ElementSystemMatrix< boost::mpl::int_<0> > >::type const _A = {};
static boost::proto::terminal< ElementVector< boost::mpl::int_<0> > >::type const _a = {};
static boost::proto::terminal< ElementSystemMatrix< boost::mpl::int_<1> > >::type const _T = {};
static boost::proto::terminal< ElementMatrix< boost::mpl::int_<2> > >::type const _M = {};

/// Reperesents element RHS vector
struct ElementRHS
{
};

/// Terminal for the element RHS vector ("b")
static boost::proto::terminal<ElementRHS>::type const _x = {};

/// Match only element matrices that are used in a linear system
struct ElementSystemMatrixTerm :
  boost::proto::or_
  <
    boost::proto::terminal< ElementSystemMatrix<boost::proto::_> >,
    BlockLhsGrammar<SystemRHSTag>
  >
{
};

/// Match element matrix terminals
struct ElementMatrixTerm :
  boost::proto::or_
  <
    boost::proto::terminal< ElementMatrix<boost::proto::_> >,
    ElementSystemMatrixTerm
  >
{
};

/// Match element matrix terminals
struct ElementVectorTerm :
  boost::proto::terminal< ElementVector<boost::proto::_> >
{
};

/// Match subrows
template<typename IdxT>
struct IsSubRows :
  boost::proto::function
  <
    ElementMatrixTerm,
    boost::proto::subscript< boost::proto::terminal< Var<IdxT, boost::proto::_> >, boost::proto::terminal< boost::proto::_ > >,
    boost::proto::terminal< Var<IdxT, boost::proto::_> >
  >
{
};

/// Match subrows for vectors
template<typename IdxT>
struct IsVectorSubRows :
  boost::proto::subscript
  <
    ElementVectorTerm,
    boost::proto::subscript< boost::proto::terminal< Var<IdxT, boost::proto::_> >, boost::proto::terminal< boost::proto::_ > >
  >
{
};

/// Match subrows
template<typename IdxT>
struct IsSubCols :
  boost::proto::function<ElementMatrixTerm, boost::proto::terminal< Var<IdxT, boost::proto::_> >, boost::proto::subscript< boost::proto::terminal< Var<IdxT, boost::proto::_> >, boost::proto::terminal< boost::proto::_ > > >
{
};

/// Match submatrices
template<typename IdxT>
struct IsSubMatrix :
  boost::proto::function
  <
    ElementMatrixTerm,
    boost::proto::subscript< boost::proto::terminal< Var<IdxT, boost::proto::_> >, boost::proto::terminal< boost::proto::_ > >,
    boost::proto::subscript< boost::proto::terminal< Var<IdxT, boost::proto::_> >, boost::proto::terminal< boost::proto::_ > >
  >
{
};

/// Match but don't evaluate subblock-expressions
template<typename IdxT>
struct ElementMatrixSubBlocks :
  boost::proto::or_
  <
    IsSubRows<IdxT>,
    IsSubCols<IdxT>,
    IsSubMatrix<IdxT>
  >
{
};

/// Transform to set a range with true indicating a variable that has an equation, and false for no equation
template<int I>
struct IsEquationVariable :
  boost::proto::or_
  <
    boost::proto::when // scalar field used as equation in an element matrix
    <
      boost::proto::or_
      <
        boost::proto::function<ElementSystemMatrixTerm, boost::proto::terminal< Var<boost::mpl::int_<I>, boost::proto::_> > >,
        boost::proto::function<ElementSystemMatrixTerm, boost::proto::terminal< Var<boost::mpl::int_<I>, boost::proto::_> >, boost::proto::_ >,
        boost::proto::function<ElementSystemMatrixTerm, boost::proto::_, boost::proto::terminal< Var<boost::mpl::int_<I>, boost::proto::_> > >,
        ElementMatrixSubBlocks< boost::mpl::int_<I> >
      >,
      boost::mpl::true_()
    >,
    boost::proto::when
    <
      boost::proto::terminal<boost::proto::_>,
      boost::mpl::false_()
    >,
    boost::proto::when
    <
      boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >
    , boost::proto::fold
      <
        boost::proto::_, boost::mpl::false_(), boost::mpl::max< boost::proto::_state, boost::proto::call< IsEquationVariable<I> > >()
      >
    >
  >
{
};

/// Wrap IsEquationVariable as a lambda expression
template<typename I, typename ExprT>
struct EquationVariablesOp
{
  typedef typename boost::result_of<IsEquationVariable<I::value>(ExprT)>::type type;
};

/// Calculate if a variable is part of an equation
template<typename ExprT, typename NbVarsT>
struct EquationVariables
{
  /// Types of the used variables
  typedef typename boost::fusion::result_of::as_vector
  <
    typename boost::mpl::transform
    <
      typename boost::mpl::copy<boost::mpl::range_c<int,0,NbVarsT::value>, boost::mpl::back_inserter< boost::mpl::vector_c<Uint> > >::type, //range from 0 to NbVarsT
      EquationVariablesOp<boost::mpl::_1, ExprT>
    >::type
  >::type type;
};

/// Get the width of a field varaible, based on the variable type
template<typename T, typename SF>
struct FieldWidth
{
  static const Uint value = 0;
  typedef boost::mpl::int_<value> type;
};

/// Scalars have width 1
template<typename SF>
struct FieldWidth<ScalarField, SF>
{
  static const Uint value = 1;
  typedef boost::mpl::int_<value> type;
};

/// VectorFields have the same dimension as the problem domain
template<typename SF>
struct FieldWidth<VectorField, SF>
{
  static const Uint value = SF::dimension;
  typedef boost::mpl::int_<value> type;
};

/// Given a variable's data, get the product of the number of nodes with the dimension variable (i.e. the size of the element matrix if this variable would be the only one in the problem)
template<typename VariableT, typename SF, typename SupportSF>
struct NodesTimesDim
{
  typedef boost::mpl::int_<SF::nb_nodes * FieldWidth<VariableT, SupportSF>::value> type;
};

template<typename SF, typename SupportSF>
struct NodesTimesDim<boost::mpl::void_, SF, SupportSF>
{
  typedef boost::mpl::int_<0> type;
};

/// The size of the element matrix for each variable
template<typename VariablesT, typename VariablesSFT, typename SupportSF>
struct MatrixSizePerVar
{
  typedef typename boost::mpl::eval_if
  <
    boost::mpl::is_sequence<VariablesSFT>,
    boost::mpl::transform
    <
      typename boost::mpl::copy<VariablesT, boost::mpl::back_inserter< boost::mpl::vector0<> > >::type,
      VariablesSFT,
      NodesTimesDim<boost::mpl::_1, boost::mpl::_2, SupportSF>
    >,
    boost::mpl::transform
    <
      typename boost::mpl::copy<VariablesT, boost::mpl::back_inserter< boost::mpl::vector0<> > >::type,
      NodesTimesDim<boost::mpl::_1, VariablesSFT, SupportSF>
    >
  >::type type;
};

/// Filter the matrix size so equation variables are the only ones left with non-zero size
template<typename MatrixSizesT, typename EquationVariablesT>
struct FilterMatrixSizes
{
  typedef typename boost::mpl::transform
  <
    MatrixSizesT,
    typename boost::mpl::copy<EquationVariablesT, boost::mpl::back_inserter< boost::mpl::vector0<> > >::type,
    boost::mpl::if_<boost::mpl::_2, boost::mpl::_1, boost::mpl::int_<0> >
  >::type type;
};

/// Calculate the size of the element matrix
template<typename MatrixSizesT, typename EquationVariablesT>
struct ElementMatrixSize
{
  typedef typename boost::mpl::fold
  <
    typename FilterMatrixSizes<MatrixSizesT, EquationVariablesT>::type,
    boost::mpl::int_<0>,
    boost::mpl::plus<boost::mpl::_1, boost::mpl::_2>
  >::type type;
};

/// Get a given element matrix
struct ElementMatrixValue :
  boost::proto::transform<ElementMatrixValue>
{

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<DataT>::type::ElementMatrixT& result_type;

    result_type operator ()(typename impl::expr_param, typename impl::state_param state, typename impl::data_param data) const
    {
      return data.element_matrix(state);
    }
  };
};

/// Get a given element matrix
struct ElementVectorValue :
boost::proto::transform<ElementVectorValue>
{

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<DataT>::type::ElementVectorT& result_type;

    result_type operator ()(typename impl::expr_param, typename impl::state_param state, typename impl::data_param data) const
    {
      return data.element_vector(state);
    }
  };
};

/// Only get the rows relevant for a given variable
struct ElementMatrixRowsValue :
  boost::proto::transform<ElementMatrixRowsValue>
{

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarDataType<ExprT, DataT>::type VarDataT;
    typedef typename boost::remove_reference<DataT>::type::ElementMatrixT ElementMatrixT;

    typedef Eigen::Block
    <
      ElementMatrixT,
      VarDataT::dimension * VarDataT::EtypeT::nb_nodes,
      ElementMatrixT::ColsAtCompileTime
    > result_type;

    result_type operator ()(typename impl::expr_param var, typename impl::state_param state, typename impl::data_param data) const
    {
      return data.element_matrix(state).template block<VarDataT::dimension * VarDataT::EtypeT::nb_nodes, ElementMatrixT::ColsAtCompileTime>(VarDataT::EtypeT::nb_nodes*data.var_data(var).offset, 0);
    }
  };
};

/// Only get the rows relevant for a given variable
struct ElementVectorRowsValue :
  boost::proto::transform< ElementVectorRowsValue >
{

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::child_c<ExprT, 1>::type Child1T;
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(Child1T)>::type>::type RowVarIdxT;
    typedef typename VarDataType<RowVarIdxT, DataT>::type RowVarDataT;

    typedef Eigen::Block
    <
      typename boost::remove_reference<DataT>::type::ElementVectorT,
      RowVarDataT::dimension * RowVarDataT::EtypeT::nb_nodes,
      1
    > result_type;

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param, typename impl::data_param data) const
    {
      return data.element_vector(boost::proto::value(boost::proto::child_c<0>(expr))).template block
      <
        RowVarDataT::dimension * RowVarDataT::EtypeT::nb_nodes,
        1
        >(data.var_data(RowVarIdxT()).offset * RowVarDataT::EtypeT::nb_nodes, 0);
    }
  };
};

/// Only get a block containing the rows for the first var and the cols for the second
struct ElementMatrixBlockValue :
  boost::proto::transform< ElementMatrixBlockValue >
{

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::child_c<ExprT, 1>::type Child1T;
    typedef typename boost::proto::result_of::child_c<ExprT, 2>::type Child2T;
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(Child1T)>::type>::type RowVarIdxT;
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(Child2T)>::type>::type ColVarIdxT;
    typedef typename VarDataType<RowVarIdxT, DataT>::type RowVarDataT;
    typedef typename VarDataType<ColVarIdxT, DataT>::type ColVarDataT;

    typedef Eigen::Block
    <
      typename boost::remove_reference<DataT>::type::ElementMatrixT,
      RowVarDataT::dimension * RowVarDataT::EtypeT::nb_nodes,
      ColVarDataT::dimension * ColVarDataT::EtypeT::nb_nodes
    > result_type;

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param, typename impl::data_param data) const
    {
      return data.element_matrix(boost::proto::value(boost::proto::child_c<0>(expr))).template block
      <
        RowVarDataT::dimension * RowVarDataT::EtypeT::nb_nodes,
        ColVarDataT::dimension * ColVarDataT::EtypeT::nb_nodes
        >(data.var_data(RowVarIdxT()).offset * RowVarDataT::EtypeT::nb_nodes, data.var_data(ColVarIdxT()).offset * ColVarDataT::EtypeT::nb_nodes);
    }
  };
};

/// Get the RHS vector
struct ElementRHSValue :
  boost::proto::transform<ElementRHSValue>
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<DataT>::type::ElementVectorT& result_type;

    result_type operator ()(typename impl::expr_param, typename impl::state_param, typename impl::data_param data) const
    {
      return data.element_rhs();
    }
  };
};

/// A subblock of rows
template<typename I, typename J>
struct SubRows : boost::proto::transform< SubRows<I, J> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::child_c<ExprT, 1>::type Child1T;
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(Child1T)>::type>::type RowVarIdxT;
    typedef typename VarDataType<RowVarIdxT, DataT>::type RowVarDataT;

    typedef typename boost::remove_reference<StateT>::type MatrixT;

    typedef Eigen::Block
    <
      MatrixT,
      RowVarDataT::EtypeT::nb_nodes,
      MatrixT::ColsAtCompileTime
    > result_type;

    result_type operator ()(typename impl::expr_param expr, typename boost::remove_const<typename impl::state>::type matrix, typename impl::data_param data) const
    {
      return matrix.template block<RowVarDataT::EtypeT::nb_nodes, MatrixT::ColsAtCompileTime>
      (
        RowVarDataT::EtypeT::nb_nodes * IndexValues<I, J>()(boost::proto::right(boost::proto::child_c<1>(expr))),
        0
      );
    }
  };
};

/// A subblock for the element vector
template<typename I, typename J>
struct VectorSubRows : boost::proto::transform< VectorSubRows<I, J> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::child_c<ExprT, 1>::type Child1T;
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(Child1T)>::type>::type RowVarIdxT;
    typedef typename VarDataType<RowVarIdxT, DataT>::type RowVarDataT;

    typedef typename boost::remove_reference<StateT>::type VectorT;

    typedef Eigen::Block
    <
    VectorT,
    RowVarDataT::EtypeT::nb_nodes,
    1
    > result_type;

    result_type operator ()(typename impl::expr_param expr, typename boost::remove_const<typename impl::state>::type matrix, typename impl::data_param data) const
    {
      return matrix.template block<RowVarDataT::EtypeT::nb_nodes, 1>
      (
        RowVarDataT::EtypeT::nb_nodes * IndexValues<I, J>()(boost::proto::right(boost::proto::child_c<1>(expr))),
        0
      );
    }
  };
};

/// A subblock of columns
template<typename I, typename J>
struct SubCols : boost::proto::transform< SubCols<I, J> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::child_c<ExprT, 2>::type Child2T;
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(Child2T)>::type>::type ColVarIdxT;
    typedef typename VarDataType<ColVarIdxT, DataT>::type ColVarDataT;

    typedef typename boost::remove_reference<StateT>::type MatrixT;

    typedef Eigen::Block
    <
      MatrixT,
      MatrixT::RowsAtCompileTime,
      ColVarDataT::EtypeT::nb_nodes
    > result_type;

    result_type operator ()(typename impl::expr_param expr, typename boost::remove_const<typename impl::state>::type matrix, typename impl::data_param data) const
    {
      return matrix.template block<MatrixT::RowsAtCompileTime, ColVarDataT::EtypeT::nb_nodes>
      (
        0,
        ColVarDataT::EtypeT::nb_nodes * IndexValues<I, J>()(boost::proto::right(boost::proto::child_c<2>(expr)))
      );
    }
  };
};

/// A submatrix
template<typename I, typename J>
struct SubMatrix : boost::proto::transform< SubMatrix<I, J> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::child_c<ExprT, 1>::type Child1T;
    typedef typename boost::proto::result_of::child_c<ExprT, 2>::type Child2T;
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(Child1T)>::type>::type RowVarIdxT;
    typedef typename boost::mpl::prior<typename boost::result_of<ExprVarArity(Child2T)>::type>::type ColVarIdxT;
    typedef typename VarDataType<RowVarIdxT, DataT>::type RowVarDataT;
    typedef typename VarDataType<ColVarIdxT, DataT>::type ColVarDataT;

    typedef typename boost::remove_reference<StateT>::type MatrixT;

    typedef Eigen::Block
    <
      MatrixT,
      RowVarDataT::EtypeT::nb_nodes,
      ColVarDataT::EtypeT::nb_nodes
    > result_type;

    result_type operator ()(typename impl::expr_param expr, typename boost::remove_const<typename impl::state>::type matrix, typename impl::data_param data) const
    {
      return matrix.template block<RowVarDataT::EtypeT::nb_nodes, ColVarDataT::EtypeT::nb_nodes>
      (
        RowVarDataT::EtypeT::nb_nodes * IndexValues<I, J>()(boost::proto::right(boost::proto::child_c<1>(expr))),
        ColVarDataT::EtypeT::nb_nodes * IndexValues<I, J>()(boost::proto::right(boost::proto::child_c<2>(expr)))
      );
    }
  };
};

struct ElementMatrixGrammar :
  boost::proto::or_
  <
    boost::proto::when
    <
      ElementMatrixTerm,
      ElementMatrixValue(boost::proto::_expr, boost::proto::_value)
    >,
    boost::proto::when
    <
      ElementVectorTerm,
      ElementVectorValue(boost::proto::_expr, boost::proto::_value)
    >,
    boost::proto::when
    <
      boost::proto::function<ElementMatrixTerm, FieldTypes>,
      ElementMatrixRowsValue(boost::proto::_value(boost::proto::_child1), boost::proto::_value(boost::proto::_child0))
    >,
    boost::proto::when
    <
      boost::proto::subscript<ElementVectorTerm, FieldTypes>,
      ElementVectorRowsValue
    >,
    boost::proto::when
    <
      boost::proto::function<ElementMatrixTerm, FieldTypes, FieldTypes>,
      ElementMatrixBlockValue
    >,
    boost::proto::when
    <
      boost::proto::terminal<ElementRHS>,
      ElementRHSValue
    >
  >
{
};

/// Gets submatrices of vector variables
template<typename I, typename J>
struct ElementMatrixGrammarIndexed :
  boost::proto::or_
  <
    boost::proto::when
    <
      IsSubRows<boost::proto::_>,
      boost::proto::call< SubRows<I, J> >(boost::proto::_expr, ElementMatrixBlockValue)
    >,
    boost::proto::when
    <
      IsVectorSubRows<boost::proto::_>,
      boost::proto::call< VectorSubRows<I, J> >(boost::proto::_expr, ElementVectorRowsValue)
    >,
    boost::proto::when
    <
      IsSubCols<boost::proto::_>,
      boost::proto::call< SubCols<I, J> >(boost::proto::_expr, ElementMatrixBlockValue)
    >,
    boost::proto::when
    <
      IsSubMatrix<boost::proto::_>,
      boost::proto::call< SubMatrix<I, J> >(boost::proto::_expr, ElementMatrixBlockValue)
    >
  >
{
};


} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ElementMatrix_hpp
