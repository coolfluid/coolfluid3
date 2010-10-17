// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoContexts_hpp
#define CF_Actions_ProtoContexts_hpp

#include "Actions/ProtoFunctions.hpp"
#include "Actions/ProtoSFContexts.hpp"
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

/// Fall-back context for numbered variables, doing nothing
/// This gets used in case there is a gap in the variable numbers, i.e.
/// if pressure has index 0 and temperature index 8, and nothing in between
/// is used.
template<typename SF, typename VarT>
struct ElementVarContext
{
  void init(const VarT&, const Mesh::CElements& elements, const RealVector& mapped_coordinates)
  {
  }
  
  void fill(const Uint element_idx)
  {
  }
};

/// Evaluate const nodes numbered variables
template<typename SF>
struct ElementVarContext<SF, ConstNodes>
{
  typedef ElementVarContext<SF, ConstNodes> ThisContextT;
  typedef Mesh::ElementNodeValues<SF::nb_nodes, SF::dimension> NodesT;
  typedef SFContext<SF> SFContextT;
  typedef RealVector data_type;
  
  ElementVarContext() : m_sf_context(0) {}
  
  ~ElementVarContext()
  {
    delete m_sf_context;
  }
  
  void init_data(data_type& data)
  {
    data.resize(SF::dimension);
  }
  
  void init(const ConstNodes&, const Mesh::CElements& elements, const RealVector& mapped_coordinates)
  {
    coordinates = &elements.coordinates();
    connectivity = &elements.connectivity_table();
    delete m_sf_context;
    m_sf_context = new SFContext<SF>(nodes, mapped_coordinates);
  }
  
  void fill(const Uint element_idx)
  {
    nodes.fill(*coordinates, (*connectivity)[element_idx]);
  }
  
  SFContextT& sf_context()
  {
    return *m_sf_context;
  }
  
  template<typename Expr,
           typename Tag = typename Expr::proto_tag,
           typename Arg = typename Expr::proto_child0>
  struct eval;
  
  template<typename Expr, typename I>
  struct eval< Expr, boost::proto::tag::terminal, Var<I, ConstNodes> >
  {
    typedef const NodesT& result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return ctx.nodes;
    }
  };
  
  NodesT nodes;
  const RealVector* mapped_coordinates;
  const Mesh::CArray* coordinates;
  const Mesh::CTable* connectivity;
  
private:
  SFContextT* m_sf_context;
};

