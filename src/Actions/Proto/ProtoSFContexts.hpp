// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoSFContexts_hpp
#define CF_Actions_ProtoSFContexts_hpp

#include "Actions/Proto/ProtoFunctions.hpp"
#include "Actions/Proto/ProtoTransforms.hpp"
#include "Actions/Proto/ProtoVariables.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Common/CF.hpp"

namespace CF {
namespace Actions {
namespace Proto {

/// Evaluates functions that depend on the shape functions
template<typename SF>
struct SFContext
{
  typedef typename SF::NodeMatrixT NodesT;
  typedef typename SF::MappedCoordsT MappedCoordsT;
 
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  SFContext(const NodesT& nodes_ref, const MappedCoordsT& mapped_coords) : 
    nodes(nodes_ref),
    mapped_coordinates(mapped_coords)
  {}
  
  typedef SFContext<SF> ThisContextT;
  typedef SF ShapeFunctionT;
  
  template<typename Expr,
           typename Tag = typename Expr::proto_tag,
           typename Arg = typename Expr::proto_child0>
  struct eval;
  
  /// Process volume
  template<typename Expr, typename ChildT>
  struct eval<Expr, sf_function_tag<volume_tag>, ChildT >
  {
    typedef Real result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return SF::volume(ctx.nodes);
    }
  };
  
  /// Process jacobian
  template<typename Expr, typename ChildT>
  struct eval<Expr, sf_function_tag<jacobian_tag>, ChildT >
  {
    typedef const typename SF::JacobianT& result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      SF::jacobian(ctx.mapped_coordinates, ctx.nodes, ctx.jacobian_matrix);
      return ctx.jacobian_matrix;
    }
  };
  
  /// Process jacobian_adjoint
  template<typename Expr, typename ChildT>
  struct eval<Expr, sf_function_tag<jacobian_adjoint_tag>, ChildT >
  {
    typedef const typename SF::JacobianT& result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      SF::jacobian_adjoint(ctx.mapped_coordinates, ctx.nodes, ctx.jacobian_adjoint_matrix);
      return ctx.jacobian_adjoint_matrix;
    }
  };
  
  /// Process jacobian determinant
  template<typename Expr, typename ChildT>
  struct eval<Expr, sf_function_tag<jacobian_determinant_tag>, ChildT >
  {
    typedef Real result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return SF::jacobian_determinant(ctx.mapped_coordinates, ctx.nodes);
    }
  };
  
  /// Process mapped_gradient
  template<typename Expr, typename ChildT>
  struct eval<Expr, sf_function_tag<mapped_gradient_tag>, ChildT >
  {
    typedef const typename SF::MappedGradientT& result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      SF::mapped_gradient(ctx.mapped_coordinates, ctx.mapped_gradient_matrix);
      return ctx.mapped_gradient_matrix;
    }
  };
  
  /// Process normal
  template<typename Expr, typename ChildT>
  struct eval<Expr, sf_function_tag<normal_tag>, ChildT >
  {
    typedef const typename SF::CoordsT& result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      SF::normal(ctx.mapped_coordinates, ctx.nodes, ctx.normal_vector);
      return ctx.normal_vector;
    }
  };
  
  /// Process Laplacian operator
  template<typename Expr, typename ChildT>
  struct eval<Expr, sf_function_tag<laplacian_tag>, ChildT >
  {
    typedef const Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes>& result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      SF::mapped_gradient(ctx.mapped_coordinates, ctx.mapped_gradient_matrix);
      SF::jacobian_adjoint(ctx.mapped_coordinates, ctx.nodes, ctx.jacobian_adjoint_matrix);
      
      ctx.laplacian_matrix = (ctx.mapped_gradient_matrix.transpose() * ctx.jacobian_adjoint_matrix.transpose()) * 
                             (ctx.jacobian_adjoint_matrix * ctx.mapped_gradient_matrix) / SF::jacobian_determinant(ctx.mapped_coordinates, ctx.nodes);
      
      return ctx.laplacian_matrix;
    }
  };
    
  const NodesT& nodes;
  const MappedCoordsT& mapped_coordinates;
  
  /// Temp storage for non-scalar results
  typename SF::JacobianT jacobian_matrix;
  typename SF::JacobianT jacobian_adjoint_matrix;
  
  typename SF::MappedGradientT mapped_gradient_matrix;
  
  typename SF::CoordsT normal_vector;
  
  Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> laplacian_matrix;
};

/// Calculation and storage of the shape function matrix for a given shape function
template<typename SF>
struct ShapeFunctionMatrix
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  const typename SF::ShapeFunctionsT& shape_function(const typename SF::MappedCoordsT& mapped_coords)
  {
    SF::shape_function(mapped_coords, m_shape_function);
    return m_shape_function;
  }
  
private:  
  typename SF::ShapeFunctionsT m_shape_function;
};

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoSFContexts_hpp
