// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_Transforms_hpp
#define CF_Solver_Actions_Proto_Transforms_hpp

#include <boost/fusion/container/vector/convert.hpp>
#include <boost/mpl/max.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/proto/proto.hpp>

#include "Terminals.hpp"

/// @file
/// Some generally useful transforms, not bound to mesh or node related expressions

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

template<typename T>
struct VarArity
{
  typedef typename T::index_type type;
};

/// Gets the arity (max index) for the numbered variables
struct ExprVarArity
  : boost::proto::or_<
        boost::proto::when< boost::proto::terminal< Var<boost::proto::_, boost::proto::_> >,
          boost::mpl::next< VarArity<boost::proto::_value> >()
        >
      , boost::proto::when< boost::proto::terminal<boost::proto::_>,
          boost::mpl::int_<0>()
        >
      , boost::proto::when<
            boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >
          , boost::proto::fold<boost::proto::_, boost::mpl::int_<0>(), boost::mpl::max<ExprVarArity, boost::proto::_state>()>
        >
    >
{};

/// Transform that extracts the type of variable I
template<Uint I>
struct DefineType
  : boost::proto::or_<
      boost::proto::when< boost::proto::terminal< Var<boost::mpl::int_<I>, boost::proto::_> >,
          boost::proto::_value
      >
    , boost::proto::when< boost::proto::terminal< boost::proto::_ >,
          boost::mpl::void_()
      >
    , boost::proto::when< boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >
            , boost::proto::fold<boost::proto::_, boost::mpl::void_(), boost::mpl::if_<boost::mpl::is_void_<boost::proto::_state>, DefineType<I>, boost::proto::_state>() >
      >
  >
{};

/// Ease application of DefineType as an MPL lambda expression, and strip the Var encapsulation
template<typename I, typename Expr>
struct DefineTypeOp
{
  typedef typename boost::result_of<DefineType<I::value>(Expr)>::type var_type;
  typedef typename boost::mpl::if_<boost::mpl::is_void_<var_type>, boost::mpl::void_, typename var_type::type>::type type;
};

/// Convenience struct to get easy access to the numberof variables and their type
template<typename ExprT>
struct ExpressionProperties
{
  /// Number of variables in the expression (boost mpl integral constant)
  typedef typename boost::result_of<ExprVarArity(ExprT)>::type NbVarsT;
  
  /// Types of the used variables
  typedef typename boost::fusion::result_of::as_vector
  <
    typename boost::mpl::transform
    <
      typename boost::mpl::copy<boost::mpl::range_c<int,0,NbVarsT::value>, boost::mpl::back_inserter< boost::mpl::vector_c<Uint> > >::type, //range from 0 to NbVarsT
      DefineTypeOp<boost::mpl::_1, ExprT>
    >::type
  >::type VariablesT;
};

/// Copy the terminal values to a fusion list
template<typename VarsT>
struct CopyNumberedVars
  : boost::proto::callable_context< CopyNumberedVars<VarsT>, boost::proto::null_context >
{
  typedef void result_type;
  
  CopyNumberedVars(VarsT& vars) : m_vars(vars) {}

  template<typename I, typename T>
  void operator()(boost::proto::tag::terminal, const Var<I, T>& val)
  {
    boost::fusion::at<I>(m_vars) = static_cast<T const&>(val);
  }
  
private:
  VarsT& m_vars;  
};

/// Function object to add options, based on the type of variable
struct AddVariableOptions
{
  AddVariableOptions(Common::Component::Ptr an_owner) :
    owner(an_owner)
  {
  }
  
  template<typename T>
  inline void operator()(T& t) const;

  Common::Component::Ptr owner;
};

template<typename T>
inline void AddVariableOptions::operator()(T& var) const
{
  // Add options in case we derive from OptionVariableBase
  if(boost::is_base_of<OptionVariable, T>::value)
  {
    var.set_owner(owner);
  }
}

template<>
inline void AddVariableOptions::operator()<boost::mpl::void_>(boost::mpl::void_&) const
{
}

/// Returns the index of a numbered variable as a boost integral constant
struct VarNumber :
  boost::proto::transform< VarNumber >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  { 
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value<ExprT>::type
      >::type
    >::type VarT;
    
    typedef typename VarT::index_type result_type;
  
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      return result_type();
    }
  };
};

