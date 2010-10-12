// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/foreach.hpp>

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/sequence/intrinsic/value_at.hpp>

#include <boost/mpl/max.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/void.hpp>

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
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Mesh/SF/Types.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace boost;

// Experimental boost::proto stuff. Scroll down to the actual test to see the use
namespace CF
{

namespace Math
{
  template <typename T1, typename T2>
  std::ostream& operator<<(std::ostream& os, const CF::Math::Expr<T1, T2>& expr)
  {
    for(Uint i = 0; i != expr.size(); ++i)
      os << expr.at(i) << " ";
    return os;
}

}
  
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
template<class Expr>
struct MeshExpr;

struct MeshDomain
  : proto::domain<proto::generator<MeshExpr>, MeshGrammar>
{};

template<class Expr>
struct MeshExpr : proto::extends<Expr, MeshExpr<Expr>, MeshDomain>
{
  typedef proto::extends<Expr, MeshExpr<Expr>, MeshDomain> base_type;

  MeshExpr(Expr const &expr = Expr()) : base_type(expr) {}
  
  template<typename T>
  MeshExpr(const T& par1)
  {
    typedef typename fusion::result_of::value_at_c<Expr, 0>::type var_t;
    proto::value(*this) = var_t(par1);
  }
};

//////////////////////////////////////////////////////////
// custom functions

// Wrap the jacobian determinant function
struct jacobian_determinant_tag
{};

template<typename MappedCoordsT, typename NodesT>
typename proto::result_of::make_expr<
    jacobian_determinant_tag
  , MeshDomain
  , MappedCoordsT const &
  , NodesT const &
>::type const
jacobian_determinant(MappedCoordsT const& mapped_coords, NodesT const& nodes)
{
  return proto::make_expr<jacobian_determinant_tag, MeshDomain>(boost::ref(mapped_coords), boost::ref(nodes));
}

// Wrap the normal function
struct normal_tag
{};

template<typename MappedCoordsT, typename NodesT>
typename proto::result_of::make_expr<
    normal_tag
  , MeshDomain
  , MappedCoordsT const &
  , NodesT const &
>::type const
normal(MappedCoordsT const& mapped_coords, NodesT const& nodes)
{
  return proto::make_expr<normal_tag, MeshDomain>(boost::ref(mapped_coords), boost::ref(nodes));
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

struct coords_tag
{};

template<typename MappedCoordsT, typename NodesT>
typename proto::result_of::make_expr<
    coords_tag
  , MeshDomain
  , MappedCoordsT const &
  , NodesT const &
>::type const
coords(MappedCoordsT const& mapped_coords, NodesT const& nodes)
{
  return proto::make_expr<coords_tag, MeshDomain>(boost::ref(mapped_coords), boost::ref(nodes));
}

///////////////////////////////////////////////////////////
// Placeholders

// Wrap std::cout
proto::terminal< std::ostream & >::type _cout = {std::cout};

// Wrap some math functions
proto::terminal< double(*)(double) >::type const _sin = {&sin};
proto::terminal< double(*)(double, double) >::type const _atan2 = {&atan2};

// Pow function based on Proto docs example
// Define a pow_fun function object
template<Uint Exp>
struct pow_fun
{
  typedef Real result_type;
  Real operator()(Real d) const
  {
    for(Uint i = 0; i != (Exp-1); ++i)
      d *= d;
    return d;
  }
};

template<Uint Exp, typename Arg>
typename proto::result_of::make_expr<
    proto::tag::function  // Tag type
  , pow_fun<Exp>          // First child (by value)
  , Arg const &           // Second child (by reference)
>::type const
pow(Arg const &arg)
{
  return proto::make_expr<proto::tag::function>(
      pow_fun<Exp>()    // First child (by value)
    , boost::ref(arg)   // Second child (by reference)
  );
}

// Placeholders for different types of data
struct ElementIdxHolder {};
proto::terminal<ElementIdxHolder>::type const _elem = {{}}; // Represents an element index ({{{}}} makes this a static initialization)

struct MappedCoordHolder {};
proto::terminal<MappedCoordHolder>::type const _mapped_coord = {{}};

/// Creates a variable that has unique ID I
template<typename I, typename T>
struct Var : T
{
  /// Type that is wrapped
  typedef T type;
  Var() : T() {}
  
  /// Index of the var
  typedef I index_type;
  
  template<typename T1>
  Var(const T1& par1) : T(par1) {}
  
  /// Type of the terminal expression wrapping this variable
  typedef MeshExpr<typename proto::terminal<Var<I, T> >::type> expr_type;
};

/// Represent const element nodes. This will be replaced with the current list of element nodes
struct ConstNodes
{
};

std::ostream& operator<<(std::ostream& output, const ConstNodes&)
{
  output << "ConstNodes";
  return output;
}

/// Const node field data
template<typename T>
struct ConstNodesFd
{
  ConstNodesFd() : field_name() {}
  
  ConstNodesFd(const std::string& fld_name) : field_name(fld_name) {}
  
  std::string field_name;
};

template<typename T>
std::ostream& operator<<(std::ostream& output, const ConstNodesFd<T>& fd)
{
  output << "field " << fd.field_name;
  return output;
}

/// Represent mutable element nodes
struct MutableNodes
{
};

template<Uint I, typename T>
struct MeshTerm : Var<mpl::int_<I>, T>::expr_type
{
  typedef typename Var<mpl::int_<I>, T>::expr_type base_type;
  
  MeshTerm() : base_type() {}
  
  template<typename T1>
  MeshTerm(const T1& par1) : base_type(par1) {}
};

template<typename T>
struct VarArity
{
  typedef typename T::index_type type;
};

// Gets the arity (max index) for the numbered variables
struct ExprVarArity
  : proto::or_<
        proto::when< proto::terminal< Var<proto::_, proto::_> >,
          mpl::next< VarArity<proto::_value> >()
        >
      , proto::when< proto::terminal<proto::_>,
          mpl::int_<0>()
        >
      , proto::when<
            proto::nary_expr<proto::_, proto::vararg<proto::_> >
          , proto::fold<proto::_, mpl::int_<0>(), mpl::max<ExprVarArity, proto::_state>()>
        >
    >
{};

/// Transform that extracts the type of variable I
template<Uint I>
struct DefineType
  : proto::or_<
      proto::when< proto::terminal< Var<mpl::int_<I>, proto::_> >,
          proto::_value
      >
    , proto::when< proto::terminal< proto::_ >,
          mpl::void_()
      >
    , proto::when< proto::nary_expr<proto::_, proto::vararg<proto::_> >
            , proto::fold<proto::_, mpl::void_(), mpl::if_<mpl::is_void_<proto::_state>, DefineType<I>, proto::_state>() >
      >
  >
{};

/// Ease application of DefineType as an MPL lambda expression, and strip the Var encapsulation
template<typename I, typename Expr>
struct DefineTypeOp
{
  typedef typename boost::result_of<DefineType<I::value>(Expr)>::type var_type;
  typedef typename mpl::if_<mpl::is_void_<var_type>, mpl::void_, typename var_type::type>::type type;
};

/// Copy the terminal values to a fusion list
template<typename VarsT>
struct CopyNumberedVars
  : proto::callable_context< CopyNumberedVars<VarsT>, proto::null_context >
{
  typedef void result_type;
  
  CopyNumberedVars(VarsT& vars) : m_vars(vars) {}

  template<typename I, typename T>
  void operator()(proto::tag::terminal, Var<I, T> val)
  {
    fusion::at<I>(m_vars) = val;
  }
  
private:
  VarsT& m_vars;  
};

////////////////////////////////////////////////////
// debugging stuff
template <class T> struct error_printer {};
template <class T1, class T2> struct error_printer2 {};

struct print
{
  template <typename T>
  void operator()(T const& x) const;
};

template <typename T>
void print::operator()(T const& x) const
{
  std::cout << x << ", ";
}

template <>
void print::operator()(mpl::void_ const&) const
{
}

template<typename Expr>
void introspect(const Expr& E)
{
  // Determine the number of different variables
  typedef typename boost::result_of<ExprVarArity(Expr)>::type nb_vars;
  
  std::cout << "nb vars: " << nb_vars::value << std::endl;
  
  // init empty vector that will store variable indices
  typedef mpl::vector_c<Uint> numbers_empty;
  
  // Fill the vector with indices 0 to 9, so we allow 10 different (field or node related) variables in an expression
  typedef typename mpl::copy<
      mpl::range_c<int,0,nb_vars::value>
    , mpl::back_inserter< numbers_empty >
    >::type range;
  
  // Get the type for each variable that is used, or set to mpl::void_ for unused indices
  typedef typename mpl::transform<range, DefineTypeOp<mpl::_1, Expr > >::type expr_types;
  
  typedef typename fusion::result_of::as_vector<expr_types>::type FusionVarsT;
  //error_printer<FusionVarsT>().print(); // induce a compile error to see the type
  
  FusionVarsT vars;
  CopyNumberedVars<FusionVarsT> ctx(vars);
  proto::eval(E, ctx);
  fusion::for_each(vars, print());
  std::cout << std::endl;
}

///////////////////////////////////////////////////

/// Fall-back context for numbered variables, doing nothing
/// This gets used in case there is a gap in the variable numbers, i.e.
/// if pressure has index 0 and temperature index 8, and nothing in between
/// is used.
template<typename SF, typename VarT>
struct VarContext
{
  void init(const VarT&, const CElements& elements)
  {
  }
  
  void fill(const VarT&, const Uint element_idx)
  {
  }
};

/// Evaluate const nodes numbered variables
template<typename SF>
struct VarContext<SF, ConstNodes>
  : proto::callable_context< VarContext<SF, ConstNodes> const, proto::null_context const>
{
  typedef SF ShapeFunctionT;
  typedef ElementNodeValues<SF::nb_nodes, SF::dimension> NodesT;
  typedef const NodesT& result_type;
  
  void init(const ConstNodes&, const CElements& elements)
  {
    coordinates = &elements.coordinates();
    connectivity = &elements.connectivity_table();
  }
  
  void fill(const ConstNodes&, const Uint element_idx)
  {
    nodes.fill(*coordinates, (*connectivity)[element_idx]);
  }
  
  template<typename I>
  result_type operator()(proto::tag::terminal, const Var<I, ConstNodes>&) const
  {
    return nodes;
  }
  
  NodesT nodes;
  const CArray* coordinates;
  const CTable* connectivity;
};

/// Strip the CF expression templates
template<typename T1, typename T2=mpl::void_, typename T3=mpl::void_>
struct StripExpr
{
  typedef T1 type;
};

template<typename T1, typename T2>
struct StripExpr< Math::Mult<RealVector, T1, T2> >
{
  typedef RealVector type;
};

template<typename T1, typename T2>
struct StripExpr< Math::Mult<T1, RealVector, T2> >
{
  typedef RealVector type;
};

///////////////////////////////////////////////////////////
// Context

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
    : proto::default_eval<Expr, ThisContextT>
  {};
  
  /// Process numbered variables
  template<typename Expr, typename I, typename T>
  struct eval< Expr, proto::tag::terminal, Var<I, T> >
  {
    typedef typename fusion::result_of::value_at<ContextsT, I>::type::result_type result_type;

    result_type operator()(Expr& expr, ThisContextT& ctx)
    {
      return proto::eval(expr, fusion::at<I>(ctx.contexts));
    }
  };
  
  /// Placeholder that evaluates to the current element index
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, ElementIdxHolder>
  {
    typedef Uint result_type;

    result_type operator()(Expr &, const ThisContextT& ctx) const
    {
      return ctx.element_idx;
    }
  };
  
  /// Placeholder that evaluates to the current mapped coordinates
  template<typename Expr>
  struct eval<Expr, proto::tag::terminal, MappedCoordHolder>
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
      return SF::volume(proto::eval(proto::child(expr), ctx));
    }
  };
  
  /// jacobian determinant function
  template<typename Expr, typename MappedCoordsT>
  struct eval<Expr, jacobian_determinant_tag, MappedCoordsT >
  {
    typedef Real result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      return SF::jacobian_determinant(context.mapped_coords, proto::eval(proto::child_c<1>(expr), context));
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
      SF::normal(context.mapped_coords, proto::eval(proto::child_c<1>(expr), context), context.surface_normal);
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
        typename StripExpr
        <
          typename proto::result_of::eval
          <
            RealChildT, ThisContextT
          >::type
        >::type
      >::type
    >::type result_type;

    result_type operator()(Expr& expr, ThisContextT& context)
    {
      result_type r(2); // TODO: Auto-detect result size
      Integrators::integrate<I>(proto::child(expr), r, context);
      return r;
    }
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
      ::CF::Mesh::eval<SF>(context.mapped_coords, proto::eval(proto::child_c<1>(expr), context), context.real_coords);
      return context.real_coords;
    }
  };
};

