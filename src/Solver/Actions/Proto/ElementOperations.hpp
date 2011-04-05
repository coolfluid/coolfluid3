// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementOperations_hpp
#define CF_Solver_Actions_Proto_ElementOperations_hpp

#include <boost/mpl/assert.hpp>
#include <boost/proto/core.hpp>

#include "Common/CF.hpp"

#include "Transforms.hpp"

/// @file
/// Operations used in element-wise expressions

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Helper to get the variable type at a child of an expression
template<typename ExprT, int Idx>
struct VarChild
{
  typedef typename boost::remove_reference
  <
    typename boost::proto::result_of::value
    <
      typename boost::proto::result_of::child_c<ExprT, Idx>::type
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
    typedef const typename boost::remove_reference<DataT>::type::SupportT::SF::NodeMatrixT& result_type;
    
    result_type operator()(typename impl::expr_param, typename impl::state_param, typename impl::data_param data)
    {
      return data.support().nodes();
    }
    
  };
};

/// Interpolated real-world coordinates at mapped coordinates
struct CoordinatesOp : boost::proto::transform< CoordinatesOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef const typename boost::remove_reference<DataT>::type::SupportT::SF::CoordsT& result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.support().coordinates(mapped_coords);
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<1>(expr)), data);
    }
  };
};

/// Interpolated values at mapped coordinates
struct InterpolationOp : boost::proto::transform< InterpolationOp >
{
  template<typename VarT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, StateT, DataT>
  {
    
    typedef typename VarDataType<VarT, DataT>::type::EvalT result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(var).eval(mapped_coords);
    }
  };
};

/// Jacobian matrix
struct JacobianOp : boost::proto::transform< JacobianOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef const typename boost::remove_reference<DataT>::type::SupportT::ShapeFunctionT::JacobianT& result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.support().jacobian(mapped_coords);
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<1>(expr)), data);
    }
  };
};

/// Jacobian determinant
struct JacobianDeterminantOp : boost::proto::transform< JacobianDeterminantOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef Real result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.support().jacobian_determinant(mapped_coords);
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<1>(expr)), data);
    }
  };
};

/// Face Normal
struct NormalOp : boost::proto::transform< NormalOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef const typename boost::remove_reference<DataT>::type::SupportT::SF::CoordsT& result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.support().normal(mapped_coords);
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<1>(expr)), data);
    }
  };
};

/// Gradient
struct GradientOp : boost::proto::transform< GradientOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef const typename VarDataType<VarT, DataT>::type::SF::MappedGradientT& result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).gradient(mapped_coords, data.support());
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<2>(expr)), data);
    }
  };
};

/// Advection
struct AdvectionOp : boost::proto::transform< AdvectionOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef const typename VarDataType<VarT, DataT>::type::SF::ShapeFunctionsT& result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).advection(mapped_coords, data.support());
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<2>(expr)), data);
    }
  };
};

/// Shape functions
struct ShapeFunctionOp : boost::proto::transform< ShapeFunctionOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef const typename VarDataType<VarT, DataT>::type::SF::ShapeFunctionsT& result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).shape_function(mapped_coords);
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<2>(expr)), data);
    }
  };
};

template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct LaplacianElmOp
{ 
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, Dim*SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& data, const StateT& state) const
  {
    matrix.setZero();
    const typename SF::MappedGradientT& grad = data.gradient(state, support);
    const Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> m = grad.transpose() * grad;
    for(Uint d = 0; d != Dim; ++d)
    {
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(SF::nb_nodes*d, Offset+SF::nb_nodes*d).noalias() = m;
    }
    return matrix;
  }
};

/// Specialization for scalars
template<typename SF, Uint Offset, Uint MatrixSize>
struct LaplacianElmOp<SF, 1, Offset, MatrixSize>
{ 
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& data, const StateT& state) const
  {
    matrix.setZero();
    const typename SF::MappedGradientT& grad = data.gradient(state, support);
    matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, Offset).noalias() = grad.transpose() * grad;
    return matrix;
  }
};

/// Element matrix when the variable value is integrated, i.e. the outer product of the SF with itself
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ValueElmOp
{
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, Dim*SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& data, const StateT& state) const
  {
    matrix.setZero();
    const typename SF::ShapeFunctionsT& sf = data.shape_function(state);
    Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> m = sf.transpose() * sf;
    for(Uint d = 0; d != Dim; ++d)
    {
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(SF::nb_nodes*d, Offset+SF::nb_nodes*d).noalias() = m;
    }
    return matrix;
  }
};

/// Specialization for scalars
template<typename SF, Uint Offset, Uint MatrixSize>
struct ValueElmOp<SF, 1, Offset, MatrixSize>
{
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& data, const StateT& state) const
  {
    matrix.setZero();
    const typename SF::ShapeFunctionsT& sf = data.shape_function(state);
    matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, Offset).noalias() = sf.transpose() * sf;
    return matrix;
  }
};

