// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Common tests for shape functions that can be used to model the volume of a mesh"

#include <boost/mpl/for_each.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>


#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Math/RealVector.hpp"
#include "Math/MatrixInverterT.hpp"

#include "Mesh/GeoShape.hpp"
#include "Mesh/ElementNodes.hpp"

#include "Mesh/SF/Types.hpp"
#include "Mesh/Tetra3D.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Mesh::SF;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct VolumeSFFixture
{
  typedef std::vector<RealVector> NodesT;
  typedef std::map<GeoShape::Type, NodesT> NodesMapT;
  
  /// common setup for each test case
  VolumeSFFixture()
  {
    nodes[GeoShape::LINE]  = list_of(point1(5.))(point1(10.));
    nodes[GeoShape::TRIAG] = list_of(point2(0.5, 0.3))(point2(1.1, 1.2))(point2(0.8, 2.1));
    nodes[GeoShape::QUAD]  = list_of(point2(0.5, 0.3))(point2(1.1, 1.2))(point2(1.35, 1.9))(point2(0.8, 2.1));
    nodes[GeoShape::TETRA] = list_of(point3(0.830434, 0.885201, 0.188108))(point3(0.89653, 0.899961, 0.297475))(point3(0.888273, 0.821744, 0.211428))(point3(0.950439, 0.904872, 0.20736));
    nodes[GeoShape::HEXA]  = list_of(point3(0.5, 0.5, 0.5))(point3(1., 0., 0.))(point3(1.,1.,0.))(point3(0., 1., 0.))(point3(0., 0., 1.))(point3(1., 0., 1.))(point3(1.5, 1.5, 1.5))(point3(0., 1., 1.));
  }

  // Store test nodes per shape type
  NodesMapT nodes;
  
  /// Applies a functor if the element is a volume element
  template<typename FunctorT>
  struct VolumeMPLFunctor
  {
    VolumeMPLFunctor(const NodesMapT& nodes) : m_nodes(nodes) {}
    
    template<typename ShapeFunctionT> void operator()(const ShapeFunctionT& T)
    {
      FunctorT functor;
      cf_assert(ShapeFunctionT::dimension == ShapeFunctionT::dimensionality);
      CFinfo << "---------------------- Start " << T.getElementTypeName() << " test ----------------------" << CFendl;
      const Uint segments = 5; // number of segments in each direction for the mapped coord calculation
      const Real step = ((ShapeFunctionT::shape == GeoShape::TRIAG || ShapeFunctionT::shape == GeoShape::TETRA) ? 1. : 2.) / static_cast<Real>(segments);
      const Real mapped_coord_min = (ShapeFunctionT::shape == GeoShape::TRIAG || ShapeFunctionT::shape == GeoShape::TETRA) ? 0. : -1.;
      const NodesMapT::const_iterator shape_nodes_it = m_nodes.find(ShapeFunctionT::shape);
      cf_assert(shape_nodes_it != m_nodes.end());
      const NodesT& nodes = shape_nodes_it->second;
      switch(ShapeFunctionT::dimension)
      {
        case DIM_1D:
          for(Uint i = 0; i <= segments; ++i)
          {
            const RealVector mapped_coord = list_of(mapped_coord_min + i * step);
            functor(T, nodes, mapped_coord);
          }
          break;
        case DIM_2D:
          for(Uint i = 0; i <= segments; ++i)
          {
            for(Uint j = 0; j <= segments; ++j)
            {
              const RealVector mapped_coord = list_of(mapped_coord_min + i * step)(mapped_coord_min + j * step);
              functor(T, nodes, mapped_coord);
            }
          }
          break;
        case DIM_3D:
          for(Uint i = 0; i <= segments; ++i)
          {
            for(Uint j = 0; j <= segments; ++j)
            {
              for(Uint k = 0; k <= segments; ++k)
              {
                const RealVector mapped_coord = list_of(mapped_coord_min + i * step)(mapped_coord_min + j * step)(mapped_coord_min + k * step);
                functor(T, nodes, mapped_coord);
              }
            }
          }
          break;
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
template<typename ShapeFunctionT, typename NodesT>
RealVector gradient(const NodesT& nodes, const RealVector& mapped_coordinates, const RealVector& function_values)
{
  // Get the gradient in mapped coordinates
  RealMatrix mapped_grad(ShapeFunctionT::dimensionality,ShapeFunctionT::nb_nodes);
  ShapeFunctionT::mapped_gradient(mapped_coordinates,mapped_grad);
  
  // The Jacobian adjugate
  RealMatrix jacobian_adj(ShapeFunctionT::dimension, ShapeFunctionT::dimensionality);
  ShapeFunctionT::jacobian_adjoint(mapped_coordinates, nodes, jacobian_adj);
  
  // The gradient operator matrix in the absolute frame
  RealMatrix grad(ShapeFunctionT::dimension,ShapeFunctionT::nb_nodes);
  grad = (jacobian_adj * mapped_grad) / ShapeFunctionT::jacobian_determinant(mapped_coordinates, nodes);
  
  // Apply the gradient to the function values
  RealVector result(ShapeFunctionT::dimension);
  result = grad * function_values;
  return result;
}

/// Checks if the jacobian_determinant function result is the same as det(jacobian)
struct CheckJacobianDeterminant
{
  template<typename ShapeFunctionT, typename NodesT>
  void operator()(const ShapeFunctionT& T, const NodesT& nodes, const RealVector& mapped_coord)
  {
    RealMatrix jacobian(ShapeFunctionT::dimensionality, ShapeFunctionT::dimension);
    ShapeFunctionT::jacobian(mapped_coord, nodes, jacobian);
    switch(ShapeFunctionT::dimension)
    {
      case DIM_1D:
        BOOST_CHECK_CLOSE(jacobian(0,0), ShapeFunctionT::jacobian_determinant(mapped_coord, nodes), 1e-6);
        break;
      case DIM_2D:
        BOOST_CHECK_CLOSE(jacobian.determ2(), ShapeFunctionT::jacobian_determinant(mapped_coord, nodes), 1e-6);
        break;
      case DIM_3D:
        BOOST_CHECK_CLOSE(jacobian.determ3(), ShapeFunctionT::jacobian_determinant(mapped_coord, nodes), 1e-6);
        break;
    }
  }
};

/// Checks if the inverse of the jacobian matrix equals jacobian_adjoint / jacobian_determinant
struct CheckJacobianInverse
{
  template<typename ShapeFunctionT, typename NodesT>
  void operator()(const ShapeFunctionT& T, const NodesT& nodes, const RealVector& mapped_coord)
  {
    RealMatrix jacobian(ShapeFunctionT::dimensionality, ShapeFunctionT::dimension);
    ShapeFunctionT::jacobian(mapped_coord, nodes, jacobian);
    
    RealMatrix jacobian_adjoint(ShapeFunctionT::dimension, ShapeFunctionT::dimensionality);
    ShapeFunctionT::jacobian_adjoint(mapped_coord, nodes, jacobian_adjoint);
    
    RealMatrix identity(ShapeFunctionT::dimensionality, ShapeFunctionT::dimension);
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
  template<typename ShapeFunctionT, typename NodesT>
  void operator()(const ShapeFunctionT& T, const NodesT& nodes, const RealVector& mapped_coord)
  {
    // Define the function values
    RealVector function(ShapeFunctionT::nb_nodes);
    for(Uint i = 0; i != ShapeFunctionT::nb_nodes; ++i)
      function[i] = nodes[i][XX];
    
    // Calculate the gradient
    RealVector grad = gradient<ShapeFunctionT>(nodes, mapped_coord, function);
    
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
  boost::mpl::for_each<VolumeTypes>(functor);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestJacobianInverse )
{
  VolumeMPLFunctor<CheckJacobianInverse> functor(nodes);
  boost::mpl::for_each<VolumeTypes>(functor); 
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestGradientX )
{
  VolumeMPLFunctor<CheckGradientX> functor(nodes);
  boost::mpl::for_each<VolumeTypes>(functor); 
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

