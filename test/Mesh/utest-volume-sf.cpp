// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Common tests for shape functions that can be used to model the volume of a mesh"

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/fusion/container/map.hpp>
#include <boost/fusion/container/map/convert.hpp>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/GeoShape.hpp"

#include "Mesh/SF/Types.hpp"
#include "Mesh/Tetra3D.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Mesh::SF;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

template<int Dim>
struct FunctorForDim;

template<>
struct FunctorForDim<1>
{
  template<typename ShapeFunctionT, typename NodesT, typename FunctorT>
  void operator()(const Uint segments, const ShapeFunctionT& sf, const NodesT& nodes, FunctorT& functor)
  {
    const Real step = ((ShapeFunctionT::shape == GeoShape::TRIAG || ShapeFunctionT::shape == GeoShape::TETRA) ? 1. : 2.) / static_cast<Real>(segments);
    const Real mapped_coord_min = (ShapeFunctionT::shape == GeoShape::TRIAG || ShapeFunctionT::shape == GeoShape::TETRA) ? 0. : -1.; 
    for(Uint i = 0; i <= segments; ++i)
    {
      typename ShapeFunctionT::MappedCoordsT mapped_coord;
      mapped_coord << mapped_coord_min + i * step;
      functor(sf, nodes, mapped_coord);
    }
  }
};

template<>
struct FunctorForDim<2>
{
  template<typename ShapeFunctionT, typename NodesT, typename FunctorT>
  void operator()(const Uint segments, const ShapeFunctionT& sf, const NodesT& nodes, FunctorT& functor)
  {
    const Real step = ((ShapeFunctionT::shape == GeoShape::TRIAG || ShapeFunctionT::shape == GeoShape::TETRA) ? 1. : 2.) / static_cast<Real>(segments);
    const Real mapped_coord_min = (ShapeFunctionT::shape == GeoShape::TRIAG || ShapeFunctionT::shape == GeoShape::TETRA) ? 0. : -1.; 
    for(Uint i = 0; i <= segments; ++i)
    {
      for(Uint j = 0; j <= segments; ++j)
      {
        typename ShapeFunctionT::MappedCoordsT mapped_coord;
        mapped_coord << mapped_coord_min + i * step, mapped_coord_min + j * step;
        functor(sf, nodes, mapped_coord);
      }
    }
  }
};

