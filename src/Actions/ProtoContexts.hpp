// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoContexts_hpp
#define CF_Actions_ProtoContexts_hpp

#include "Actions/ProtoFunctions.hpp"
#include "Actions/ProtoTransforms.hpp"
#include "Actions/ProtoVariables.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Common/CF.hpp"

namespace CF {
namespace Actions {

/// Fall-back context for numbered variables, doing nothing
/// This gets used in case there is a gap in the variable numbers, i.e.
/// if pressure has index 0 and temperature index 8, and nothing in between
/// is used.
template<typename SF, typename VarT>
struct VarContext
{
  void init(const VarT&, const Mesh::CElements& elements)
  {
  }
  
  void fill(const VarT&, const Uint element_idx)
  {
  }
};

/// Evaluate const nodes numbered variables
template<typename SF>
struct VarContext<SF, ConstNodes>
  : boost::proto::callable_context< VarContext<SF, ConstNodes> const, boost::proto::null_context const>
{
  typedef SF ShapeFunctionT;
  typedef Mesh::ElementNodeValues<SF::nb_nodes, SF::dimension> NodesT;
  typedef const NodesT& result_type;
  
  void init(const ConstNodes&, const Mesh::CElements& elements)
  {
    coordinates = &elements.coordinates();
    connectivity = &elements.connectivity_table();
  }
  
  void fill(const ConstNodes&, const Uint element_idx)
  {
    nodes.fill(*coordinates, (*connectivity)[element_idx]);
  }
  
  template<typename I>
  result_type operator()(boost::proto::tag::terminal, const Var<I, ConstNodes>&) const
  {
    return nodes;
  }
  
  NodesT nodes;
  const Mesh::CArray* coordinates;
  const Mesh::CTable* connectivity;
};

template<typename ShapeFunctionT>
struct MeshSizeContext;

template<typename T> struct printer {};

/// Context for evaluating mesh-related expressions, providing an interface to field variables and the shape functions
template<typename ShapeFunctionT, typename ContextsT>
struct MeshContext
{
  MeshContext(ContextsT& ctxts) :
    element_idx(0)
  , mapped_coords(0., ShapeFunctionT::dimensionality) // mapped coords default to centroid
  , contexts(ctxts)
  {
  }
  
  typedef ShapeFunctionT SF;
  typedef MeshContext<ShapeFunctionT, ContextsT> ThisContextT;
  
  /// Reference for the context
  Uint element_idx;
  RealVector mapped_coords;
  RealVector real_coords;
  RealVector surface_normal;
  ContextsT& contexts;
  
  template<typename Expr,
           typename Tag = typename Expr::proto_tag,
           typename Arg = typename Expr::proto_child0>
  struct eval
    : boost::proto::default_eval<Expr, ThisContextT>
  {};
  
  /// Process numbered variables
  template<typename Expr, typename I, typename T>
  struct eval< Expr, boost::proto::tag::terminal, Var<I, T> >
  {
    typedef typename boost::fusion::result_of::value_at<ContextsT, I>::type::result_type result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return boost::proto::eval(expr, boost::fusion::at<I>(ctx.contexts));
    }
  };
  
  /// Placeholder that evaluates to the current element index
  template<typename Expr>
  struct eval<Expr, boost::proto::tag::terminal, ElementIdxHolder>
  {
    typedef Uint result_type;

    result_type operator()(Expr &, const ThisContextT& ctx) const
    {
      return ctx.element_idx;
    }
  };
  
  /// Placeholder that evaluates to the current mapped coordinates
  template<typename Expr>
  struct eval<Expr, boost::proto::tag::terminal, MappedCoordHolder>
  {
    typedef const RealVector& result_type;

    result_type operator()(Expr &, const ThisContextT& ctx) const
    {
      return ctx.mapped_coords;
    }
  };
  
  /// volume function
  template<typename Expr, typename NodesT>
  struct eval<Expr, volume_tag, NodesT >
  {
    typedef Real result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return SF::volume(boost::proto::eval(boost::proto::child(expr), ctx));
    }
  };
  
