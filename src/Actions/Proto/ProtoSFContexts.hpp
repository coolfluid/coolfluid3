// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoSFContexts_hpp
#define CF_Actions_ProtoSFContexts_hpp

#include "Actions/ProtoFunctions.hpp"
#include "Actions/ProtoTransforms.hpp"
#include "Actions/ProtoVariables.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Common/CF.hpp"

namespace CF {
namespace Actions {

/// Evaluates functions that depend on the shape functions
template<typename SF>
struct SFContext
{
  typedef Mesh::ElementNodeValues<SF::nb_nodes, SF::dimension> NodesT;
  SFContext(const NodesT& nodes_ref, const RealVector& mapped_coords) : 
    nodes(nodes_ref),
    mapped_coordinates(mapped_coords),
    jacobian_matrix(SF::dimensionality, SF::dimension),
    jacobian_adjoint_matrix(SF::dimension, SF::dimension),
    mapped_gradient_matrix(SF::dimension, SF::nb_nodes),
    normal_vector(SF::dimension),
    laplacian_matrix(SF::nb_nodes, SF::nb_nodes),
    gradient_real(SF::dimension, SF::nb_nodes),
    gradient_real_transposed(SF::nb_nodes, SF::dimension)
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
    typedef const RealMatrix& result_type;

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
    typedef const RealMatrix& result_type;

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
    typedef const RealMatrix& result_type;

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
    typedef const RealVector& result_type;

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
    typedef const RealMatrix& result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      SF::mapped_gradient(ctx.mapped_coordinates, ctx.mapped_gradient_matrix);
      SF::jacobian_adjoint(ctx.mapped_coordinates, ctx.nodes, ctx.jacobian_adjoint_matrix);
      
      ctx.gradient_real = ctx.jacobian_adjoint_matrix * ctx.mapped_gradient_matrix;
      ctx.gradient_real.transpose(ctx.gradient_real_transposed);
      
      ctx.laplacian_matrix = (ctx.gradient_real_transposed * ctx.gradient_real) / SF::jacobian_determinant(ctx.mapped_coordinates, ctx.nodes);
      
      return ctx.laplacian_matrix;
    }
  };
    
  const NodesT& nodes;
  const RealVector& mapped_coordinates;
  
  /// Temp storage for non-scalar results
  RealMatrix jacobian_matrix;
  RealMatrix jacobian_adjoint_matrix;
  
  RealMatrix mapped_gradient_matrix;
  
  RealVector normal_vector;
  
  RealMatrix laplacian_matrix;
  RealMatrix gradient_real;
  RealMatrix gradient_real_transposed;
};

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoSFContexts_hpp