struct ResultSize
{
  Uint nb_rows;
  Uint nb_cols;
  
  bool operator==(const ResultSize& other) const
  {
    return other.nb_rows == nb_rows && other.nb_cols == nb_cols;
  }
};

/// addition doesn't change size
const ResultSize operator+(const ResultSize& a, const ResultSize& b)
{
  cf_assert(a == b);
  return a;
}

/// Context used to evaluate the size of expressions
template<typename ShapeFunctionT>
struct MeshSizeContext
{
};

/// Wrap up a var type in its context
template<typename SF, typename VarT>
struct AddContext
{
  typedef VarContext<SF, VarT> type;
};

template<typename VarsT, typename ContextsT>
struct InitContexts
{
  InitContexts(const VarsT& vars, ContextsT& contexts, CElements& elements) : m_vars(vars), m_contexts(contexts), m_elements(elements) {}
  
  template<typename I>
  void operator()(const I&)
  {
    fusion::at<I>(m_contexts).init(fusion::at<I>(m_vars), m_elements);
  }
  
  const VarsT& m_vars;
  ContextsT& m_contexts;
  CElements& m_elements;
};

template<typename VarsT, typename ContextsT>
struct FillContexts
{
  FillContexts(const VarsT& vars, ContextsT& contexts) : m_vars(vars), m_contexts(contexts), elem_idx(0) {}
  