/// Gradient (scalars only)
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct GradientElmOp;

/// Specialization for scalars
template<typename SF, Uint Offset, Uint MatrixSize>
struct GradientElmOp<SF, 1, Offset, MatrixSize>
{
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes * SF::dimension, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& data, const StateT& state) const
  {
    matrix.setZero();
    const typename SF::ShapeFunctionsT& sf = data.shape_function(state);
    const typename SF::MappedGradientT& gradient_matrix = data.gradient(state, support);
    for(Uint i = 0; i != SF::dimension; ++i)
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, Offset).noalias() = sf.transpose() * gradient_matrix.row(i);
    return matrix;
  }
};

/// Divergence
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct DivergenceElmOp
{
  // Divergence only makes sense for vectors with the dimension of the problem
  BOOST_MPL_ASSERT_RELATION( SF::dimension, ==, Dim );
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& data, const StateT& state) const
  {
    matrix.setZero();
    const typename SF::ShapeFunctionsT& sf = data.shape_function(state);
    const typename SF::MappedGradientT& gradient_matrix = data.gradient(state, support);
    for(Uint i = 0; i != SF::dimension; ++i)
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, i*SF::nb_nodes).noalias() = sf.transpose() * gradient_matrix.row(i);
    return matrix;
  }
};

/// Advection
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct AdvectionElmOp
{
  BOOST_MPL_ASSERT_RELATION( SF::dimension, ==, Dim );
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, Dim*SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  template<typename SupportT, typename VarDataT, typename StateT>
  result_type operator()(MatrixT& matrix, const SupportT& support, const VarDataT& data, const StateT& state) const
  {
    matrix.setZero();
    const typename SF::ShapeFunctionsT& sf = data.shape_function(state);
    const typename SF::MappedGradientT& gradient_matrix = data.gradient(state, support);
    // The advection operator, pre-multiplied with the weight functions (sf)
    const Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> advection_op = sf.transpose() * (sf * data.value() * gradient_matrix);
    for(Uint i = 0; i != SF::dimension; ++i)
      matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, i*SF::nb_nodes).noalias() = advection_op;
    return matrix;
  }
};

/// Operation with a custom implementation
template<template<typename, Uint, Uint, Uint> class OpImpl>
struct CustomSFOpTransform : boost::proto::transform< CustomSFOpTransform<OpImpl> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    /// Type of the variable we apply to
    typedef typename VarChild<ExprT, 1>::type VarT;
    
    /// Type of the variable data
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    
    /// Type of the operator
    typedef OpImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size> OpT;
    
    /// Result type
    typedef typename OpT::result_type result_type;
    
    /// State without ref and const
    typedef typename boost::remove_const<typename boost::remove_reference<StateT>::type>::type StateValT;

    result_type operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data)
    {
      return OpT()(expr.value, data.support(), data.var_data(typename VarT::index_type()), const_cast<StateValT&>(state));
    }
  };
};

/// Wrap all operations in a template, so we can detect ops using a wildcard
template<typename OpT>
struct SFOp
{
  typedef OpT type;
};

template<typename CustomT>
struct CustomSFOp
{
};

template<typename OpT>
struct SFOp< CustomSFOp<OpT> >
{
  typedef OpT type;
};

/// Helper struct to declare custom types
template<template<typename, Uint, Uint, Uint> class OpT>
struct MakeSFOp
{
  typedef typename boost::proto::terminal< SFOp< CustomSFOp< CustomSFOpTransform<OpT> > > >::type const type;
};

/// Static terminals that can be used in proto expressions
boost::proto::terminal< SFOp<VolumeOp> >::type const volume = {};
boost::proto::terminal< SFOp<NodesOp> >::type const nodes = {};

boost::proto::terminal< SFOp<CoordinatesOp> >::type const coordinates = {};
boost::proto::terminal< SFOp<JacobianOp> >::type const jacobian = {};
boost::proto::terminal< SFOp<JacobianDeterminantOp> >::type const jacobian_determinant = {};
boost::proto::terminal< SFOp<NormalOp> >::type const normal = {};
boost::proto::terminal< SFOp<GradientOp> >::type const gradient = {};
boost::proto::terminal< SFOp<AdvectionOp> >::type const advection = {};

boost::proto::terminal< SFOp<ShapeFunctionOp> >::type const shape_function = {};

MakeSFOp<GradientElmOp>::type gradient_elm = {};
MakeSFOp<DivergenceElmOp>::type divergence_elm = {};
MakeSFOp<LaplacianElmOp>::type laplacian_elm = {};
MakeSFOp<ValueElmOp>::type value_elm = {};
MakeSFOp<AdvectionElmOp>::type advection_elm = {};

/// Placeholder for a linearize op
struct LinearizeOp
{
};

boost::proto::terminal<LinearizeOp>::type const linearize = {};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementOperations_hpp
