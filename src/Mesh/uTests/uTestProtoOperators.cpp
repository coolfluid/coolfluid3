// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::CField"

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

#include "Mesh/SF/Types.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include <boost/foreach.hpp>
#include <boost/proto/proto.hpp>
#include <boost/test/unit_test.hpp>

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace boost;

// Experimental boost::proto stuff. Scroll down to the actual test to see the use
namespace CF
{

namespace Mesh
{

enum ArgumentTypes { NODE, ELEMENTNODEVIEW };

//////////////////////////////////////////////////////////
// functors

// Wrap the volume function
struct volume_fun
{
  typedef Real result_type;
  template<typename ShapeFunctionT>
  Real operator()(const ShapeFunctionT&, const ConstElementNodeView& nodes) const
  {
      return ShapeFunctionT::volume(nodes);
  }
};

// Lazy evaluation
template<typename ShapeFunctionT, typename Arg>
typename proto::result_of::make_expr<
    proto::tag::function  // Tag type
  , volume_fun          // First child (by value)
  , ShapeFunctionT const &
  , Arg const &           // Second child (by reference)
>::type const
volume(ShapeFunctionT const& sf, Arg const& arg)
{
    return proto::make_expr<proto::tag::function>(
        volume_fun()
      , boost::ref(sf)
      , boost::ref(arg)
    );
}

///////////////////////////////////////////////////////////
// Placeholders

// Define a placeholder type
template<ArgumentTypes I>
struct Placeholder
{};

// Placeholders for different types of data
proto::terminal<Placeholder<NODE> >::type const aNode = {{}}; // Represents a node
proto::terminal<Placeholder<ELEMENTNODEVIEW> >::type const anElementNodeView = {{}}; // Represents a view of the element nodes

// placeholders for shape functions
struct SFHolder
{};

proto::terminal<SFHolder>::type const aShapeFunction = {{}};

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
  
  /// Reference for the context
  CArray& coordinates;
  CTable& connectivity;
  Uint element_idx;
  
//   template<
//       typename Expr
//       // defaulted template parameters, so we can
//       // specialize on the expressions that need
//       // special handling.
//     , typename Tag = typename proto::tag_of<Expr>::type
//     , typename Arg0 = typename proto::result_of::child_c<Expr, 0>::type
//   >
//   struct eval;

  template<typename Expr,
           typename Tag = typename proto::tag_of<Expr>::type,
           typename Arg = typename proto::result_of::child<Expr>::type>
  struct eval
    : proto::default_eval<Expr, MeshContext<ShapeFunctionT> const, Tag>
  {};

  // Handle placeholder terminals here...
  template<typename Expr, ArgumentTypes I>
  struct eval<Expr, proto::tag::terminal, Placeholder<I> >;
  
  // Handle placeholder terminals here...
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, Placeholder<ELEMENTNODEVIEW> >
  {
    typedef const ConstElementNodeView result_type;

    result_type operator()(Expr &, const MeshContext<ShapeFunctionT>& ctx) const
    {
      return ConstElementNodeView(ctx.coordinates, ctx.connectivity[ctx.element_idx]);
    }
  };
  
  // Handle shape function placeholders
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, SFHolder >
  {
    typedef const ShapeFunctionT& result_type;

    result_type operator()(Expr &, const MeshContext<ShapeFunctionT>&) const
    {
      static ShapeFunctionT sf;
      return sf;
    }
  };
};

/// Looper defines a functor taking the type that boost::mpl::for_each
/// passes. It is the core of the looping mechanism.
template<typename ExpressionT>
struct Looper
{
public: // functions

  /// Constructor
  Looper(const ExpressionT& expr, CElements& elements) : m_expr(expr), m_elements(elements) {}

  /// Operator
  template < typename ShapeFunctionT >
  void operator() ( ShapeFunctionT& T )
  {
    if(!IsElementType<ShapeFunctionT>()(m_elements.element_type()))
      return;
    
    MeshContext<ShapeFunctionT> context(m_elements.coordinates(), m_elements.connectivity_table());
    
    for(context.element_idx = 0; context.element_idx != context.connectivity.size(); ++context.element_idx)
    {
      std::cout << proto::eval(m_expr, context) << std::endl;
    }
    
  }
private:
  const ExpressionT& m_expr;
  CElements& m_elements;
};

template<typename ExpressionT>
void print_for_all_elements(CMesh& mesh, const ExpressionT& expr)
{
  BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(mesh))
  {
    boost::mpl::for_each<SF::VolumeTypes>(Looper<ExpressionT>(expr, elements));
  }
}
  
} // namespace Mesh

} // Namespace CF

using namespace CF;
using namespace CF::Mesh;

struct ProtoOperatorsFixture
{
};

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( Volume, ProtoOperatorsFixture )
{
  CMesh::Ptr mesh(new CMesh("rect"));
  Tools::MeshGeneration::create_rectangle(*mesh, 5, 5, 5, 5);
  
  // Use the volume function
  print_for_all_elements(*mesh, volume(aShapeFunction, anElementNodeView));
  
  // For an all-quad mesh, this is the same... cool or what?
  print_for_all_elements(*mesh,
                     0.5*((anElementNodeView[2][XX] - anElementNodeView[0][XX]) * (anElementNodeView[3][YY] - anElementNodeView[1][YY])
                       -  (anElementNodeView[2][YY] - anElementNodeView[0][YY]) * (anElementNodeView[3][XX] - anElementNodeView[1][XX])));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
