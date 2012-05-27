// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Common tests for shape functions that can be used to model the volume of a mesh"

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/fusion/container/map.hpp>
#include <boost/fusion/container/map/convert.hpp>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/FindComponents.hpp"

#include "mesh/GeoShape.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/ElementTypes.hpp"

using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

typedef boost::mpl::vector<
LagrangeP1::Line1D,
LagrangeP1::Quad2D,
LagrangeP1::Triag2D,
LagrangeP1::Hexa3D,
LagrangeP1::Tetra3D,
LagrangeP2::Quad2D,
LagrangeP2::Triag2D,
LagrangeP3::Quad2D,
LagrangeP3::Triag2D
> TestTypes;

typedef boost::mpl::filter_view<TestTypes, IsCellType> TestCellTypes;
typedef boost::mpl::filter_view<TestTypes, IsFaceType> TestFaceTypes;
typedef boost::mpl::filter_view<TestTypes, IsEdgeType> TestEdgeTypes;


////////////////////////////////////////////////////////////////////////////////

template<int Dim>
struct FunctorForDim;

template<>
struct FunctorForDim<1>
{
  template<typename ETYPE, typename NodesT, typename FunctorT>
  void operator()(const Uint segments, const ETYPE& sf, const NodesT& nodes, FunctorT& functor)
  {
    const Real step = ((ETYPE::shape == GeoShape::TRIAG || ETYPE::shape == GeoShape::TETRA) ? 1. : 2.) / static_cast<Real>(segments);
    const Real mapped_coord_min = (ETYPE::shape == GeoShape::TRIAG || ETYPE::shape == GeoShape::TETRA) ? 0. : -1.;
    for(Uint i = 0; i <= segments; ++i)
    {
      typename ETYPE::MappedCoordsT mapped_coord;
      mapped_coord << mapped_coord_min + i * step;
      functor(sf, nodes, mapped_coord);
    }
  }
};

template<>
struct FunctorForDim<2>
{
  template<typename ETYPE, typename NodesT, typename FunctorT>
  void operator()(const Uint segments, const ETYPE& sf, const NodesT& nodes, FunctorT& functor)
  {
    const Real step = ((ETYPE::shape == GeoShape::TRIAG || ETYPE::shape == GeoShape::TETRA) ? 1. : 2.) / static_cast<Real>(segments);
    const Real mapped_coord_min = (ETYPE::shape == GeoShape::TRIAG || ETYPE::shape == GeoShape::TETRA) ? 0. : -1.;
    for(Uint i = 0; i <= segments; ++i)
    {
      for(Uint j = 0; j <= segments; ++j)
      {
        typename ETYPE::MappedCoordsT mapped_coord;
        mapped_coord << mapped_coord_min + i * step, mapped_coord_min + j * step;
        functor(sf, nodes, mapped_coord);
      }
    }
  }
};

template<>
struct FunctorForDim<3>
{
  template<typename ETYPE, typename NodesT, typename FunctorT>
  void operator()(const Uint segments, const ETYPE& sf, const NodesT& nodes, FunctorT& functor)
  {
    const Real step = ((ETYPE::shape == GeoShape::TRIAG || ETYPE::shape == GeoShape::TETRA) ? 1. : 2.) / static_cast<Real>(segments);
    const Real mapped_coord_min = (ETYPE::shape == GeoShape::TRIAG || ETYPE::shape == GeoShape::TETRA) ? 0. : -1.;
    for(Uint i = 0; i <= segments; ++i)
    {
      for(Uint j = 0; j <= segments; ++j)
      {
        for(Uint k = 0; k <= segments; ++k)
        {
          typename ETYPE::MappedCoordsT mapped_coord;
          mapped_coord << mapped_coord_min + i * step, mapped_coord_min + j * step, mapped_coord_min + k * step;
          functor(sf, nodes, mapped_coord);
        }
      }
    }
  }
};

struct VolumeSFFixture
{
  /// Functor to create a fusion pair between a shape function and its node matrix
  template<typename SF>
  struct MakeSFNodesPair
  {
    typedef boost::fusion::pair<SF, typename SF::NodesT> type;
  };

  /// Map between a shape function and its node matrix. Used to store nodes for each possible test
  typedef boost::fusion::result_of::as_map
  <
    boost::mpl::transform_view
    <
      TestTypes,
      MakeSFNodesPair<boost::mpl::_1>
    >
  >::type NodesMapT;

