// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::CField"

#include <boost/foreach.hpp>
#include <boost/proto/proto.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/Log.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementNodes.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Mesh/SF/Types.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace boost;

namespace boost
{
  
namespace proto
{
  template<template<typename, typename> class Extends>
  struct pod_dummy_generator
  {
      BOOST_PROTO_CALLABLE()

      template<typename Sig>
      struct result;

      template<typename This, typename Expr>
      struct result<This(Expr)>
      {
          typedef Extends<Expr, is_proto_expr> type;
      };

      template<typename This, typename Expr>
      struct result<This(Expr &)>
      {
          typedef Extends<Expr, is_proto_expr> type;
      };

      template<typename This, typename Expr>
      struct result<This(Expr const &)>
      {
          typedef Extends<Expr, is_proto_expr> type;
      };

      /// \param expr The expression to wrap
      /// \return <tt>Extends\<Expr\> that = {expr}; return that;</tt>
      template<typename Expr>
      Extends<Expr, is_proto_expr> operator ()(Expr const &expr) const
      {
          Extends<Expr, is_proto_expr> that = {expr};
          return that;
      }
  };
} // namespace proto

} // namespace boost

// Experimental boost::proto stuff. Scroll down to the actual test to see the use
namespace CF
{

namespace Mesh
{

//////////////////////////////////////////////////////////
// Grammar that must be matched for an expression to be valid

// struct MeshGrammar
//   : proto::or_<
//         proto::terminal< Real >
//       , proto::plus< MeshGrammar, MeshGrammar >
//       , proto::minus< MeshGrammar, MeshGrammar >
//       , proto::multiplies< MeshGrammar, MeshGrammar >
//       , proto::plus_assign< MeshGrammar, MeshGrammar >
//       , proto::shift_left< proto::terminal< std::ostream & >, proto::_ >
//       , proto::shift_left< MeshGrammar, proto::_ >
//       , proto::subscript< proto::_, proto::_ > // TODO: deal with subscript properly
//     >
// {};

struct MeshGrammar : proto::_ {};
  
//////////////////////////////////////////////////////////
// Proto domain for mesh operations

/// Wrapper for expressions in the MeshDomain
template<class Expr, class Dummy>
struct MeshExpr;

struct MeshDomain
  : proto::domain<proto::pod_dummy_generator<MeshExpr>, MeshGrammar>
{};

template<class Expr, class Dummy = proto::is_proto_expr>
struct MeshExpr
{
  BOOST_PROTO_EXTENDS(Expr, MeshExpr<Expr>, MeshDomain)
};

//////////////////////////////////////////////////////////
// custom functions

// Wrap the volume function
struct jacobian_determinant_tag
{};

template<typename Arg>
typename proto::result_of::make_expr<
    jacobian_determinant_tag
  , MeshDomain
  , Arg const &
>::type const
jacobian_determinant(Arg const &arg)
{
  return proto::make_expr<jacobian_determinant_tag, MeshDomain>(boost::ref(arg));
}

// Wrap the volume function
struct jacobian_determinantV_tag
{};

template<typename Arg>
typename proto::result_of::make_expr<
    jacobian_determinantV_tag
  , MeshDomain
  , Arg const &
>::type const
jacobian_determinantV(Arg const &arg)
{
  return proto::make_expr<jacobian_determinantV_tag, MeshDomain>(boost::ref(arg));
}

struct volume_tag
{};

template<typename Arg>
typename proto::result_of::make_expr<
    volume_tag
  , MeshDomain
  , Arg const &
>::type const
volume(Arg const &arg)
{
  return proto::make_expr<volume_tag, MeshDomain>(boost::ref(arg));
}


template<Uint Order>
struct integral_tag
{};

template<Uint Order, typename Arg>
typename proto::result_of::make_expr<
    integral_tag<Order>
  , MeshDomain
  , Arg const &
>::type const
integral(Arg const &arg)
{
  return proto::make_expr<integral_tag<Order>, MeshDomain>(boost::ref(arg));
}

///////////////////////////////////////////////////////////
// Placeholders

// Wrap std::cout
proto::terminal< std::ostream & >::type cout_ = {std::cout};

// Placeholders for different types of data
struct ElementIdxHolder {};
MeshExpr<proto::terminal<ElementIdxHolder>::type> const elem_i = {{{}}}; // Represents an element index ({{{}}} makes this a static initialization)

struct ConstNodeViewHolder {};
MeshExpr<proto::terminal<ConstNodeViewHolder>::type> const const_node_v = {{{}}}; // Represents a const view of the element nodes

struct NodeVectorHolder {};
// unused // MeshExpr<proto::terminal<NodeVectorHolder>::type> const node_vec = {{{}}}; // Represents a copy of element nodes

struct MappedCoordHolder {};
MeshExpr<proto::terminal<MappedCoordHolder>::type> const mapped_c = {{{}}};

///////////////////////////////////////////////////////////
// Context

// The calculator_context from the "Hello Calculator" section,
// implemented from scratch.
template<typename ShapeFunctionT>
struct MeshContext
{
  MeshContext(const CArray& coords, const CTable::ArrayT& conn_table, const ElementType& etype, const ElementNodes& nodes) :
    coordinates(coords),
    connectivity(conn_table),
    element_type(etype),
    element_idx(0),
    element_node_vector(nodes)
  {
  }
  
