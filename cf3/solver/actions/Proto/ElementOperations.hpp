// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ElementOperations_hpp
#define cf3_solver_actions_Proto_ElementOperations_hpp

#include <boost/mpl/assert.hpp>
#include <boost/proto/core.hpp>
#include <boost/proto/traits.hpp>

#include "common/CF.hpp"

#include "Transforms.hpp"

/// @file
/// Operations used in element-wise expressions

struct C;
namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Helper to get the variable type at a child of an expression
template<typename ExprT, int Idx>
struct VarChild
{
  typedef typename boost::remove_const
  <
    typename boost::remove_reference
    <
      typename boost::proto::result_of::value
      <
        typename boost::proto::result_of::child_c<ExprT, Idx>::type
      >::type
    >::type
  >::type type;
};

/// Element volume
struct VolumeOp : boost::proto::transform< VolumeOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef Real result_type;

    result_type operator()(typename impl::expr_param, typename impl::state_param, typename impl::data_param data)
    {
      return data.support().volume();
    }

  };
};

/// Element nodes
struct NodesOp : boost::proto::transform< NodesOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef const typename boost::remove_reference<DataT>::type::SupportT::EtypeT::NodesT& result_type;

    result_type operator()(typename impl::expr_param, typename impl::state_param, typename impl::data_param data)
    {
      return data.support().nodes();
    }

  };
};

/// Base class for the implementation of operations that depend on mapped coordinates (CRTP pattern)
template<typename ExprT, typename StateT, typename DataT, typename Derived, template<typename> class ResultType>
struct MappedOpBase : boost::proto::transform_impl<ExprT, StateT, DataT>
{
  /// Type of the geometric support
  typedef typename boost::remove_reference<DataT>::type::SupportT SupportT;
  /// Type of the mapped coordinates
  typedef typename SupportT::EtypeT::MappedCoordsT MappedCoordsT;

  typedef typename ResultType<SupportT>::type result_type;

  result_type operator()(typename MappedOpBase::expr_param expr, typename MappedOpBase::state_param, typename MappedOpBase::data_param data)
  {
    return dispatch(boost::mpl::int_<boost::proto::arity_of<ExprT>::value>(), expr, data);
  }

  /// Only the variable as function argument, matrices are precomputed, no mapped coords needed
  result_type dispatch(boost::mpl::int_<0>, typename MappedOpBase::expr_param, typename MappedOpBase::data_param data)
  {
    return Derived::apply(data.support());
  }

  /// Mapped coordinates were supplied as an argument
  result_type dispatch(boost::mpl::int_<2>, typename MappedOpBase::expr_param expr, typename MappedOpBase::data_param data)
  {
    return Derived::apply(data.support(), boost::proto::value(boost::proto::child_c<1>(expr)));
  }
};

/// Base class for the implementation of operations that depend on mapped coordinates (CRTP pattern)
template<typename ExprT, typename StateT, typename DataT, typename Derived, template<typename> class ResultType>
struct MappedVarOpBase : boost::proto::transform_impl<ExprT, StateT, DataT>
{
  /// Type of the variable itself
  typedef typename VarChild<ExprT, 1>::type VarT;
  /// Type of the data associated with the variable
  typedef typename VarDataType<VarT, DataT>::type VarDataT;
  /// Type of the geometric support
  typedef typename boost::remove_reference<DataT>::type::SupportT SupportT;
  /// Type of the mapped coordinates
  typedef typename SupportT::EtypeT::MappedCoordsT MappedCoordsT;

  typedef typename ResultType<VarDataT>::type result_type;

  result_type operator()(typename MappedVarOpBase::expr_param expr, typename MappedVarOpBase::state_param, typename MappedVarOpBase::data_param data)
  {
    return dispatch(boost::mpl::int_<boost::proto::arity_of<ExprT>::value>(), expr, data);
  }

  /// Only the variable as function argument, matrices are precomputed, no mapped coords needed
  result_type dispatch(boost::mpl::int_<2>, typename MappedVarOpBase::expr_param, typename MappedVarOpBase::data_param data)
  {
    return Derived::apply(data.support(), data.var_data(typename VarT::index_type()));
  }

  /// Mapped coordinates were put in the last argument
  result_type dispatch(boost::mpl::int_<3>, typename MappedVarOpBase::expr_param expr, typename MappedVarOpBase::data_param data)
  {
    return Derived::apply(data.support(), data.var_data(typename VarT::index_type()), boost::proto::value(boost::proto::child_c<2>(expr)));
  }
};

