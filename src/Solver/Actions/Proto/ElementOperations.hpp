// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementOperations_hpp
#define CF_Solver_Actions_Proto_ElementOperations_hpp

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

/// DivergenceLin
struct DivergenceLinOp : boost::proto::transform< DivergenceLinOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef const typename VarDataType<VarT, DataT>::type::DivergenceLinT& result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).divergence_lin(mapped_coords, data.support());
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<2>(expr)), data);
    }
  };
};

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct LaplacianImpl;

/// Laplacian
struct LaplacianElmOp : boost::proto::transform< LaplacianElmOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename LaplacianImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;

    /// Mapped coords in state
    result_type operator()(typename impl::expr_param expr, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).laplacian_elm(mapped_coords, data.support());
    }
    
    /// Mapped coords as arg
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

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ValueImpl;

/// Outer product
struct ValueElmOp : boost::proto::transform< ValueElmOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename ValueImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).value_elm(mapped_coords);
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<2>(expr)), data);
    }
  };
};

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct GradientImpl;

/// Element gradient
struct GradientElmOp : boost::proto::transform< GradientElmOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename GradientImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).gradient_elm(mapped_coords, data.support());
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<2>(expr)), data);
    }
  };
};

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct DivergenceImpl;

/// Element gradient
struct DivergenceElmOp : boost::proto::transform< DivergenceElmOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename DivergenceImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).divergence_elm(mapped_coords, data.support());
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<2>(expr)), data);
    }
  };
};

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct AdvectionImpl;

/// Element advection
struct AdvectionElmOp : boost::proto::transform< AdvectionElmOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename VarChild<ExprT, 1>::type VarT;
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename AdvectionImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    typedef typename boost::remove_reference<DataT>::type::SupportT::SF::MappedCoordsT MappedCoordsT;
    
    result_type operator()(typename impl::expr_param var, const MappedCoordsT& mapped_coords, typename impl::data_param data)
    {
      return data.var_data(typename VarT::index_type()).advection_elm(mapped_coords, data.support());
    }
    
    result_type operator()(typename impl::expr_param expr, const Uint&, typename impl::data_param data)
    {
      return operator()(expr, boost::proto::value(boost::proto::child_c<2>(expr)), data);
    }
  };
};

/// Wrap all operations in a teplate, so we can detect ops using a wildcard
template<typename OpT>
struct SFOp
{
  typedef OpT type;
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
boost::proto::terminal< SFOp<DivergenceLinOp> >::type const divergence_lin = {};

boost::proto::terminal< SFOp<ShapeFunctionOp> >::type const shape_function = {};

boost::proto::terminal< SFOp<GradientElmOp> >::type const gradient_elm = {};
boost::proto::terminal< SFOp<DivergenceElmOp> >::type const divergence_elm = {};
boost::proto::terminal< SFOp<LaplacianElmOp> >::type const laplacian_elm = {};
boost::proto::terminal< SFOp<ValueElmOp> >::type const value_elm = {};
boost::proto::terminal< SFOp<AdvectionElmOp> >::type const advection_elm = {};

/// Placeholder for a linearize op
struct LinearizeOp
{
};

boost::proto::terminal<LinearizeOp>::type const linearize = {};

/// SF operations needing only a support
struct SFSupportOp :
  boost::proto::or_
  <
    boost::proto::terminal< VolumeOp >,
    boost::proto::terminal< NodesOp >
  >
{
};

/// SF operations needing a support and mapped coordinates
struct SFSupportMappedOp :
  boost::proto::or_
  <
    boost::proto::terminal<CoordinatesOp>,
    boost::proto::terminal<JacobianOp>,
    boost::proto::terminal<JacobianDeterminantOp>,
    boost::proto::terminal<NormalOp>
  >
{
};

/// SF operations needing a support, a field and mapped coordinates
struct SFSupportFieldMappedOp :
  boost::proto::or_
  <
    boost::proto::or_
    <
      boost::proto::terminal<ShapeFunctionOp>,
      boost::proto::terminal<ValueElmOp>,
      boost::proto::terminal<GradientOp>,
      boost::proto::terminal<AdvectionOp>,
      boost::proto::terminal<DivergenceLinOp>
    >,
    boost::proto::terminal<GradientElmOp>,
    boost::proto::terminal<DivergenceElmOp>,
    boost::proto::terminal<AdvectionElmOp>,
    boost::proto::terminal<LaplacianElmOp>
  >
{
};

/////////////////////////////////////
// Implementations
// The actual implementations of the element matrix operations. These can be specialized.
////////////////////////////////////

/// Called by LaplacianElmOp
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct LaplacianImpl
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  LaplacianImpl()
  {
    m_matrix.setZero();
  }
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, Dim*SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  result_type operator()(const typename SF::MappedGradientT& gradient_matrix) const
  {
    Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> m = gradient_matrix.transpose() * gradient_matrix;
    for(Uint d = 0; d != Dim; ++d)
    {
      m_matrix.template block<SF::nb_nodes, SF::nb_nodes>(SF::nb_nodes*d, Offset+SF::nb_nodes*d).noalias() = m;
    }
    return m_matrix;
  }
  
private:
  mutable MatrixT m_matrix;
};