  typedef ShapeFunctionT SF;
  typedef MeshContext<ShapeFunctionT> ThisContextT;
  
  /// Reference for the context
  const CArray& coordinates;
  const CTable::ArrayT& connectivity;
  const ElementType& element_type;
  Uint element_idx;
  RealVector mapped_coords;
  const ElementNodes& element_node_vector;
  
  template<typename Expr,
           typename Tag = typename Expr::proto_tag,
           typename Arg = typename Expr::proto_child0>
  struct eval
    : proto::default_eval<Expr, ThisContextT>
  {};
  
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, ConstNodeViewHolder>
  {
    typedef const ConstElementNodeView result_type;

    result_type operator()(Expr &, const MeshContext<ShapeFunctionT>& ctx) const
    {
      return ConstElementNodeView(ctx.coordinates, ctx.connectivity[ctx.element_idx]);
    }
  };
  
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, NodeVectorHolder>
  {
    typedef const ElementNodes& result_type;

    result_type operator()(Expr &, const MeshContext<ShapeFunctionT>& ctx) const
    {
      return ctx.element_node_vector;
    }
  };
  
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, ElementIdxHolder>
  {
    typedef Uint result_type;

    result_type operator()(Expr &, const MeshContext<ShapeFunctionT>& ctx) const
    {
      return ctx.element_idx;
    }
  };
  
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, MappedCoordHolder>
  {
    typedef const RealVector& result_type;

    result_type operator()(Expr &, const MeshContext<ShapeFunctionT>& ctx) const
    {
      return ctx.mapped_coords;
    }
  };
  
    // Handle volume function
  template<typename Expr, typename NoArg>
  struct eval<Expr, volume_tag, NoArg >
  {
    typedef Real result_type;

    result_type operator()(Expr& expr, const MeshContext<ShapeFunctionT>& ctx) const
    {
      return SF::volume(ctx.element_node_vector);
    }
  };
    
  template<typename Expr, typename MappedCoordsT>
  struct eval<Expr, jacobian_determinant_tag, MappedCoordsT >
  {
    typedef Real result_type;

    result_type operator()(Expr& expr, const MeshContext<ShapeFunctionT>& context) const
    {
      return SF::jacobian_determinant(context.mapped_coords, context.element_node_vector);
    }
  };
  
  template<typename Expr, typename MappedCoordsT>
  struct eval<Expr, jacobian_determinantV_tag, MappedCoordsT >
  {
    typedef Real result_type;

    result_type operator()(Expr& expr, const MeshContext<ShapeFunctionT>& context) const
    {
      return context.element_type.jacobian_determinantV(context.mapped_coords, context.element_node_vector);
    }
  };
  
  // Handle integration
  template<typename Expr, Uint I, typename ChildExpr>
  struct eval<Expr, integral_tag<I>, ChildExpr >
  {
    typedef typename boost::remove_const<typename boost::remove_reference<ChildExpr>::type>::type RealChildT;
    typedef typename proto::result_of::eval<RealChildT, ThisContextT>::type result_type;

    result_type operator()(Expr& expr, MeshContext<ShapeFunctionT>& context) const
    {
      result_type r;
      Integrators::integrate<I>(proto::child(expr), r, context);
      return r;
    }
  };
};


/// Looper defines a functor taking the type that boost::mpl::for_each
/// passes. It is the core of the looping mechanism.
template<typename ExpressionT, typename ExpressionEvaluatorT>
struct ElementLooper
{
public: // functions

  /// Constructor
  ElementLooper(const ExpressionT& expr, ExpressionEvaluatorT& evaluator, CElements& elements) : m_expr(expr), m_evaluator(evaluator), m_elements(elements) {}

  /// Operator
  template < typename ShapeFunctionT >
  void operator() ( ShapeFunctionT& T )
  {
    if(!IsElementType<ShapeFunctionT>()(m_elements.element_type()))
      return;
    
    const CArray& coords = m_elements.coordinates();
    const CTable::ArrayT& conn = m_elements.connectivity_table().array();
    ElementNodes nodes(m_elements.element_type().nb_nodes(), m_elements.element_type().dimension());
    MeshContext<ShapeFunctionT> context(coords, conn, m_elements.element_type(), nodes);
    
    for(context.element_idx = 0; context.element_idx != context.connectivity.size(); ++context.element_idx)
    {
      nodes.fill(coords, conn[context.element_idx]);
      m_evaluator.eval(m_expr, context);
    }
    
  }
private:
  const ExpressionT& m_expr;
  ExpressionEvaluatorT m_evaluator;
  CElements& m_elements;
};

/// Simply execute, do nothing with the result
struct NullEvaluator
{
  template<typename ExpressionT, typename ContextT>
  void eval(ExpressionT& expr, ContextT& context)
  {
    proto::eval(expr, context);
  }
};

/// Acuumulates results of a certain type
template<typename ResultT>
struct AccumulatingEvaluator
{
  AccumulatingEvaluator(ResultT& a_result) : result(a_result) {}
  
