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

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace boost;

// Experimental boost::proto stuff. Scroll down to the actual test to see the use
namespace CF
{

namespace Mesh
{

//////////////////////////////////////////////////////////
// functors

// Wrap the volume function
struct volume_fun
{
  typedef Real result_type;
  template<typename ShapeFunctionT, typename NodesT>
  Real operator()(const ShapeFunctionT&, const NodesT& nodes) const
  {
      return ShapeFunctionT::volume(nodes);
  }
};

// Lazy evaluation
template<typename ShapeFunctionT, typename NodesT>
typename proto::result_of::make_expr<
    proto::tag::function
  , volume_fun          // First child is the functor wrapping the function
  , ShapeFunctionT const & // First function arg
  , NodesT const &           // Second function arg
>::type const
volume(ShapeFunctionT const& sf, NodesT const& nodes)
{
    return proto::make_expr<proto::tag::function>(
        volume_fun()
      , boost::ref(sf)
      , boost::ref(nodes)
    );
}

// Wrap the volume function
struct jacobian_determinant_fun
{
  typedef Real result_type;
  template<typename ShapeFunctionT, typename NodesT>
  Real operator()(const ShapeFunctionT&, const RealVector& mapped_coord, const NodesT& nodes) const
  {
      return ShapeFunctionT::jacobian_determinant(mapped_coord, nodes);
  }
};

// Lazy evaluation
template<typename ShapeFunctionT, typename MappedCoordT, typename NodesT>
typename proto::result_of::make_expr<
    proto::tag::function
  , jacobian_determinant_fun
  , ShapeFunctionT const &
  , MappedCoordT const &
  , NodesT const &
>::type const
jacobian_determinant(ShapeFunctionT const& sf, MappedCoordT const& mapped_coord, NodesT const& nodes)
{
    return proto::make_expr<proto::tag::function>(
        jacobian_determinant_fun()
      , boost::ref(sf)
      , boost::ref(mapped_coord)
      , boost::ref(nodes)
    );
}

template<typename ShapeFunctionT>
struct MeshContext;

struct integral_tag;

template<typename Arg>
typename proto::result_of::make_expr<
    integral_tag
  , Arg const &
>::type const
integral(Arg const &arg)
{
  return proto::make_expr<integral_tag>(boost::ref(arg));
}

///////////////////////////////////////////////////////////
// Placeholders

// Wrap std::cout
proto::terminal< std::ostream & >::type cout_ = { std::cout };

// Placeholders for different types of data
struct ElementIdxHolder {};
proto::terminal<ElementIdxHolder>::type const elem_i = {{}}; // Represents an element index

struct ConstNodeViewHolder {};
proto::terminal<ConstNodeViewHolder>::type const const_node_v = {{}}; // Represents a const view of the element nodes

struct MappedCoordHolder {};
proto::terminal<MappedCoordHolder>::type const mapped_c = {{}};

// placeholders for shape functions
struct SFHolder {};
proto::terminal<SFHolder>::type const shape_f = {{}};

///////////////////////////////////////////////////////////
// Context

// The calculator_context from the "Hello Calculator" section,
// implemented from scratch.
template<typename ShapeFunctionT>
struct MeshContext
{
  MeshContext(CArray& coords, CTable& conn_table) :
    coordinates(coords),
    connectivity(conn_table),
    element_idx(0)
  {
  }
  
  typedef ShapeFunctionT SF;
  typedef MeshContext<ShapeFunctionT> ThisContextT;
  
  /// Reference for the context
  CArray& coordinates;
  CTable& connectivity;
  Uint element_idx;
  RealVector mapped_coords;
  
  template<typename Expr,
           typename Tag = typename Expr::proto_tag,
           typename Arg = typename Expr::proto_child0>
  struct eval
    : proto::default_eval<Expr, ThisContextT>
  {};
  
  // Handle placeholder terminals here...
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, ConstNodeViewHolder>
  {
    typedef const ConstElementNodeView result_type;

    result_type operator()(Expr &, MeshContext<ShapeFunctionT>& ctx) const
    {
      return ConstElementNodeView(ctx.coordinates, ctx.connectivity[ctx.element_idx]);
    }
  };
  
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, ElementIdxHolder>
  {
    typedef Uint result_type;

    result_type operator()(Expr &, MeshContext<ShapeFunctionT>& ctx) const
    {
      return ctx.element_idx;
    }
  };
  