  /// common setup for each test case
  VolumeSFFixture() :
    // If you get a compile error here, you forrgot to add nodes for a new volume shape function type
    nodes
    (
      boost::fusion::make_pair< LagrangeP1::Line1D>( ( LagrangeP1::Line1D::NodesT() <<
        5.,
        10.
      ).finished() ),
      boost::fusion::make_pair< LagrangeP1::Quad2D>( ( LagrangeP1::Quad2D::NodesT() <<
        0.5, 0.3,
        1.1, 1.2,
        1.35, 1.9,
        0.8, 2.1
      ).finished() ),
      boost::fusion::make_pair< LagrangeP1::Triag2D>( ( LagrangeP1::Triag2D::NodesT() <<
        0.5, 0.3,
        1.1, 1.2,
        0.8, 2.1
      ).finished() ),
      boost::fusion::make_pair< LagrangeP1::Hexa3D>( ( LagrangeP1::Hexa3D::NodesT() <<
        0.5, 0.5, 0.5,
        1., 0., 0.,
        1.,1.,0.,
        0., 1., 0.,
        0., 0., 1.,
        1., 0., 1.,
        1.5, 1.5, 1.5,
        0., 1., 1.
      ).finished() ),
      boost::fusion::make_pair< LagrangeP1::Tetra3D>( ( LagrangeP1::Tetra3D::NodesT() <<
        0.830434, 0.885201, 0.188108,
        0.89653, 0.899961, 0.297475,
        0.888273, 0.821744, 0.211428,
        0.950439, 0.904872, 0.20736
      ).finished() ),
      boost::fusion::make_pair< LagrangeP2::Quad2D>( ( LagrangeP2::Quad2D::NodesT() <<
         0.5, 0.3,
         1.1, 1.2,
         1.35, 1.9,
         0.8, 2.1,
         0.9, 0.5,
         1.2, 1.3,
         1.0,2.0,
         0.75,1.6,
         1.0,1.0
       ).finished() ),
      boost::fusion::make_pair< LagrangeP2::Triag2D>( ( LagrangeP2::Triag2D::NodesT() <<
        0.5, 0.3,
        1.1, 1.2,
        0.8, 2.1,
        0.9, 0.5,
        1.0, 1.5,
        0.75,1.6
      ).finished() ),
      boost::fusion::make_pair< LagrangeP3::Quad2D>( ( LagrangeP3::Quad2D::NodesT() <<
                                                       -1.,     -1.,
                                                        1.,     -1.,
                                                        1.,      1.,
                                                       -1.,      1.,
                                                       -1./3.,  -1.,
                                                        1./3.,  -1.,
                                                        1.,     -1./3.,
                                                        1.,      1./3.,
                                                        1./3.,   1.,
                                                       -1./3.,   1.,
                                                       -1.,      1./3.,
                                                       -1.,     -1./3.,
                                                       -1./3.,  -1./3.,
                                                        1./3.,  -1./3.,
                                                        1./3.,   1./3.,
                                                       -1./3.,   1./3.
      ).finished() ),
      boost::fusion::make_pair< LagrangeP3::Triag2D>( ( LagrangeP3::Triag2D::NodesT() <<
        0.5, 0.3,
        1.1, 1.2,
        0.8, 2.1,
        0.9, 0.5,
        1.0, 1.5,
        1.0, 1.5,
        1.0, 1.5,
        1.0, 1.5,
        1.0, 1.5,
        0.75,1.6
      ).finished() )
    ) // end nodes(...) construction
  {
  }

  // Store test nodes per shape type
  const NodesMapT nodes;

  /// Applies a functor if the element is a volume element
  template<typename FunctorT>
  struct VolumeMPLFunctor
  {
    VolumeMPLFunctor(const NodesMapT& nodes) : m_nodes(nodes) {}

    template<typename ETYPE> void operator()(const ETYPE& T)
    {
      FunctorT functor;
      cf3_assert(ETYPE::dimension == ETYPE::dimensionality);
      CFinfo << "---------------------- Start " << allocate_component< ElementTypeT<ETYPE> >("lala")->derived_type_name() << " test ----------------------" << CFendl;
      const Uint segments = 5; // number of segments in each direction for the mapped coord calculation
      try
      {
        FunctorForDim<ETYPE::dimension>()(segments, T, boost::fusion::at_key<ETYPE>(m_nodes), functor);
      }
      catch(...)
      {
        CFinfo << "  Unimplemented method for " << T.type_name() << CFendl;
      }
    }