/// Evaluate scalar mutable nodal field data
template<typename SF>
struct ElementVarContext< SF, Field<Real> >
  : boost::proto::callable_context< ElementVarContext<SF, Field<Real> >, boost::proto::null_context>
{
  typedef SF ShapeFunctionT;
  typedef Mesh::ElementNodeView<SF::nb_nodes, 1> NodesViewT;
  typedef NodesViewT& result_type;
  typedef Real data_type;
  
  void init_data(Real& data)
  {
    data = 0.;
  }
  
  void init(const Field<Real>& placeholder, Mesh::CElements& elements, const RealVector& mapped_coordinates)
  {
    Mesh::CFieldElements& field_elems = elements.get_field_elements(placeholder.field_name);
    data = &field_elems.data();
    connectivity = &field_elems.connectivity_table();
    
    
    Mesh::CField::ConstPtr field = boost::dynamic_pointer_cast<Mesh::CField const>(field_elems.get_parent());
    cf_assert(field);
    var_begin = field->var_index(placeholder.var_name);
    cf_assert(field->var_length(placeholder.var_name) == 1);
  }
  
  void fill(const Uint element_idx)
  {
    node_view.fill(*data, (*connectivity)[element_idx], var_begin);
  }
  
  template<typename I>
  result_type operator()(boost::proto::tag::terminal, const Var<I, Field<Real> >&)
  {
    return node_view;
  }
  
  Uint var_begin;
  NodesViewT node_view;
  Mesh::CArray* data;
  const Mesh::CTable* connectivity;
};
/*
/// Evaluate scalar mutable nodal field data
template<typename SF>
struct ElementVarContext< SF, Field<RealMatrix> >
  : boost::proto::callable_context< ElementVarContext<SF, Field<RealMatrix> >, boost::proto::null_context>
{
  typedef SF ShapeFunctionT;
  typedef Mesh::ElementNodeTensorViewD<SF::nb_nodes> NodesViewT;
  typedef NodesViewT& result_type;
  typedef RealMatrix data_type;
  
  void init_data(RealMatrix& data)
  {
    data.resize(matrix_dim, matrix_dim);
  }
  
  void init(const Field<RealMatrix>& placeholder, Mesh::CElements& elements)
  {
    Mesh::CFieldElements& field_elems = elements.get_field_elements(placeholder.field_name);
    data = &field_elems.data();
    connectivity = &field_elems.connectivity_table();
    
    Mesh::CField::ConstPtr field = boost::dynamic_pointer_cast<Mesh::CField const>(field_elems.get_parent());
    cf_assert(field);
    var_begin = field->var_index(placeholder.var_name);
    var_length = field->var_length(placeholder.var_name);
    matrix_dim = static_cast<Uint>(sqrt(static_cast<Real>(var_length)));
    node_view.init(matrix_dim, matrix_dim);
  }
  
  void fill(const Uint element_idx)
  {
    node_view.fill(*data, (*connectivity)[element_idx], var_begin);
  }
  
  template<typename I>
  result_type operator()(boost::proto::tag::terminal, const Var<I, Field<RealMatrix> >&)
  {
    return node_view;
  }
  
  Uint matrix_dim;
  Uint var_begin;
  Uint var_length;
  NodesViewT node_view;
  Mesh::CArray* data;
  const Mesh::CTable* connectivity;
};
*/
template<typename ShapeFunctionT>
struct MeshSizeContext;

template<typename T>
struct error_printer {};

/// Context for evaluating mesh-related expressions, providing an interface to field variables and the shape functions
template<typename ShapeFunctionT, typename ContextsT>
struct ElementMeshContext
{
  ElementMeshContext(ContextsT& ctxts, RealVector& mapped_coordinates) :
    element_idx(0)
  , contexts(ctxts)
  , mapped_coords(mapped_coordinates)
  {
    mapped_coords.resize(ShapeFunctionT::dimensionality, 0.);
  }
  
  typedef ShapeFunctionT SF;
  typedef ElementMeshContext<ShapeFunctionT, ContextsT> ThisContextT;
  
