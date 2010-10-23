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
  
  Mesh::CArray& coordinates = Common::get_component_typed<Mesh::CArray>(region, Common::IsComponentTag("coordinates"));
  
  // Create the global context
  NodeMeshContext<ContextsT> context(contexts);
  
  // Evaluate the expression for each node
  const Uint nb_nodes = coordinates.size();
  for(context.node_idx = 0; context.node_idx != nb_nodes; ++context.node_idx)
  {
    fill_ctx.node_idx = context.node_idx;
    boost::mpl::for_each<boost::mpl::range_c<int, 0, nb_vars::value> >(fill_ctx);
    boost::proto::eval(expr, context);
  }
}

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoNodeLooper_hpp