/// Interpolated real-world coordinates at mapped coordinates
struct CoordinatesOp : boost::proto::transform< CoordinatesOp >
{
  template<typename SupportT>
  struct ResultType
  {
    typedef const typename SupportT::EtypeT::CoordsT& type;
  };

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : MappedOpBase<ExprT, StateT, DataT, impl<ExprT, StateT, DataT>, ResultType>
  {
    static typename impl::result_type apply(const typename impl::SupportT& support)
    {
      return support.coordinates();
    }

    static typename impl::result_type apply(const typename impl::SupportT& support, const typename impl::MappedCoordsT& mapped_coords)
    {
      return support.coordinates(mapped_coords);
    }
  };
};

/// Interpolated values at mapped coordinates
struct InterpolationOp : boost::proto::transform< InterpolationOp >
{
  template<typename VarT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, StateT, DataT>
  {
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename VarDataT::EvalT result_type;
    typedef typename VarDataT::EtypeT::MappedCoordsT MappedCoordsT;

    /// Mapped coords supplied explicitely
    result_type operator()(typename impl::expr_param, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename boost::remove_reference<VarT>::type::index_type()).eval(mapped_coords);
    }

    /// Use precomputed value
    template<typename T>
    result_type operator()(typename impl::expr_param, T, typename impl::data_param data)
    {
      return data.var_data(typename boost::remove_reference<VarT>::type::index_type()).eval();
    }
  };
};

/// Jacobian matrix
struct JacobianOp : boost::proto::transform< JacobianOp >
{
  template<typename SupportT>
  struct ResultType
  {
    typedef const typename SupportT::ShapeFunctionT::JacobianT& type;
  };

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : MappedOpBase<ExprT, StateT, DataT, impl<ExprT, StateT, DataT>, ResultType>
  {
    static typename impl::result_type apply(const typename impl::SupportT& support)
    {
      return support.jacobian();
    }

    static typename impl::result_type apply(const typename impl::SupportT& support, const typename impl::MappedCoordsT& mapped_coords)
    {
      return support.jacobian(mapped_coords);
    }
  };
};

/// Jacobian determinant
struct JacobianDeterminantOp : boost::proto::transform< JacobianDeterminantOp >
{
  template<typename SupportT>
  struct ResultType
  {
    typedef Real type;
  };

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : MappedOpBase<ExprT, StateT, DataT, impl<ExprT, StateT, DataT>, ResultType>
  {
    static typename impl::result_type apply(const typename impl::SupportT& support)
    {
      return support.jacobian_determinant();
    }

    static typename impl::result_type apply(const typename impl::SupportT& support, const typename impl::MappedCoordsT& mapped_coords)
    {
      return support.jacobian_determinant(mapped_coords);
    }
  };
};

/// Face Normal
struct NormalOp : boost::proto::transform< NormalOp >
{
  template<typename SupportT>
  struct ResultType
  {
    typedef const typename SupportT::EtypeT::CoordsT& type;
  };

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : MappedOpBase<ExprT, StateT, DataT, impl<ExprT, StateT, DataT>, ResultType>
  {
    static typename impl::result_type apply(const typename impl::SupportT& support)
    {
      return support.normal();
    }

    static typename impl::result_type apply(const typename impl::SupportT& support, const typename impl::MappedCoordsT& mapped_coords)
    {
      return support.normal(mapped_coords);
    }
  };
};

/// Gradient
struct NablaOp : boost::proto::transform< NablaOp >
{
  template<typename VarDataT>
  struct ResultType
  {
    typedef const typename VarDataT::EtypeT::SF::GradientT& type;
  };

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : MappedVarOpBase<ExprT, StateT, DataT, impl<ExprT, StateT, DataT>, ResultType>
  {
    static typename impl::result_type apply(const typename impl::SupportT& support, const typename impl::VarDataT& data)
    {
      return data.nabla();
    }

    static typename impl::result_type apply(const typename impl::SupportT& support, const typename impl::VarDataT& data, const typename impl::MappedCoordsT& mapped_coords)
    {
      return data.nabla(mapped_coords);
    }
  };
};