  /// Reference for the context
  Uint element_idx;
  RealVector real_coords;
  RealVector surface_normal;
  ContextsT& contexts;
  RealVector& mapped_coords;
  Uint element_node_idx;
  RealMatrix jacobian_matrix;
  RealMatrix jacobian_adjoint_matrix;
  RealMatrix mapped_gradient_matrix;
  
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
    typedef typename boost::fusion::result_of::value_at<ContextsT, I>::type ContextT;
    typedef typename boost::proto::result_of::eval<Expr, ContextT>::type result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return boost::proto::eval(expr, boost::fusion::at<I>(ctx.contexts));
    }
  };
  
  /// By default, functions are evaluated through the proto default context
  template<typename Expr, typename Tag, typename T>
  struct FunEval : boost::proto::default_eval<Expr, ThisContextT>
  {};
  
  /// Numbered variables are functors that take mapped coordinates as their argument
  template<typename Expr, typename I, typename T>
  struct FunEval< Expr, boost::proto::tag::terminal, Var<I, T> >
  {
    typedef typename boost::fusion::result_of::value_at<ContextsT, I>::type ContextT;
    typedef typename ContextT::data_type result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      result_type r;
      boost::fusion::at<I>(context.contexts).init_data(r);
      ::CF::Mesh::eval<SF>
      (
        boost::proto::eval(boost::proto::right(expr), context), // should evaluate to mapped coordinates
        boost::proto::eval(boost::proto::left(expr), context), // should evaluate to the nodal values
        r
      );
      return r;
    }
  };
  
  template<typename Tag, typename T>
  struct ChildOrValue
  {
    typedef typename boost::proto::result_of::child<T>::type type;
  };
  
  template<typename T>
  struct ChildOrValue<boost::proto::tag::terminal, T>
  {
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value<T>::type
      >::type
    >::type type;
  };
  
  
  /// Choose between normal and numbered variable functor evaluation
  template<typename Expr, typename T>
  struct eval<Expr, boost::proto::tag::function, T > :
    FunEval
    <
      Expr,
      typename boost::proto::tag_of<T>::type,
      typename ChildOrValue<typename boost::proto::tag_of<T>::type, T>::type
    >
  {};
  
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
  
  /// Placeholder that evaluates to the current relative node index in the element
  template<typename Expr>
  struct eval<Expr, boost::proto::tag::terminal, ElementNodeIdxHolder>
  {
    typedef Uint result_type;

    result_type operator()(Expr &, const ThisContextT& ctx) const
    {
      return ctx.element_node_idx;
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
  
  /// Handle shape function functions
  template<typename Expr, typename TagT, typename ChildExpr>
  struct eval<Expr, sf_function_tag<TagT>, ChildExpr >
  {
    // Index of the variable that is used in the function
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value<ChildExpr>::type
      >::type
    >::type::index_type I;
    
    // Context that will evaluate the function
    typedef typename boost::fusion::result_of::value_at<ContextsT, I>::type ContextT;
    
    // return type of the evaluation
    typedef typename boost::proto::result_of::eval<Expr, typename ContextT::SFContextT>::type result_type;
    
    result_type operator()(Expr &expr, ThisContextT& ctx)
    {
      return boost::proto::eval(expr, boost::fusion::at<I>(ctx.contexts).sf_context());
    }
  };
  
  /// Handle integration
  template<typename Expr, Uint I, typename ChildExpr>
  struct eval<Expr, integral_tag<I>, ChildExpr >
  {
    typedef typename boost::proto::result_of::child<Expr>::type ChildT;
    
    typedef typename boost::proto::result_of::eval
    <
      ChildT, MeshSizeContext<SF>
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
      integration_ftor(const ChildT& expr, ThisContextT& ctx) : m_expr(expr), m_context(ctx) {}
      
      inline result_type operator()() const
      {
        return boost::proto::eval(m_expr, m_context);
      }
      
      const ChildT& m_expr;
      ThisContextT& m_context;
    };
  };
  
  /// Loop over element nodes
  template<typename Expr, typename NodesArg>
  struct eval<Expr, for_each_node_tag, NodesArg >
  {
    // First child represents the nodes or states
    typedef typename boost::proto::result_of::eval
    <
      typename boost::proto::result_of::left<Expr>::type, ThisContextT
    >::type NodesT;
    
    typedef void result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      NodesT nodes = boost::proto::eval(boost::proto::left(expr), context);
      for(context.element_node_idx = 0; context.element_node_idx != nodes.size(); ++context.element_node_idx)
        boost::proto::eval(boost::proto::right(expr), context);
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
  
  /// Handle shape function functions
  template<typename Expr, typename TagT, typename ChildExpr>
  struct eval<Expr, sf_function_tag<TagT>, ChildExpr >
  {
    // return type of the evaluation
    typedef typename SizeOfResult<SF, TagT>::type result_type;
    
    result_type operator()(Expr &expr, ThisContextT& ctx)
    {
      return result_type();
    }
  };
  
  /// Handle integration
  template<typename Expr, Uint I, typename ChildExpr>
  struct eval<Expr, integral_tag<I>, ChildExpr >
  {
    typedef typename boost::proto::result_of::eval
    <
      typename boost::proto::result_of::child<Expr>::type, ThisContextT
    >::type result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      return result_type();
    }
  };
};

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoContexts_hpp
