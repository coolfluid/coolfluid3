// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoContexts_hpp
#define CF_Actions_ProtoContexts_hpp

#include "Actions/Proto/ProtoEigenContext.hpp"
#include "Actions/Proto/ProtoFunctions.hpp"
#include "Actions/Proto/ProtoSFContexts.hpp"
#include "Actions/Proto/ProtoTransforms.hpp"
#include "Actions/Proto/ProtoVariables.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Common/CF.hpp"

namespace boost {
namespace proto {
namespace result_of {
template<class Expr, class Context >
class eval;
}
}
}

namespace CF {
namespace Actions {
namespace Proto {

/// Fall-back context for numbered variables, doing nothing
/// This gets used in case there is a gap in the variable numbers, i.e.
/// if pressure has index 0 and temperature index 8, and nothing in between
/// is used.
template<typename SF, typename VarT>
struct ElementVarContext
{
  void init(const VarT&, const Mesh::CElements& elements, const typename SF::MappedCoordsT& mapped_coordinates)
  {
  }
  
  void fill(const Uint element_idx)
  {
  }
};

/// Evaluate const nodes numbered variables
template<typename SF>
struct ElementVarContext<SF, ConstNodes> : ShapeFunctionMatrix<SF>
{
  typedef SF ShapeFunctionT;
  typedef ElementVarContext<SF, ConstNodes> ThisContextT;
  typedef typename SF::NodeMatrixT NodeValueMatrixT;
  typedef SFContext<SF> SFContextT;
  typedef typename SF::MappedCoordsT MappedCoordsT;
  
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  ElementVarContext() : m_sf_context(0) {}
  
  ~ElementVarContext()
  {
    delete m_sf_context;
  }
  
  void init(const ConstNodes&, const Mesh::CElements& elements, const MappedCoordsT& mapped_coordinates)
  {
    coordinates = &elements.coordinates();
    connectivity = &elements.connectivity_table();
    delete m_sf_context;
    m_sf_context = new SFContext<SF>(nodes, mapped_coordinates);
  }
  
  void fill(const Uint element_idx)
  {
    Mesh::fill(nodes, *coordinates, (*connectivity)[element_idx]);
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
    typedef const NodeValueMatrixT& result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return ctx.nodes;
    }
  };
  
  const NodeValueMatrixT& node_value_matrix()
  {
    return nodes;
  }
  
  NodeValueMatrixT nodes;
  const Mesh::CTable<Real>* coordinates;
  const Mesh::CTable<Uint>* connectivity;
  
private:
  SFContextT* m_sf_context;
};

/// Evaluate scalar mutable nodal field data
template<typename SF>
struct ElementVarContext< SF, Field<Real> >
  : boost::proto::callable_context< ElementVarContext<SF, Field<Real> >, boost::proto::null_context>,
    ShapeFunctionMatrix<SF>
{
  typedef SF ShapeFunctionT;
  typedef Mesh::ElementNodeView<SF::nb_nodes, 1> NodesViewT;
  typedef NodesViewT& result_type;
  typedef typename SF::MappedCoordsT MappedCoordsT;
  typedef Eigen::Matrix<Real, SF::nb_nodes, 1> NodeValueMatrixT;
  
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  void init(const Field<Real>& placeholder, Mesh::CElements& elements, const MappedCoordsT& mapped_coordinates)
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
  
  const NodeValueMatrixT& node_value_matrix()
  {
    for(Uint node = 0; node != SF::nb_nodes; ++node)
      m_node_matrix[node] = node_view[node];
    return m_node_matrix;
  }
  
  Uint var_begin;
  NodesViewT node_view;
  Mesh::CTable<Real>* data;
  const Mesh::CTable<Uint>* connectivity;
private:
  NodeValueMatrixT m_node_matrix;
};

/// Context for evaluating mesh-related expressions, providing an interface to field variables and the shape functions
template<typename ShapeFunctionT, typename ContextsT>
struct ElementMeshContext
{
  typedef ShapeFunctionT SF;
  typedef ElementMeshContext<ShapeFunctionT, ContextsT> ThisContextT;
  typedef typename SF::MappedCoordsT MappedCoordsT;
  
  ElementMeshContext(ContextsT& ctxts, MappedCoordsT& mapped_coordinates) :
    element_idx(0)
  , contexts(ctxts)
  , mapped_coords(mapped_coordinates)
  {
    mapped_coords.resize(ShapeFunctionT::dimensionality, 0.);
  }
  
