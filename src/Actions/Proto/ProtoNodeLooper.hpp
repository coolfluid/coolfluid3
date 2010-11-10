// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoNodeLooper_hpp
#define CF_Actions_ProtoNodeLooper_hpp

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/sequence.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>

#include "Actions/Proto/ProtoNodeContexts.hpp"

#include "Mesh/CMesh.hpp"

namespace CF {
namespace Actions {
namespace Proto {

////////////////////////////////////////////////////////
// Helper transforms
///////////////////////////////////////////////////////

/// Wrap up a var type in its context
template<typename VarT>
struct AddNodeContext
{
  typedef NodeVarContext<VarT> type;
};

/// Initialize the contexts for numbered variables. Called when the CElements has changed
template<typename VarsT, typename ContextsT>
struct InitNodeContexts
{
  InitNodeContexts(const VarsT& vars, ContextsT& contexts, Mesh::CRegion& region) : m_vars(vars), m_contexts(contexts), m_region(region) {}
  
  template<typename I>
  void operator()(const I&)
  {
    boost::fusion::at<I>(m_contexts).init(boost::fusion::at<I>(m_vars), m_region);
  }
  
  const VarsT& m_vars;
  ContextsT& m_contexts;
  Mesh::CRegion& m_region;
};

/// Fill the numbered variables. Called when the element changed
template<typename VarsT, typename ContextsT>
struct FillNodeContexts
{
  FillNodeContexts(const VarsT& vars, ContextsT& contexts) : m_vars(vars), m_contexts(contexts), node_idx(0) {}
  
  template<typename I>
  void operator()(const I&)
  {
    boost::fusion::at<I>(m_contexts).fill(boost::fusion::at<I>(m_vars), node_idx);
  }
  
  const VarsT& m_vars;
  ContextsT& m_contexts;
  Uint node_idx;
};

/// Creates a list of unique nodes in the region
void make_node_list(const Mesh::CRegion& region, const Mesh::CArray& coordinates, std::vector<Uint>& nodes)
{
  std::vector<bool> node_is_used(coordinates.size(), false);
  
  // First count the number of unique nodes
  Uint nb_nodes = 0;
  BOOST_FOREACH(const Mesh::CElements& elements, Common::recursive_range_typed<Mesh::CElements>(region))
  {
    const Mesh::CTable& conn_tbl = elements.connectivity_table();
    const Uint nb_elems = conn_tbl.size();
    const Uint nb_elem_nodes = conn_tbl.row_size();
    
    for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
    {
      const Mesh::CTable::ConstRow row = conn_tbl[elem_idx];
      for(Uint node_idx = 0; node_idx != nb_elem_nodes; ++node_idx)
      {
        const Uint node = row[node_idx];
        if(!node_is_used[node])
        {
          node_is_used[node] = true;
          ++nb_nodes;
        }
      }
    }
  }
  
  // reserve space for all unique nodes
  nodes.clear();
  nodes.reserve(nb_nodes);
  
  // Add the unique node indices
  node_is_used.assign(coordinates.size(), false);
  BOOST_FOREACH(const Mesh::CElements& elements, Common::recursive_range_typed<Mesh::CElements>(region))
  {
    const Mesh::CTable& conn_tbl = elements.connectivity_table();
    const Uint nb_elems = conn_tbl.size();
    const Uint nb_nodes = conn_tbl.row_size();
    
    for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
    {
      const Mesh::CTable::ConstRow row = conn_tbl[elem_idx];
      for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
      {
        const Uint node = row[node_idx];
        if(!node_is_used[node])
        {
          node_is_used[node] = true;
          nodes.push_back(node);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////
// Public interface
///////////////////////////////////////////////////////

/// The actual looping function. Executes expr for each node.
template<typename Expr>
void for_each_node(Mesh::CRegion& region, const Expr& expr)
{
  // Number of variables
  typedef typename boost::result_of<ExprVarArity(Expr)>::type nb_vars;
  
  // init empty vector that will store variable indices
  typedef boost::mpl::vector_c<Uint> numbers_empty;
  
  // Fill the vector with indices 0 to 9, so we allow 10 different (field or node related) variables in an expression
  typedef typename boost::mpl::copy<
      boost::mpl::range_c<int,0,nb_vars::value>
    , boost::mpl::back_inserter< numbers_empty >
    >::type range_nb_vars;
  
  // Get the type for each variable that is used, or set to boost::mpl::void_ for unused indices
  typedef typename boost::mpl::transform<range_nb_vars, DefineTypeOp<boost::mpl::_1, Expr > >::type expr_types;
  
  typedef typename boost::fusion::result_of::as_vector<expr_types>::type FusionVarsT;
  
  FusionVarsT vars;
  CopyNumberedVars<FusionVarsT> ctx(vars);
  boost::proto::eval(expr, ctx);
  
  // Inititalize variable-specific contexts
  typedef typename boost::mpl::transform< FusionVarsT, AddNodeContext<boost::mpl::_1> >::type ContextsT;
  ContextsT contexts;
  
  InitNodeContexts<FusionVarsT, ContextsT> init_ctx(vars, contexts, region);
  boost::mpl::for_each<boost::mpl::range_c<int, 0, nb_vars::value> >(init_ctx);
  
  FillNodeContexts<FusionVarsT, ContextsT> fill_ctx(vars, contexts);
  
  // Create the global context
  NodeMeshContext<ContextsT> context(contexts);
  
  Mesh::CArray* coordinates = Common::get_component_typed_ptr<Mesh::CArray>(region, Common::IsComponentTag("coordinates")).get();
  if(coordinates) // region owns coordinates, so we assume a loop over all nodes
  {
    // Evaluate the expression for each node
    const Uint nb_nodes = coordinates->size();
    for(context.node_idx = 0; context.node_idx != nb_nodes; ++context.node_idx)
    {
      fill_ctx.node_idx = context.node_idx;
      boost::mpl::for_each<boost::mpl::range_c<int, 0, nb_vars::value> >(fill_ctx);
      EvaluateExpr()(expr, 0, context);
    }
  }
  else // no coordinates found, assert that all CElements share the same coords, and use it to construct a list of nodes to visit
  {
    BOOST_FOREACH(Mesh::CElements& elements, Common::recursive_range_typed<Mesh::CElements>(region))
    {
      if(coordinates)
      {
        cf_assert(coordinates == &elements.coordinates());
        continue;
      }
      coordinates = &elements.coordinates();
    }
    std::vector<Uint> nodes;
    make_node_list(region, *coordinates, nodes);
    const Uint nb_nodes = nodes.size();
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      context.node_idx = nodes[i];
      fill_ctx.node_idx = context.node_idx;
      boost::mpl::for_each<boost::mpl::range_c<int, 0, nb_vars::value> >(fill_ctx);
      EvaluateExpr()(expr, 0, context);
    }
  }
}

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoNodeLooper_hpp