/// Specialization for scalars
template<typename SF, Uint Offset, Uint MatrixSize>
struct LaplacianImpl<SF, 1, Offset, MatrixSize>
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  LaplacianImpl()
  {
    m_matrix.setZero();
  }
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  result_type operator()(const typename SF::MappedGradientT& gradient_matrix) const
  {
    m_matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, Offset).noalias() = gradient_matrix.transpose() * gradient_matrix;
    return m_matrix;
  }
  
private:
  mutable MatrixT m_matrix;
};

/// Called by ValueElmOp
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ValueImpl
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  ValueImpl()
  {
    m_matrix.setZero();
  }
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, Dim*SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  result_type operator()(const typename SF::ShapeFunctionsT& sf) const
  {
    Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> m = sf.transpose() * sf;
    for(Uint d = 0; d != Dim; ++d)
    {
      m_matrix.template block<SF::nb_nodes, SF::nb_nodes>(SF::nb_nodes*d, Offset+SF::nb_nodes*d).noalias() = m;
    }
    return m_matrix;
  }
  
private:
  mutable MatrixT m_matrix;
};

/// Specialization for scalars
template<typename SF, Uint Offset, Uint MatrixSize>
struct ValueImpl<SF, 1, Offset, MatrixSize>
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  ValueImpl()
  {
    m_matrix.setZero();
  }
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  result_type operator()(const typename SF::ShapeFunctionsT& sf) const
  {
    m_matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, Offset).noalias() = sf.transpose() * sf;
    return m_matrix;
  }
  
private:
  mutable MatrixT m_matrix;
};

/// Interpolation of a field
template<typename SF, Uint Dim>
struct InterpolationImpl
{ 
  typedef Eigen::Matrix<Real, 1, Dim> MatrixT;
  typedef const MatrixT& result_type;
  
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  template<typename NodeValuesT>
  result_type operator()(const typename SF::ShapeFunctionsT& sf, const NodeValuesT& values) const
  {
    m_result.noalias() = sf * values;
    return m_result;
  }
  
private:
  mutable MatrixT m_result;
};

/// Interpolation of a scalar field
template<typename SF>
struct InterpolationImpl<SF, 1>
{
  typedef Real result_type;
  
  template<typename NodeValuesT>
  result_type operator()(const typename SF::ShapeFunctionsT& sf, const NodeValuesT& values) const
  {
    return sf * values;
  }
};

/// Gradient
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct GradientImpl
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  GradientImpl()
  {
    m_matrix.setZero();
  }
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes * SF::dimension, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  result_type operator()(const typename SF::ShapeFunctionsT& sf, const typename SF::MappedGradientT& gradient_matrix) const
  {
    for(Uint j = 0; j != Dim; ++j)
      for(Uint i = 0; i != SF::dimension; ++i)
        m_matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, Offset+j*SF::nb_nodes).noalias() = sf.transpose() * gradient_matrix.row(i);
    
    return m_matrix;
  }
  
private:
  mutable MatrixT m_matrix;
};

/// Specialization for scalars
template<typename SF, Uint Offset, Uint MatrixSize>
struct GradientImpl<SF, 1, Offset, MatrixSize>
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  GradientImpl()
  {
    m_matrix.setZero();
  }
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes * SF::dimension, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  result_type operator()(const typename SF::ShapeFunctionsT& sf, const typename SF::MappedGradientT& gradient_matrix) const
  {
    for(Uint i = 0; i != SF::dimension; ++i)
      m_matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, Offset).noalias() = sf.transpose() * gradient_matrix.row(i);
    return m_matrix;
  }
  
private:
  mutable MatrixT m_matrix;
};

/// Divergence
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct DivergenceImpl
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  DivergenceImpl()
  {
    m_matrix.setZero();
  }
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  result_type operator()(const typename SF::ShapeFunctionsT& sf, const typename SF::MappedGradientT& gradient_matrix) const
  {
    for(Uint i = 0; i != SF::dimension; ++i)
      m_matrix.template block<SF::nb_nodes, SF::nb_nodes>(0, i*SF::nb_nodes).noalias() = sf.transpose() * gradient_matrix.row(i);
    return m_matrix;
  }
  
private:
  mutable MatrixT m_matrix;
};

/// Advection
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct AdvectionImpl
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  AdvectionImpl()
  {
    m_matrix.setZero();
  }
  
  /// Type of the element matrix
  typedef Eigen::Matrix<Real, Dim*SF::nb_nodes, MatrixSize> MatrixT;
  typedef const MatrixT& result_type;
  
  result_type operator()(const typename SF::NodeMatrixT& advection_v, const typename SF::ShapeFunctionsT& sf, const typename SF::MappedGradientT& gradient_matrix) const
  {
    // The advection operator, pre-multiplied with the weight functions (sf)
    const Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> advection_op = sf.transpose() * (sf * advection_v * gradient_matrix);
    for(Uint i = 0; i != SF::dimension; ++i)
      m_matrix.template block<SF::nb_nodes, SF::nb_nodes>(i*SF::nb_nodes, i*SF::nb_nodes).noalias() = advection_op;
    return m_matrix;
  }
  
private:
  mutable MatrixT m_matrix;
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementOperations_hpp