  /// jacobian determinant function
  template<typename Expr, typename MappedCoordsT>
  struct eval<Expr, jacobian_determinant_tag, MappedCoordsT >
  {
    typedef Real result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      return SF::jacobian_determinant(context.mapped_coords, boost::proto::eval(boost::proto::child_c<1>(expr), context));
    }
  };
  
  /// normal vector calculation
  template<typename Expr, typename MappedCoordsT>
  struct eval<Expr, normal_tag, MappedCoordsT >
  {
    typedef const RealVector& result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      context.surface_normal.resize(SF::dimension);
      SF::normal(context.mapped_coords, boost::proto::eval(boost::proto::child_c<1>(expr), context), context.surface_normal);
      return context.surface_normal;
    }
  };
  
  /// Handle integration
  template<typename Expr, Uint I, typename ChildExpr>
  struct eval<Expr, integral_tag<I>, ChildExpr >
  {
    typedef typename boost::remove_const<typename boost::remove_reference<ChildExpr>::type>::type RealChildT;
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::eval
        <
          RealChildT, MeshSizeContext<SF>
        >::type
      >::type
    >::type TypeSizeT;
    
    typedef typename TypeSizeT::result_type result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      result_type r;
      TypeSizeT::init(r);
      Mesh::Integrators::gauss_integrate<I, SF::shape>(integration_ftor(boost::proto::child(expr), context), context.mapped_coords, r);
      return r;
    }
    
    struct integration_ftor
    {
      integration_ftor(const RealChildT& expr, ThisContextT& ctx) : m_expr(expr), m_context(ctx) {}
      
      inline result_type operator()() const
      {
        return boost::proto::eval(m_expr, m_context);
      }
      
      const RealChildT& m_expr;
      ThisContextT& m_context;
    };
  };
  
  /// Convert the current mapped coords to real coords
  template<typename Expr, typename ChildExpr>
  struct eval<Expr, coords_tag, ChildExpr >
  {
    typedef typename boost::remove_const<typename boost::remove_reference<ChildExpr>::type>::type RealChildT;
    typedef const RealVector& result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      context.real_coords.resize(SF::dimension);
      ::CF::Mesh::eval<SF>(context.mapped_coords, boost::proto::eval(boost::proto::child_c<1>(expr), context), context.real_coords);
      return context.real_coords;
    }
  };
};

/// Context for detrmining the result size and type of mesh expressions
template<typename ShapeFunctionT>
struct MeshSizeContext
{ 
  typedef ShapeFunctionT SF;
  typedef MeshSizeContext<ShapeFunctionT> ThisContextT;
  
  template<typename Expr,
           typename Tag = typename Expr::proto_tag,
           typename Arg = typename Expr::proto_child0>
  struct eval
    : boost::proto::default_eval<Expr, ThisContextT>
  {};
  
  /// Unless specialized, terminals evaluate to a 1x1 value
  template<typename Expr, typename T>
  struct eval< Expr, boost::proto::tag::terminal, T >
  {
    typedef ResultSize<1, 1> result_type;
    
    result_type operator()(Expr &, ThisContextT&)
    {
      return result_type();
    }
  };
  
  /// ConstNodes encapsulates vector types
  template<typename Expr, typename I>
  struct eval< Expr, boost::proto::tag::terminal, Var<I, ConstNodes> >
  {
    typedef ResultSize<SF::dimension, 1> result_type;
    
    result_type operator()(Expr &, ThisContextT&)
    {
      return result_type();
    }
  };
  
  /// Placeholder that evaluates to the current element index
  template<typename Expr>
  struct eval<Expr, boost::proto::tag::terminal, ElementIdxHolder>
  {
    typedef ResultSize<1, 1> result_type;

    result_type operator()(Expr &, const ThisContextT&) const
    {
      return result_type();
    }
  };
  
  /// Placeholder that evaluates to the current mapped coordinates
  template<typename Expr>
  struct eval<Expr, boost::proto::tag::terminal, MappedCoordHolder>
  {
    typedef ResultSize<SF::dimensionality, 1> result_type;

    result_type operator()(Expr &, const ThisContextT&) const
    {
      return result_type();
    }
  };
  
  /// volume function
  template<typename Expr, typename NodesT>
  struct eval<Expr, volume_tag, NodesT >
  {
    typedef ResultSize<1, 1> result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return result_type();
    }
  };
  
  /// jacobian determinant function
  template<typename Expr, typename MappedCoordsT>
  struct eval<Expr, jacobian_determinant_tag, MappedCoordsT >
  {
    typedef ResultSize<1, 1> result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return result_type();
    }
  };
  
  /// normal vector calculation
  template<typename Expr, typename MappedCoordsT>
  struct eval<Expr, normal_tag, MappedCoordsT >
  {
    typedef ResultSize<SF::dimension, 1> result_type;
    
    result_type operator()(Expr &, ThisContextT&)
    {
      return result_type();
    }
  };
  
  /// Handle integration
  template<typename Expr, Uint I, typename ChildExpr>
  struct eval<Expr, integral_tag<I>, ChildExpr >
  {
    typedef typename boost::remove_const<typename boost::remove_reference<ChildExpr>::type>::type RealChildT;
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename StripExpr
        <
          typename boost::proto::result_of::eval
          <
            RealChildT, ThisContextT
          >::type
        >::type
      >::type
    >::type result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      return result_type();
    }
  };
  
  /// Convert the current mapped coords to real coords
  template<typename Expr, typename ChildExpr>
  struct eval<Expr, coords_tag, ChildExpr >
  {
    typedef ResultSize<SF::dimension, 1> result_type;
    
    result_type operator()(Expr &, ThisContextT&)
    {
      return result_type();
    }
  };
};

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoContexts_hpp