  template<typename I>
  void operator()(const I&)
  {
    fusion::at<I>(m_contexts).fill(fusion::at<I>(m_vars), elem_idx);
  }
  
  const VarsT& m_vars;
  ContextsT& m_contexts;
  Uint elem_idx;
};

/// Looper defines a functor taking the type that boost::mpl::for_each
/// passes. It is the core of the looping mechanism.
template<typename ExpressionT, typename VarTypesT, Uint nb_vars>
class ElementLooper
{
public: // functions

  /// Constructor
  ElementLooper(const ExpressionT& expr, const VarTypesT& vars, CElements& elements) : m_expr(expr), m_vars(vars), m_elements(elements) {}

  /// Operator
  template < typename ShapeFunctionT >
  void operator() ( ShapeFunctionT& T )
  {
    if(!IsElementType<ShapeFunctionT>()(m_elements.element_type()))
      return;
    
    // Inititalize variable-specific contexts
    typedef typename mpl::transform< VarTypesT, AddContext<ShapeFunctionT, mpl::_1> >::type ContextsT;
    ContextsT contexts;
    
    InitContexts<VarTypesT, ContextsT> init_ctx(m_vars, contexts, m_elements);
    boost::mpl::for_each<boost::mpl::range_c<int, 0, nb_vars> >(init_ctx);
    
    FillContexts<VarTypesT, ContextsT> fill_ctx(m_vars, contexts);
    
    // Create the global context
    MeshContext<ShapeFunctionT, ContextsT> context(contexts);
    
    const Uint nb_elems = m_elements.connectivity_table().size();
    for(context.element_idx = 0; context.element_idx != nb_elems; ++context.element_idx)
    {
      fill_ctx.elem_idx = context.element_idx;
      boost::mpl::for_each<boost::mpl::range_c<int, 0, nb_vars> >(fill_ctx);
      proto::eval(m_expr, context);
    }
    
  }
private:
  const ExpressionT& m_expr;
  const VarTypesT& m_vars;
  CElements& m_elements;
};