  private:
    const NodesMapT& m_nodes;
  };
};

/// Calculates the gradient using the given shape function
/// @param nodes The node coordinates for the element
/// @param mapped_coordinates Location where the gradient is to be calculated
/// @param function_values Nodal values of the function for which the gradient will be calculated
template<typename ETYPE, typename NodesT, typename MappedCoordsT>
typename ETYPE::CoordsT gradient(const NodesT& nodes, const MappedCoordsT& mapped_coordinates, const RealVector& function_values)
{
  // Get the gradient in mapped coordinates
  typename ETYPE::SF::GradientT mapped_grad;
  ETYPE::SF::compute_gradient(mapped_coordinates,mapped_grad);

  // The Jacobian adjugate
  typename ETYPE::JacobianT jacobian_adj;
  ETYPE::compute_jacobian_adjoint(mapped_coordinates, nodes, jacobian_adj);

  return ((jacobian_adj * mapped_grad) / ETYPE::jacobian_determinant(mapped_coordinates, nodes)) * function_values;
}

/// Checks if the jacobian_determinant function result is the same as det(jacobian)
struct CheckJacobianDeterminant
{
  template<typename ETYPE, typename NodesT, typename CoordsT>
  void operator()(const ETYPE& T, const NodesT& nodes, const CoordsT& mapped_coord)
  {
    typename ETYPE::JacobianT jacobian;
    ETYPE::compute_jacobian(mapped_coord, nodes, jacobian);
    BOOST_CHECK_CLOSE(jacobian.determinant(), ETYPE::jacobian_determinant(mapped_coord, nodes), 1e-6);
  }
};

/// Checks if the inverse of the jacobian matrix equals jacobian_adjoint / jacobian_determinant
struct CheckJacobianInverse
{
  template<typename ETYPE, typename NodesT, typename CoordsT>
  void operator()(const ETYPE& T, const NodesT& nodes, const CoordsT& mapped_coord)
  {
    typename ETYPE::JacobianT jacobian;
    ETYPE::compute_jacobian(mapped_coord, nodes, jacobian);

    typename ETYPE::JacobianT jacobian_adjoint;
    ETYPE::compute_jacobian_adjoint(mapped_coord, nodes, jacobian_adjoint);

    typename ETYPE::JacobianT identity;
    identity = jacobian * jacobian_adjoint / ETYPE::jacobian_determinant(mapped_coord, nodes);

    for(Uint i = 0; i != ETYPE::dimensionality; ++i)
    {
      for(Uint j = 0; j != ETYPE::dimension; ++j)
      {
        if(j == i)
          BOOST_CHECK_CLOSE(identity(i, j), 1., 1e-6);
        else
          BOOST_CHECK_SMALL(identity(i, j), 1e-14);
      }
    }
  }
};

/// Check if the gradient of X is one in the X direction and zero in the other directions
struct CheckGradientX
{
  template<typename ETYPE, typename NodesT, typename CoordsT>
  void operator()(const ETYPE& T, const NodesT& nodes, const CoordsT& mapped_coord)
  {
    // Define the function values
    RealVector function(ETYPE::nb_nodes);
    for(Uint i = 0; i != ETYPE::nb_nodes; ++i)
      function[i] = nodes(i, XX);

    // Calculate the gradient
    typename ETYPE::CoordsT grad = gradient<ETYPE>(nodes, mapped_coord, function);

    // Check the result
    BOOST_CHECK_CLOSE(grad[0], 1, 1e-6);
    for(Uint i = 1; i != grad.size(); ++i)
      BOOST_CHECK_SMALL(grad[i], 1e-14);
  }
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( VolumeSFSuite, VolumeSFFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestJacobianDeterminant )
{
  Core::instance().environment().options().set("exception_outputs",false);
  Core::instance().environment().options().set("exception_backtrace",false);
  VolumeMPLFunctor<CheckJacobianDeterminant> functor(nodes);
  boost::mpl::for_each<TestCellTypes>(functor);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestJacobianInverse )
{
  VolumeMPLFunctor<CheckJacobianInverse> functor(nodes);
  boost::mpl::for_each<TestCellTypes>(functor);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestGradientX )
{
  VolumeMPLFunctor<CheckGradientX> functor(nodes);
  boost::mpl::for_each<TestCellTypes>(functor);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

