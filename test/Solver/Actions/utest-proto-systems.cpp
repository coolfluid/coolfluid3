// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/mpl/max_element.hpp>

#include "Solver/Actions/Proto/CProtoElementsAction.hpp"
#include "Solver/Actions/Proto/CProtoNodesAction.hpp"
#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"

#include "Solver/CEigenLSS.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;

using namespace boost;

template<typename I>
struct DefineTypeExpr
{
  template<typename ExprT>
  struct apply
  {
    typedef typename boost::result_of<DefineType<I::value>(ExprT)>::type var_type;
    typedef typename boost::mpl::if_<boost::mpl::is_void_<var_type>, boost::mpl::void_, typename var_type::type>::type type;
  };
};

template<typename SystemT>
struct DefineTypeSystem
{
  template<typename I>
  struct apply
  {
    /// type for every expression in the system
    typedef typename boost::mpl::transform< SystemT,  DefineTypeExpr<I> >::type var_types;
    
    /// find non-void values, if any
    typedef typename boost::mpl::find_if
    <
      var_types,
      boost::mpl::is_not_void_<boost::mpl::_1>
    >::type iter;
    
    /// if no non-void values were found, return void, otherwise return the found type
    typedef typename boost::mpl::if_
    <
      boost::is_same< boost::mpl::end<var_types>, iter >,
      boost::mpl::void_,
      typename boost::mpl::deref<iter>::type
    >::type type;
  };
};

/// Turns a single expression into a system containing only one expression, to make a uniform treatment of systems and single equations possible
template<typename ExprT>
struct ExprSystemType
{
  typedef typename boost::mpl::if_
  <
    boost::proto::is_expr<ExprT>,
    boost::fusion::vector1<ExprT>,
    ExprT
  >::type type;
};

/// Extracts number of variables and their types from the given equation or system of equations
template<typename ExprT>
struct ExpressionProperties
{
  /// Always use a system of equations, even if there is only one
  typedef typename ExprSystemType<ExprT>::type SystemT;
  
  typedef typename boost::mpl::deref
  <
    typename boost::mpl::max_element
    <
      typename boost::fusion::result_of::transform
      <
        SystemT, ExprVarArity
      >::type
    >::type
  >::type NbVarsT;
  
  /// Types of the used variables
  typedef typename boost::fusion::result_of::as_vector
  <
    typename boost::mpl::transform
    <
      typename boost::mpl::copy<boost::mpl::range_c<int,0,NbVarsT::value>, boost::mpl::back_inserter< boost::mpl::vector_c<Uint> > >::type, //range from 0 to NbVarsT
      DefineTypeSystem<SystemT>
    >::type
  >::type VariablesT;
};

BOOST_AUTO_TEST_SUITE( ProtoSystemsSuite )

//Test using a fusion vector of proto expressions
BOOST_AUTO_TEST_CASE( SystemBasics )
{
  Real length              = 5.;
  const Uint nb_segments   = 5;

  // build the mesh
  CMesh::Ptr mesh(allocate_component<CMesh>("line"));
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  
  // Geometric suport
  MeshTerm<0, ConstNodes> nodes( "ConductivityRegion", find_component_ptr_recursively_with_name<CRegion>(*mesh, "cells") );
  
  Uint elem_idx = 0;
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    group
    (
      _cout << "Volume for element " << elem_idx << ": " << volume(nodes) << "\n",
      ++boost::proto::lit(elem_idx)
    )
  );
}

BOOST_AUTO_TEST_SUITE_END()