/// Shape functions
struct ShapeFunctionOp : boost::proto::transform< ShapeFunctionOp >
{
  template<typename VarDataT>
  struct ResultType
  {
    typedef const typename VarDataT::EtypeT::SF::ValueT& type;
  };

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : MappedVarOpBase<ExprT, StateT, DataT, impl<ExprT, StateT, DataT>, ResultType>
  {
    static typename impl::result_type apply(const typename impl::SupportT& support, const typename impl::VarDataT& data)
    {
      return data.shape_function();
    }

    static typename impl::result_type apply(const typename impl::SupportT& support, const typename impl::VarDataT& data, const typename impl::MappedCoordsT& mapped_coords)
    {
      return data.shape_function(mapped_coords);
    }
  };
};

/// Operation with a custom implementation
template<typename OpImpl, typename GrammarT>
struct CustomSFOpTransform : boost::proto::transform< CustomSFOpTransform<OpImpl, GrammarT> >
{
  /// Catch field terminals first
  struct ChildGrammar :
    boost::proto::or_
    <
      boost::proto::when<FieldTypes, boost::proto::_value>,
      GrammarT
    >
  {
  };

  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    /// Helper to get the variable type at a child of an expression
    template<int Idx>
    struct EvaluatedChild
    {
      typedef typename boost::remove_const
      <
        typename boost::remove_reference
        <
          typename boost::result_of
          <
            ChildGrammar(typename boost::proto::result_of::child_c<ExprT, Idx>::type, StateT, DataT)
          >::type
        >::type
      >::type type;
    };

    /// Get the type for the child at index I
    /// Terminal values are left unchanged, numbered variables are replaced by their context data
    template<Uint I>
    struct ChildType
    {
      /// Keep the type, by default
      template<typename T>
      struct DataType
      {
        typedef T type;
      };

      /// Convert numbered variables to their data
      template<typename VarI, typename T>
      struct DataType< Var<VarI, T> >
      {
        typedef typename VarDataType<VarI, DataT>::type type;
      };

      typedef typename DataType<typename EvaluatedChild<I>::type>::type type;
    };

    /// Helper to get the actual value of a child.
    /// The custom op may want to modify this, so we remove the const as well
    template<typename ChildT>
    struct GetChild
    {
      ChildT& operator()(const ChildT& child, typename impl::data_param data)
      {
        return const_cast<ChildT&>(child);
      }
    };

    /// Specialization to get the associated data
    template<typename I,  typename T>
    struct GetChild< Var<I, T> >
    {
      typedef typename VarDataType<I, DataT>::type VarDataT;

      const VarDataT& operator()(const Var<I, T>& var, typename impl::data_param data)
      {
        return data.var_data(var);
      }
    };

    /// Helper to get the result type
    template<typename TagT, Uint Arity, Uint Dummy=0>
    struct ResultType;

    /// Specialization for a function with one argument
    template<Uint Dummy>
    struct ResultType<boost::proto::tag::function, 2, Dummy>
    {
      typedef typename boost::result_of<OpImpl(typename ChildType<1>::type)>::type type;
    };

    /// Specialization for a function with 2 arguments
    template<Uint Dummy>
    struct ResultType<boost::proto::tag::function, 3, Dummy>
    {
      typedef typename boost::result_of<OpImpl(typename ChildType<1>::type, typename ChildType<2>::type)>::type type;
    };

    /// Specialization for a function with 4 arguments
    template<Uint Dummy>
    struct ResultType<boost::proto::tag::function, 5, Dummy>
    {
      typedef typename boost::result_of<OpImpl(typename ChildType<1>::type, typename ChildType<2>::type, typename ChildType<3>::type, typename ChildType<4>::type)>::type type;
    };
    
    /// Specialization for a function with 5 arguments
    template<Uint Dummy>
    struct ResultType<boost::proto::tag::function, 6, Dummy>
    {
      typedef typename boost::result_of<OpImpl(typename ChildType<1>::type, typename ChildType<2>::type, typename ChildType<3>::type, typename ChildType<4>::type, typename ChildType<5>::type)>::type type;
    };

    typedef typename ResultType<typename boost::proto::tag_of<ExprT>::type, boost::proto::arity_of<ExprT>::value>::type result_type;

    // Non-const version
    typedef typename boost::remove_const<typename impl::expr>::type& ExprParamT;

