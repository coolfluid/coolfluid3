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

/// Element volume
struct VolumeOp : boost::proto::transform< VolumeOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef Real result_type;
    
    result_type operator()(typename impl::expr_param, typename impl::state_param, typename impl::data_param data)
    {
      return data.support().volume();
    }
    
  };
};

/// Interpolated real-world coordinates at mapped coordinates
struct CoordinatesOp : boost::proto::transform< CoordinatesOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef const typename boost::remove_reference<DataT>::type::SupportT::SF::CoordsT& result_type;
    
    result_type operator()(typename impl::expr_param, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.support().coordinates(mapped_coords);
    }
  };
};

/// Interpolated values at mapped coordinates
struct InterpolationOp : boost::proto::transform< InterpolationOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    
    typedef typename VarDataType<VarT, DataT>::type::EvalT result_type;
    
    result_type operator()(typename impl::expr_param var, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.var_data(var).eval(mapped_coords);
    }
  };
};

/// Jacobian matrix
struct JacobianOp : boost::proto::transform< JacobianOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef const typename boost::remove_reference<DataT>::type::SupportT::ShapeFunctionT::JacobianT& result_type;
    
    result_type operator()(typename impl::expr_param, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.support().jacobian(mapped_coords);
    }
  };
};

/// Jacobian determinant
struct JacobianDeterminantOp : boost::proto::transform< JacobianDeterminantOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef Real result_type;
    
    result_type operator()(typename impl::expr_param, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.support().jacobian_determinant(mapped_coords);
    }
  };
};

/// Face Normal
struct NormalOp : boost::proto::transform< NormalOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef const typename boost::remove_reference<DataT>::type::SupportT::SF::CoordsT& result_type;
    
    result_type operator()(typename impl::expr_param, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.support().normal(mapped_coords);
    }
  };
};

/// Gradient
struct GradientOp : boost::proto::transform< GradientOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef const typename VarDataType<VarT, DataT>::type::SF::MappedGradientT& result_type;
    
    result_type operator()(typename impl::expr_param var, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.var_data(var).gradient(mapped_coords, data.support());
    }
  };
};

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct LaplacianImpl;

/// Laplacian
struct LaplacianElmOp : boost::proto::transform< LaplacianElmOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename LaplacianImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    
    result_type operator()(typename impl::expr_param var, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.var_data(var).laplacian_elm(mapped_coords, data.support());
    }
  };
};

/// Shape functions
struct ShapeFunctionOp : boost::proto::transform< ShapeFunctionOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef const typename VarDataType<VarT, DataT>::type::ShapeFunctionT::ShapeFunctionsT& result_type;
    
    result_type operator()(typename impl::expr_param var, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.var_data(var).shape_function(mapped_coords);
    }
  };
};

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct ValueImpl;

/// Outer product
struct ValueElmOp : boost::proto::transform< ValueElmOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename ValueImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    
    result_type operator()(typename impl::expr_param var, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.var_data(var).value_elm(mapped_coords);
    }
  };
};

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct GradientImpl;

/// Element gradient
struct GradientElmOp : boost::proto::transform< GradientElmOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename GradientImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    
    result_type operator()(typename impl::expr_param var, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.var_data(var).gradient_elm(mapped_coords, data.support());
    }
  };
};

// Forward declaration
template<typename SF, Uint Dim, Uint Offset, Uint MatrixSize>
struct DivergenceImpl;

/// Element gradient
struct DivergenceElmOp : boost::proto::transform< DivergenceElmOp >
{
  template<typename VarT, typename MappedCoordsT, typename DataT>
  struct impl : boost::proto::transform_impl<VarT, MappedCoordsT, DataT>
  {
    typedef typename VarDataType<VarT, DataT>::type VarDataT;
    typedef typename DivergenceImpl<typename VarDataT::SF, VarDataT::dimension, VarDataT::offset, VarDataT::matrix_size>::result_type result_type;
    
    result_type operator()(typename impl::expr_param var, typename impl::state_param mapped_coords, typename impl::data_param data)
    {
      return data.var_data(var).divergence_elm(mapped_coords, data.support());
    }
  };
};

/// Static terminals that can be used in proto expressions
boost::proto::terminal<VolumeOp>::type const volume = {};

boost::proto::terminal<CoordinatesOp>::type const coordinates = {};
boost::proto::terminal<JacobianOp>::type const jacobian = {};
boost::proto::terminal<JacobianDeterminantOp>::type const jacobian_determinant = {};
boost::proto::terminal<NormalOp>::type const normal = {};
boost::proto::terminal<GradientOp>::type const gradient = {};

boost::proto::terminal<ShapeFunctionOp>::type const shape_function = {};

boost::proto::terminal<GradientElmOp>::type const gradient_elm = {};
boost::proto::terminal<DivergenceElmOp>::type const divergence_elm = {};
boost::proto::terminal<LaplacianElmOp>::type const laplacian_elm = {};
boost::proto::terminal<ValueElmOp>::type const value_elm = {};

/// SF operations needing only a support
struct SFSupportOp :
  boost::proto::terminal< VolumeOp >
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
    boost::proto::terminal<ShapeFunctionOp>,
    boost::proto::terminal<ValueElmOp>,
    boost::proto::terminal<GradientOp>,
    boost::proto::terminal<GradientElmOp>,
    boost::proto::terminal<DivergenceElmOp>,
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

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementOperations_hpp