  template<typename ExpressionT, typename ContextT>
  void eval(ExpressionT& expr, ContextT& context)
  {
    result += proto::eval(expr, context);
  }
  
  ResultT& result;
};

/// List of all supported shapefunctions that allow high order integration
typedef boost::mpl::vector< SF::Line1DLagrangeP1,
                            SF::Quad2DLagrangeP1,
                            SF::Hexa3DLagrangeP1
> HigherIntegrationElements;

/// Applies the given expression to all elements using the supplied evaluator
template<typename ExpressionT, typename ExpressionEvaluatorT>
void for_all_elements(CMesh& mesh, ExpressionT& expr, ExpressionEvaluatorT& evaluator)
{
  BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(mesh))
  {
    boost::mpl::for_each<HigherIntegrationElements>(ElementLooper<ExpressionT, ExpressionEvaluatorT>(expr, evaluator, elements));
  }
}

/// executes an expression in all elements, without doing anything with the results
template<typename ExpressionT>
void for_all_elements_execute(CMesh& mesh, ExpressionT& expr)
{
  NullEvaluator evaluator;
  for_all_elements(mesh, expr, evaluator);
}

/// Accumulate a result of an expression
template<typename ExpressionT, typename ResultT>
void for_all_elements_accumulate(CMesh& mesh, ExpressionT& expr, ResultT& result)
{
  AccumulatingEvaluator<ResultT> evaluator(result);
  for_all_elements(mesh, expr, evaluator);
}
  
} // namespace Mesh

} // Namespace CF


struct ProtoOperatorsFixture : public Tools::Testing::ProfiledTestFixture, Tools::Testing::TimedTestFixture
{
  static CMesh::Ptr big_grid;
};

CMesh::Ptr ProtoOperatorsFixture::big_grid;

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ProtoBasics )
{
  CMesh::Ptr mesh(new CMesh("rect"));
  Tools::MeshGeneration::create_rectangle(*mesh, 5, 5, 5, 5);
  
  // Use the volume function
  for_all_elements_execute(*mesh, cout_ << "Volume for element " << elem_i << ": " << volume(elem_i) << "\n");
  std::cout << std::endl; // Can't be in expression
  
  // volume calculation
  Real vol1 = 0.;
  for_all_elements_accumulate(*mesh, volume(elem_i), vol1);
  
  CFinfo << "Mesh volume: " << vol1 << CFendl;
  
  // For an all-quad mesh, this is the same... cool or what?
  Real vol2 = 0.;
  for_all_elements_accumulate(*mesh,
                    0.5*((const_node_v[2][XX] - const_node_v[0][XX]) * (const_node_v[3][YY] - const_node_v[1][YY])
                      -  (const_node_v[2][YY] - const_node_v[0][YY]) * (const_node_v[3][XX] - const_node_v[1][XX])),
                             vol2);
  BOOST_CHECK_EQUAL(vol1, vol2);
  
  for_all_elements_execute(*mesh, cout_ << "test integral for element " << elem_i << ": " << integral<1>(jacobian_determinant(mapped_c)) <<  "\n");
  std::cout << std::endl;
}

// Must be run  before the next tests
BOOST_FIXTURE_TEST_CASE( CreateMesh, ProtoOperatorsFixture )
{
  ProtoOperatorsFixture::big_grid.reset(new CMesh("big_grid"));
  Tools::MeshGeneration::create_rectangle(*big_grid, 1., 1., 2000, 2000);
}

BOOST_FIXTURE_TEST_CASE( Volume, ProtoOperatorsFixture )
{
  Real vol = 0.;
  for_all_elements_accumulate(*big_grid, volume(elem_i), vol);
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Yes, this actually works!
BOOST_FIXTURE_TEST_CASE( VolumePlusAssign, ProtoOperatorsFixture )
{
  Real vol = 0.;
  for_all_elements_execute(*big_grid, vol += volume(elem_i));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

BOOST_FIXTURE_TEST_CASE( Integral, ProtoOperatorsFixture )
{
  Real vol = 0.;
  for_all_elements_accumulate(*big_grid, integral<1>(jacobian_determinant(mapped_c)), vol);
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

BOOST_FIXTURE_TEST_CASE( IntegralOrder4, ProtoOperatorsFixture )
{
  Real vol = 0.;
  for_all_elements_accumulate(*big_grid, integral<4>(jacobian_determinant(mapped_c)), vol);
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// BOOST_FIXTURE_TEST_CASE( IntegralOrder4Virtual, ProtoOperatorsFixture )
// {
//   Real vol = 0.;
//   for_all_elements_accumulate(*big_grid, integral<4>(jacobian_determinantV(mapped_c)), vol);
//   BOOST_CHECK_CLOSE(vol, 1., 0.0001);
// }

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