/// List of all supported shapefunctions that allow high order integration
typedef boost::mpl::vector< SF::Line1DLagrangeP1,
                            SF::Quad2DLagrangeP1,
                            SF::Hexa3DLagrangeP1
> HigherIntegrationElements;

template<typename ETypesT, typename Expr>
void for_each_element(CMesh& mesh, const Expr& expr)
{
  // Number of variables
  typedef typename boost::result_of<ExprVarArity(Expr)>::type nb_vars;
  
  // init empty vector that will store variable indices
  typedef mpl::vector_c<Uint> numbers_empty;
  
  // Fill the vector with indices 0 to 9, so we allow 10 different (field or node related) variables in an expression
  typedef typename mpl::copy<
      mpl::range_c<int,0,nb_vars::value>
    , mpl::back_inserter< numbers_empty >
    >::type range_nb_vars;
  
  // Get the type for each variable that is used, or set to mpl::void_ for unused indices
  typedef typename mpl::transform<range_nb_vars, DefineTypeOp<mpl::_1, Expr > >::type expr_types;
  
  typedef typename fusion::result_of::as_vector<expr_types>::type FusionVarsT;
  
  FusionVarsT vars;
  CopyNumberedVars<FusionVarsT> ctx(vars);
  proto::eval(expr, ctx);
  
  // Evaluate the expression
  BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(mesh))
  {
    boost::mpl::for_each<ETypesT>(ElementLooper<Expr, FusionVarsT, nb_vars::value>(expr, vars, elements));
  }
}

} // namespace Mesh

} // Namespace CF