/// Returns the data associated with the given numbered variable
struct NumberedData :
  boost::proto::transform< NumberedData >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  { 
    typedef typename boost::result_of<VarNumber(ExprT)>::type I;
    
    typedef typename boost::remove_reference<DataT>::type::template DataType<I>::type VarDataT;
    typedef VarDataT& result_type;
  
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      return data.template var_data<I>();
    }
  };
};

/// Returns the data value of a numbered variable
struct VarValue :
  boost::proto::transform< VarValue >
{
  template<typename VarT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, StateT, DataT>
  { 
    
    typedef typename boost::remove_reference<DataT>::type::ValueResultT result_type;
  
    result_type operator ()(
                typename impl::expr_param var
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      return data.value();
    }
  };
};

/// Matches scalar terminals
struct Scalar :
  boost::proto::when
  <
    boost::proto::or_< boost::proto::terminal<Real>, boost::proto::terminal<Uint> >, // Plain scalar
    boost::proto::_value
  >
{
};

/// Matches matrices and vectors
struct MatVec :
  boost::proto::or_
  <
    boost::proto::terminal< RealVector >,
    boost::proto::terminal< RealVector2 >,
    boost::proto::terminal< RealVector3 >,
    boost::proto::terminal< RealVector4 >,
    boost::proto::terminal< RealMatrix >,
    boost::proto::terminal< RealMatrix2 >,
    boost::proto::terminal< RealMatrix3 >,
    boost::proto::terminal< RealMatrix4 >
  >
{
};

/// Matches terminal values that can be used in math formulas
struct MathTerminals :
  boost::proto::or_
  <
    Scalar,
    boost::proto::when
    <
      MatVec,
      boost::proto::_value
    >,
    // Value of numbered variables
    boost::proto::when
    <
      boost::proto::terminal< Var< boost::proto::_, boost::proto::_ > >,
      VarValue(boost::proto::_expr, boost::proto::_state, NumberedData)
    >
  >
{
};

/// Split up MapthOpDefault using cases, for better compilation performance
template<typename GrammarT>
struct MathOpDefaultCases
{
  template<typename Tag, int Dummy = 0> struct case_ : boost::proto::not_<boost::proto::_> {};
  
  template<int Dummy> struct case_<boost::proto::tag::unary_plus, Dummy> : boost::proto::unary_plus<GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::negate, Dummy> : boost::proto::negate<GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::pre_inc, Dummy> : boost::proto::pre_inc<GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::post_inc, Dummy> : boost::proto::post_inc<GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::pre_dec, Dummy> : boost::proto::pre_dec<GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::post_dec, Dummy> : boost::proto::post_dec<GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::plus, Dummy> : boost::proto::plus<GrammarT, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::minus, Dummy> : boost::proto::minus<GrammarT, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::multiplies, Dummy> : boost::proto::multiplies<GrammarT, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::divides, Dummy> : boost::proto::divides<GrammarT, Scalar> {};
  template<int Dummy> struct case_<boost::proto::tag::assign, Dummy> : boost::proto::assign<GrammarT, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::plus_assign, Dummy> : boost::proto::plus_assign<GrammarT, GrammarT> {};
  template<int Dummy> struct case_<boost::proto::tag::function, Dummy> : boost::proto::function< boost::proto::_, boost::proto::vararg<GrammarT> > {};
};


/// Math operators evaluated using default C++ meaning
template<typename GrammarT>
struct MathOpDefault :
  boost::proto::when
  <
    boost::proto::switch_< MathOpDefaultCases<GrammarT> >,
    boost::proto::_default<GrammarT>
  >
{
};

/// Stream output
template<typename GrammarT>
struct StreamOutput :
  boost::proto::when
  <
    boost::proto::or_
    <
      boost::proto::shift_left< boost::proto::terminal< std::ostream & >, boost::proto::_ >,
      boost::proto::shift_left< StreamOutput<GrammarT>, boost::proto::_ >
    >,
    boost::proto::_default<GrammarT>
  >
{
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_Transforms_hpp