    result_type operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data)
    {
      return apply<result_type>()(typename boost::proto::tag_of<ExprT>::type(), boost::mpl::int_<boost::proto::arity_of<ExprT>::value>(), expr, state, data);
    }

    /// Pass the stored parameter if the result type is a reference, don't store otherwise
    template<typename ResultT>
    struct apply
    {
      result_type operator()(boost::proto::tag::function,
                             boost::mpl::int_<2>,
                             typename impl::expr_param expr,
                             typename impl::state_param state,
                             typename impl::data_param data) const
      {
        return OpImpl()(GetChild<typename EvaluatedChild<1>::type>()(ChildGrammar()(boost::proto::child_c<1>(expr), state, data), data));
      }

      result_type operator()(boost::proto::tag::function,
                        boost::mpl::int_<3>,
                        typename impl::expr_param expr,
                        typename impl::state_param state,
                        typename impl::data_param data) const
      {
        return OpImpl()
        (
          GetChild<typename EvaluatedChild<1>::type>()(ChildGrammar()(boost::proto::child_c<1>(expr), state, data), data),
          GetChild<typename EvaluatedChild<2>::type>()(ChildGrammar()(boost::proto::child_c<2>(expr), state, data), data)
        );
      }

      result_type operator()(boost::proto::tag::function,
                             boost::mpl::int_<5>,
                             typename impl::expr_param expr,
                             typename impl::state_param state,
                             typename impl::data_param data) const
      {
        return OpImpl()
        (
          GetChild<typename EvaluatedChild<1>::type>()(ChildGrammar()(boost::proto::child_c<1>(expr), state, data), data),
          GetChild<typename EvaluatedChild<2>::type>()(ChildGrammar()(boost::proto::child_c<2>(expr), state, data), data),
          GetChild<typename EvaluatedChild<3>::type>()(ChildGrammar()(boost::proto::child_c<3>(expr), state, data), data),
          GetChild<typename EvaluatedChild<4>::type>()(ChildGrammar()(boost::proto::child_c<4>(expr), state, data), data)
        );
      }
      
      result_type operator()(boost::proto::tag::function,
                             boost::mpl::int_<6>,
                             typename impl::expr_param expr,
                             typename impl::state_param state,
                             typename impl::data_param data) const
      {
        return OpImpl()
        (
          GetChild<typename EvaluatedChild<1>::type>()(ChildGrammar()(boost::proto::child_c<1>(expr), state, data), data),
          GetChild<typename EvaluatedChild<2>::type>()(ChildGrammar()(boost::proto::child_c<2>(expr), state, data), data),
          GetChild<typename EvaluatedChild<3>::type>()(ChildGrammar()(boost::proto::child_c<3>(expr), state, data), data),
          GetChild<typename EvaluatedChild<4>::type>()(ChildGrammar()(boost::proto::child_c<4>(expr), state, data), data),
          GetChild<typename EvaluatedChild<5>::type>()(ChildGrammar()(boost::proto::child_c<5>(expr), state, data), data)
        );
      }
    };

    /// Specialization for references
    template<typename ResultT>
    struct apply<ResultT&>
    {
      result_type operator()(boost::proto::tag::function,
                             boost::mpl::int_<2>,
                             typename impl::expr_param expr,
                             typename impl::state_param state,
                             typename impl::data_param data) const
      {
        return OpImpl()(expr.value, GetChild<typename EvaluatedChild<1>::type>()(boost::proto::value(boost::proto::child_c<1>(expr)), data));
      }
    };

  };
};

/// Wrap all operations in a template, so we can detect ops using a wildcard
template<typename OpT>
struct SFOp
{
  typedef OpT result_type;
};

template<typename CustomT>
struct CustomSFOp
{
};

template<typename OpT>
struct SFOp< CustomSFOp<OpT> >
{
  template<typename Signature>
  struct result;

  template<typename This, typename GrammarT>
  struct result<This(GrammarT)>
  {
    typedef CustomSFOpTransform<OpT, GrammarT> type;
  };
};

/// Helper struct to declare custom types
template<typename OpT>
struct MakeSFOp
{
  typedef typename boost::proto::terminal< SFOp< CustomSFOp< OpT > > >::type const type;
};

/// Static terminals that can be used in proto expressions
boost::proto::terminal< SFOp<VolumeOp> >::type const volume = {};
boost::proto::terminal< SFOp<NodesOp> >::type const nodes = {};

boost::proto::terminal< SFOp<CoordinatesOp> >::type const coordinates = {};
boost::proto::terminal< SFOp<JacobianOp> >::type const jacobian = {};
boost::proto::terminal< SFOp<JacobianDeterminantOp> >::type const jacobian_determinant = {};
boost::proto::terminal< SFOp<NormalOp> >::type const normal = {};
boost::proto::terminal< SFOp<NablaOp> >::type const nabla = {};

boost::proto::terminal< SFOp<ShapeFunctionOp> >::type const N = {};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ElementOperations_hpp