  // Handle shape function placeholders
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, SFHolder >
  {
    typedef const ShapeFunctionT& result_type;

    result_type operator()(Expr &, MeshContext<ShapeFunctionT>&) const
    {
      static ShapeFunctionT sf;
      return sf;
    }
  };
  
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, MappedCoordHolder>
  {
    typedef const RealVector& result_type;

    result_type operator()(Expr &, MeshContext<ShapeFunctionT>& ctx) const
    {
      return ctx.mapped_coords;
    }
  };
  
  // Handle integration
  template<typename Expr, typename ChildExpr>
  struct eval<Expr, integral_tag, ChildExpr >
  {
    typedef typename boost::remove_const<typename boost::remove_reference<ChildExpr>::type>::type RealChildT;
    typedef typename proto::result_of::eval<RealChildT, ThisContextT>::type result_type;

    result_type operator()(Expr& expr, MeshContext<ShapeFunctionT>& context) const
    {
      result_type r;
      Integrators::integrate<1>(proto::child(expr), r, context);
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
    
    MeshContext<ShapeFunctionT> context(m_elements.coordinates(), m_elements.connectivity_table());
    
    for(context.element_idx = 0; context.element_idx != context.connectivity.size(); ++context.element_idx)
    {
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

/// Applies the given expression to all elements using the supplied evaluator
template<typename ExpressionT, typename ExpressionEvaluatorT>
void for_all_elements(CMesh& mesh, ExpressionT& expr, ExpressionEvaluatorT& evaluator)
{
  BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(mesh))
  {
    boost::mpl::for_each<SF::VolumeTypes>(ElementLooper<ExpressionT, ExpressionEvaluatorT>(expr, evaluator, elements));
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


struct ProtoOperatorsFixture : Tools::Testing::TimedTestFixture
{
  static CMesh::Ptr big_grid;
};

CMesh::Ptr ProtoOperatorsFixture::big_grid;

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  CMesh::Ptr mesh(new CMesh("rect"));
  Tools::MeshGeneration::create_rectangle(*mesh, 5, 5, 5, 5);
  
  // Use the volume function
  for_all_elements_execute(*mesh, cout_ << "Volume for element " << elem_i << ": " << volume(shape_f, const_node_v) << "\n");
  std::cout << std::endl; // Can't be in expression
  
  // volume calculation
  Real vol1 = 0.;
  for_all_elements_accumulate(*mesh, volume(shape_f, const_node_v), vol1);
  
  CFinfo << "Mesh volume: " << vol1 << CFendl;
  
  // For an all-quad mesh, this is the same... cool or what?
  Real vol2 = 0.;
  for_all_elements_accumulate(*mesh,
                     0.5*((const_node_v[2][XX] - const_node_v[0][XX]) * (const_node_v[3][YY] - const_node_v[1][YY])
                       -  (const_node_v[2][YY] - const_node_v[0][YY]) * (const_node_v[3][XX] - const_node_v[1][XX])),
                              vol2);
  BOOST_CHECK_EQUAL(vol1, vol2);
  
  for_all_elements_execute(*mesh, cout_ << "test integral for element " << elem_i << ": " << integral(jacobian_determinant(shape_f, mapped_c, const_node_v)) <<  "\n");
  std::cout << std::endl;
}

// Must be run  before the next tests
BOOST_FIXTURE_TEST_CASE( CreateMesh, ProtoOperatorsFixture )
{
  ProtoOperatorsFixture::big_grid.reset(new CMesh("big_grid"));
  Tools::MeshGeneration::create_rectangle(*big_grid, 1., 1., 1000, 1000);
}

BOOST_FIXTURE_TEST_CASE( VolumeTiming, ProtoOperatorsFixture )
{
  Real vol = 0.;
  for_all_elements_accumulate(*big_grid, volume(shape_f, const_node_v), vol);
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

BOOST_FIXTURE_TEST_CASE( IntegralTiming, ProtoOperatorsFixture )
{
  Real vol = 0.;
  for_all_elements_accumulate(*big_grid, integral(jacobian_determinant(shape_f, mapped_c, const_node_v)), vol);
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