struct ProtoOperatorsFixture : public Tools::Testing::ProfiledTestFixture, Tools::Testing::TimedTestFixture
{
  static CMesh::Ptr grid_2d;
  static CMesh::Ptr channel_3d;
};

CMesh::Ptr ProtoOperatorsFixture::grid_2d;
CMesh::Ptr ProtoOperatorsFixture::channel_3d;

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

//////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( ProtoBasics )
{
  CMesh::Ptr mesh(new CMesh("rect"));
  Tools::MeshGeneration::create_rectangle(*mesh, 5, 5, 5, 5);
  
  // Create the variables
  MeshTerm<0, ConstNodes> nodes;
  MeshTerm<1, ConstNodesFd<Real> > temperature("Temp");
  MeshTerm<2, ConstNodesFd<RealMatrix> > mat("SomeMatrix");
  
  // Output the different numbered variable names for a nonsensical but somewhat elaborate expression
  introspect(mat * (nodes + temperature) + nodes*mat);
  
  // Use the volume function
  for_each_element<SF::VolumeTypes>(*mesh, _cout << "Element " << _elem << ": volume = " << volume(nodes) << ", centroid = " << coords(_mapped_coord, nodes) << "\n");
  std::cout << std::endl; // Can't be in expression
  
  // volume calculation
  Real vol1 = 0.;
  for_each_element<SF::VolumeTypes>(*mesh, vol1 += volume(nodes));
  
  CFinfo << "Mesh volume: " << vol1 << CFendl;

  // For an all-quad mesh, this is the same... cool or what?, also this doesn't work anymore
//   Real vol2 = 0.;
//   for_each_element<mpl::vector<SF::Quad2DLagrangeP1> >(*mesh, vol2 +=
//                    0.5*((nodes[2][XX] - nodes[0][XX]) * (nodes[3][YY] - nodes[1][YY])
//                      -  (nodes[2][YY] - nodes[0][YY]) * (nodes[3][XX] - nodes[1][XX])));
//   BOOST_CHECK_CLOSE(vol1, vol2, 1e-5);
}

