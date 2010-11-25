// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoNodeContexts_hpp
#define CF_Actions_ProtoNodeContexts_hpp

#include "Actions/Proto/ProtoFunctions.hpp"
#include "Actions/Proto/ProtoTransforms.hpp"
#include "Actions/Proto/ProtoVariables.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Common/CF.hpp"

namespace CF {
namespace Actions {
namespace Proto {

/// Fall-back context for numbered variables, doing nothing
/// This gets used in case there is a gap in the variable numbers, i.e.
/// if pressure has index 0 and temperature index 8, and nothing in between
/// is used.
template<typename VarT>
struct NodeVarContext
{ 
  void init(const VarT&, const Mesh::CRegion& region)
  {
  }
  
  void fill(const VarT&, const Uint node_idx)
  {
  }
};

/// Evaluate const nodes numbered variables
template<>
struct NodeVarContext<ConstNodes>
  : boost::proto::callable_context< NodeVarContext<ConstNodes> const, boost::proto::null_context const>
{
  typedef const RealVector& result_type;
  
  void init(const ConstNodes& placeholder, Mesh::CRegion& region)
  {
    // use a little hack to get to the coords
    coordinates = Common::get_component_typed_ptr<Mesh::CTable<Real> >(region, Common::IsComponentTag("coordinates")).get();
    if(!coordinates)
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
    }
    dim = coordinates->row_size();
    nodes.resize(dim);
  }
  
  void fill(const ConstNodes&, const Uint idx)
  {
    Mesh::CTable<Real>::ConstRow row = (*coordinates)[idx];
    for(Uint i = 0; i != dim; ++i)
      nodes[i] = row[i];
  }
  
  template<typename I>
  result_type operator()(boost::proto::tag::terminal, const Var<I, ConstNodes>&) const
  {
    return nodes;
  }
  
  RealVector nodes;
  Uint dim;
  const Mesh::CTable<Real>* coordinates;
};

/// Evaluate scalar mutable nodal field data
template<>
struct NodeVarContext<Field<Real> >
  : boost::proto::callable_context< NodeVarContext<Field<Real> >, boost::proto::null_context>
{
  typedef Real& result_type;
  
  void init(const Field<Real>& placeholder, Mesh::CRegion& region)
  {
    Mesh::CField& field = region.get_field(placeholder.field_name);
    data = &Common::get_component_typed<Mesh::CTable<Real> >(field, Common::IsComponentTag("field_data"));

    var_begin = field.var_index(placeholder.var_name);
    cf_assert(field.var_length(placeholder.var_name) == 1);
  }
  
  void fill(const Field<Real>&, const Uint idx)
  {
    value = &(*data)[idx][var_begin];
  }
  
  template<typename I>
  result_type operator()(boost::proto::tag::terminal, const Var<I, Field<Real> >&)
  {
    return *value;
  }
  
  Uint var_begin;
  Mesh::CTable<Real>* data;
  Real* value;
};

/// Context for evaluating mesh-related expressions
template<typename ContextsT>
struct NodeMeshContext
{
  NodeMeshContext(ContextsT& ctxts) :
    node_idx(0)
  , contexts(ctxts)
  {
  }
  
  typedef NodeMeshContext<ContextsT> ThisContextT;
  
  /// Reference for the context
  Uint node_idx;
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
};

} // Proto
} // Actions
} // CF

#endif // CF_Actions_ProtoNodeContexts_hpp