  /// Reference for the context
  Uint element_idx;
  ContextsT& contexts;
  MappedCoordsT& mapped_coords;
  int element_node_idx;
  
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
    // Lookup the type of the context for this numbered variable
    typedef typename boost::fusion::result_of::value_at<ContextsT, I>::type ContextT;
    
    // return type of the matrix product evaluation
    typedef typename Transform1x1MatrixToScalar
    <
      typename Eigen::ProductReturnType
      <
        typename ContextT::ShapeFunctionT::ShapeFunctionsT, typename ContextT::NodeValueMatrixT
      >::Type
    >::type result_type;
    
    result_type operator()(Expr& expr, ThisContextT& context)
    {
      ContextT& var_ctx = boost::fusion::at<I>(context.contexts);
      return 
      (
        var_ctx.shape_function(boost::proto::eval(boost::proto::right(expr), context)) * // This is the shape function matrix
        var_ctx.node_value_matrix() // these are the nodal values, in the form of an Eigen matrix
      );
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
    typedef const MappedCoordsT& result_type;

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
  
  /// Handle Eigen matrix functions
  template<typename Expr, typename TagT, typename ChildExpr>
  struct eval<Expr, eigen_function_tag<TagT>, ChildExpr >
  {
    // return type of the evaluation
    typedef typename boost::proto::result_of::eval< Expr, EigenContext<ThisContextT> >::type result_type;
    
    result_type operator()(Expr &expr, ThisContextT& ctx)
    {
      return boost::proto::eval(expr, EigenContext<ThisContextT>(ctx));
    }
  };
  
  /// Handle integration
  template<typename Expr, Uint I, typename ChildExpr>
  struct eval<Expr, integral_tag<I>, ChildExpr >
  {
    typedef typename boost::proto::result_of::child<Expr>::type ChildT;
    
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::eval
        <
          ChildT, ThisContextT
        >::type
      >::type
    >::type EigenExprT;
    
    typedef ValueType
    <
      EigenExprT
    > ValueT;
    
    typedef typename ValueT::type result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      result_type r;
      ValueT::set_zero(r);
      Mesh::Integrators::gauss_integrate<I, SF::shape>(integration_ftor(boost::proto::child(expr), context), context.mapped_coords, r);
      return r;
    }
    
    struct integration_ftor
    {
      integration_ftor(const ChildT& expr, ThisContextT& ctx) : m_expr(expr), m_context(ctx) {}
      
      inline EigenExprT operator()() const
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
    // Index of the variable that is used in the function
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value<NodesArg>::type
      >::type
    >::type::index_type I;
    
    // Context that will evaluate the function
    typedef typename boost::fusion::result_of::value_at<ContextsT, I>::type ContextT;
    
    static const int nb_nodes = static_cast<int>(ContextT::ShapeFunctionT::nb_nodes);
    
    typedef void result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      for(context.element_node_idx = 0; context.element_node_idx != nb_nodes; ++context.element_node_idx)
        boost::proto::eval(boost::proto::right(expr), context);
    }
  };
  
  /// Get node index
  template<typename Expr, typename NodesArg>
  struct eval<Expr, node_idx_tag, NodesArg >
  {
    // Index of the variable that is used in the function
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value<NodesArg>::type
      >::type
    >::type::index_type I;
    
    // Context that will provide the connectivity to get the node index
    typedef typename boost::fusion::result_of::value_at<ContextsT, I>::type ContextT;
    
    typedef Uint result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      return (*boost::fusion::at<I>(context.contexts).connectivity)[context.element_idx][context.element_node_idx];
    }
  };
  
  /// Delegate multiplication to an Eigen-specific context, if needed
  template<typename Expr, typename ChildT>
  struct eval< Expr, boost::proto::tag::multiplies, ChildT >
  {
    typedef typename boost::proto::result_of::left<Expr>::type LeftExprT;
    typedef typename boost::proto::result_of::right<Expr>::type RightExprT;
    
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::eval<LeftExprT, ThisContextT>::type
      >::type
    >::type LeftT;
    
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::eval<RightExprT, ThisContextT>::type
      >::type
    >::type RightT;
    
    typedef EigenMultiplier<LeftT, RightT> MultiplierT;
    typedef typename MultiplierT::result_type result_type;
    
    inline result_type operator()(Expr& expr, ThisContextT& context)
    {
      return MultiplierT().exec(boost::proto::eval(boost::proto::left(expr), context),
                                boost::proto::eval(boost::proto::right(expr), context));
    }
  };
};

} // Proto
} // Actions
} // CF

#endif // CF_Actions_ProtoContexts_hpp