BOOST_AUTO_TEST_CASE( RotatingCylinder )
{
  const Real radius = 1.;
  const Uint segments = 1000;
  const Real u = 300.;
  const Real circulation = 975.;
  const Real rho = 1.225;
  
  CMesh::Ptr mesh(new CMesh("circle"));
  Tools::MeshGeneration::create_circle_2d(*mesh, radius, segments);
  
  MeshTerm<0, ConstNodes> nodes;
  
  typedef boost::mpl::vector< SF::Line2DLagrangeP1> SurfaceTypes;
  
  RealVector force(0., 2);
  for_each_element<SurfaceTypes>(*mesh,
    force += integral<1>
      (
        pow<2>
        (
          2. * u * _sin(_atan2(coords(_mapped_coord, nodes)[YY], coords(_mapped_coord, nodes)[XX])) + circulation / (2. * Math::MathConsts::RealPi() * radius)
        )  * 0.5 * rho * normal(_mapped_coord, nodes) 
      )
  );

  BOOST_CHECK_CLOSE(force[YY], rho*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
}


// Must be run  before the next tests
BOOST_FIXTURE_TEST_CASE( CreateMesh2D, ProtoOperatorsFixture )
{
  ProtoOperatorsFixture::grid_2d.reset(new CMesh("grid_2d"));
  Tools::MeshGeneration::create_rectangle(*grid_2d, 1., 1., 2000, 2000);
}


/// Non-proto calculation, as reference
BOOST_FIXTURE_TEST_CASE( VolumeDirect2D, ProtoOperatorsFixture ) // timed and profiled
{
  Real volume = 0.0;
  BOOST_FOREACH(const CElements& region, recursive_range_typed<CElements>(*grid_2d))
  {
    const CArray& coords = region.coordinates();
    const CTable::ArrayT& ctbl = region.connectivity_table().array();
    const Uint element_count = ctbl.size();
    ElementNodeValues<4, 2, 1> nodes;
    for(Uint element = 0; element != element_count; ++element)
    {
      nodes.fill(coords, ctbl[element]);
      volume += (nodes[2][XX] - nodes[0][XX]) * (nodes[3][YY] - nodes[1][YY]) -
          (nodes[2][YY] - nodes[0][YY]) * (nodes[3][XX] - nodes[1][XX]);
    }
  }
  volume *= 0.5;
  BOOST_CHECK_CLOSE(volume, 1., 1e-8);
}

// Compute volume
BOOST_FIXTURE_TEST_CASE( Volume2D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<SF::VolumeTypes>(*grid_2d, vol += volume(nodes));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Compute volume, using an unnecessarily high var index
BOOST_FIXTURE_TEST_CASE( VolumeVector10, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<9, ConstNodes> nodes; // setting this to 9 increases the overhead
  for_each_element<SF::VolumeTypes>(*grid_2d, vol += volume(nodes));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Compute volume through integration
BOOST_FIXTURE_TEST_CASE( Integral2D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<SF::VolumeTypes>(*grid_2d, vol += integral<1>(jacobian_determinant(_mapped_coord, nodes)));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Compute volume through 4th order integration
BOOST_FIXTURE_TEST_CASE( IntegralOrder4, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<HigherIntegrationElements>(*grid_2d, vol += integral<4>(jacobian_determinant(_mapped_coord, nodes)));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

BOOST_FIXTURE_TEST_CASE( CreateMesh3D, ProtoOperatorsFixture )
{
  channel_3d.reset(new CMesh("channel_3d"));
  BlockMesh::BlockData block_data;
  Tools::MeshGeneration::create_channel_3d(block_data, 10., 0.5, 5., 160, 80, 120, 0.1);
  std::vector<Uint> nodes_dist;
  BlockMesh::build_mesh(block_data, *channel_3d, nodes_dist);
}

// Compute volume
BOOST_FIXTURE_TEST_CASE( Volume3D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<SF::VolumeTypes>(*channel_3d, vol += volume(nodes));
  BOOST_CHECK_CLOSE(vol, 50., 1e-6);
}

// Compute volume through integration
BOOST_FIXTURE_TEST_CASE( Integral3D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<SF::VolumeTypes>(*channel_3d, vol += integral<1>(jacobian_determinant(_mapped_coord, nodes)));
  BOOST_CHECK_CLOSE(vol, 50., 1e-6);
}

/// Non-proto calculation, as reference
BOOST_FIXTURE_TEST_CASE( VolumeDirect3D, ProtoOperatorsFixture ) // timed and profiled
{
  const CElements& elems = recursive_get_named_component_typed<CElements>(*channel_3d, "elements_Hexa3DLagrangeP1");
  const CArray& coords = elems.coordinates();
  const CTable::ArrayT conn = elems.connectivity_table().array();
  const Uint nb_elems = conn.size();
  Real volume = 0.0;
  ElementNodeValues<8, 3> nodes;
  for(Uint elem = 0; elem != nb_elems; ++elem)
  {
    nodes.fill(coords, conn[elem]);
    volume += SF::Hexa3DLagrangeP1::volume(nodes);
  }
  BOOST_CHECK_CLOSE(volume, 50., 1e-6);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