template<>
struct FunctorForDim<3>
{
  template<typename ShapeFunctionT, typename NodesT, typename FunctorT>
  void operator()(const Uint segments, const ShapeFunctionT& sf, const NodesT& nodes, FunctorT& functor)
  {
    const Real step = ((ShapeFunctionT::shape == GeoShape::TRIAG || ShapeFunctionT::shape == GeoShape::TETRA) ? 1. : 2.) / static_cast<Real>(segments);
    const Real mapped_coord_min = (ShapeFunctionT::shape == GeoShape::TRIAG || ShapeFunctionT::shape == GeoShape::TETRA) ? 0. : -1.; 
    for(Uint i = 0; i <= segments; ++i)
    {
      for(Uint j = 0; j <= segments; ++j)
      {
        for(Uint k = 0; k <= segments; ++k)
        {
          typename ShapeFunctionT::MappedCoordsT mapped_coord;
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
    typedef boost::fusion::pair<SF, typename SF::NodeMatrixT> type;
  };

  /// Map between a shape function and its node matrix. Used to store nodes for each possible test
  typedef boost::fusion::result_of::as_map
  <
    boost::mpl::transform_view
    <
      CellTypes,
      MakeSFNodesPair<boost::mpl::_1>
    >
  >::type NodesMapT;

  /// common setup for each test case
  VolumeSFFixture() :
    // If you get a compile error here, you forrgot to add nodes for a new volume shape function type
    nodes
    (
      boost::fusion::make_pair<Line1DLagrangeP1>( (Line1DLagrangeP1::NodeMatrixT() << 
        5.,
        10.
      ).finished() ),
      boost::fusion::make_pair<Triag2DLagrangeP1>( (Triag2DLagrangeP1::NodeMatrixT() << 
        0.5, 0.3,
        1.1, 1.2,
        0.8, 2.1
      ).finished() ),
      boost::fusion::make_pair<Triag2DLagrangeP2>( (Triag2DLagrangeP2::NodeMatrixT() <<
        0.5, 0.3,
        1.1, 1.2,
        0.8, 2.1,
        0.9, 0.5,
        1.0, 1.5,
        0.75,1.6
      ).finished() ),
      boost::fusion::make_pair<Triag2DLagrangeP3>( (Triag2DLagrangeP3::NodeMatrixT() <<
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
      ).finished() ),
      boost::fusion::make_pair<Quad2DLagrangeP1>( (Quad2DLagrangeP1::NodeMatrixT() <<
        0.5, 0.3,
        1.1, 1.2,
        1.35, 1.9,
        0.8, 2.1
      ).finished() ),
     boost::fusion::make_pair<Quad2DLagrangeP2>( (Quad2DLagrangeP2::NodeMatrixT() <<
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
      boost::fusion::make_pair<Hexa3DLagrangeP1>( (Hexa3DLagrangeP1::NodeMatrixT() <<
        0.5, 0.5, 0.5,
        1., 0., 0.,
        1.,1.,0.,
        0., 1., 0.,
        0., 0., 1.,
        1., 0., 1.,
        1.5, 1.5, 1.5,
        0., 1., 1.
      ).finished() ),
      boost::fusion::make_pair<Tetra3DLagrangeP1>( (Tetra3DLagrangeP1::NodeMatrixT() <<
        0.830434, 0.885201, 0.188108,
        0.89653, 0.899961, 0.297475,
        0.888273, 0.821744, 0.211428,
        0.950439, 0.904872, 0.20736
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

    template<typename ShapeFunctionT> void operator()(const ShapeFunctionT& T)
    {
      FunctorT functor;
      cf_assert(ShapeFunctionT::dimension == ShapeFunctionT::dimensionality);
      CFinfo << "---------------------- Start " << T.element_type_name() << " test ----------------------" << CFendl;
      const Uint segments = 5; // number of segments in each direction for the mapped coord calculation
      try
      {
        FunctorForDim<ShapeFunctionT::dimension>()(segments, T, boost::fusion::at_key<ShapeFunctionT>(m_nodes), functor);
      }
      catch(...)
      {
        CFinfo << "  Unimplemented method for " << T.element_type_name() << CFendl;
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
template<typename ShapeFunctionT, typename NodesT, typename MappedCoordsT>
typename ShapeFunctionT::CoordsT gradient(const NodesT& nodes, const MappedCoordsT& mapped_coordinates, const RealVector& function_values)
{
  // Get the gradient in mapped coordinates
  typename ShapeFunctionT::MappedGradientT mapped_grad;
  ShapeFunctionT::mapped_gradient(mapped_coordinates,mapped_grad);

  // The Jacobian adjugate
  typename ShapeFunctionT::JacobianT jacobian_adj;
  ShapeFunctionT::jacobian_adjoint(mapped_coordinates, nodes, jacobian_adj);

  return ((jacobian_adj * mapped_grad) / ShapeFunctionT::jacobian_determinant(mapped_coordinates, nodes)) * function_values;
}

/// Checks if the jacobian_determinant function result is the same as det(jacobian)
struct CheckJacobianDeterminant
{
  template<typename ShapeFunctionT, typename NodesT, typename CoordsT>
  void operator()(const ShapeFunctionT& T, const NodesT& nodes, const CoordsT& mapped_coord)
  {
    typename ShapeFunctionT::JacobianT jacobian;
    ShapeFunctionT::jacobian(mapped_coord, nodes, jacobian);
    BOOST_CHECK_CLOSE(jacobian.determinant(), ShapeFunctionT::jacobian_determinant(mapped_coord, nodes), 1e-6);
  }
};

/// Checks if the inverse of the jacobian matrix equals jacobian_adjoint / jacobian_determinant
struct CheckJacobianInverse
{
  template<typename ShapeFunctionT, typename NodesT, typename CoordsT>
  void operator()(const ShapeFunctionT& T, const NodesT& nodes, const CoordsT& mapped_coord)
  {
    typename ShapeFunctionT::JacobianT jacobian;
    ShapeFunctionT::jacobian(mapped_coord, nodes, jacobian);

    typename ShapeFunctionT::JacobianT jacobian_adjoint;
    ShapeFunctionT::jacobian_adjoint(mapped_coord, nodes, jacobian_adjoint);

    typename ShapeFunctionT::JacobianT identity;
    identity = jacobian * jacobian_adjoint / ShapeFunctionT::jacobian_determinant(mapped_coord, nodes);

    for(Uint i = 0; i != ShapeFunctionT::dimensionality; ++i)
    {
      for(Uint j = 0; j != ShapeFunctionT::dimension; ++j)
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
  template<typename ShapeFunctionT, typename NodesT, typename CoordsT>
  void operator()(const ShapeFunctionT& T, const NodesT& nodes, const CoordsT& mapped_coord)
  {
    // Define the function values
    RealVector function(ShapeFunctionT::nb_nodes);
    for(Uint i = 0; i != ShapeFunctionT::nb_nodes; ++i)
      function[i] = nodes(i, XX);

    // Calculate the gradient
    typename ShapeFunctionT::CoordsT grad = gradient<ShapeFunctionT>(nodes, mapped_coord, function);

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
  VolumeMPLFunctor<CheckJacobianDeterminant> functor(nodes);
  boost::mpl::for_each<CellTypes>(functor);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestJacobianInverse )
{
  VolumeMPLFunctor<CheckJacobianInverse> functor(nodes);
  boost::mpl::for_each<CellTypes>(functor);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestGradientX )
{
  VolumeMPLFunctor<CheckGradientX> functor(nodes);
  boost::mpl::for_each<CellTypes>(functor);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

