// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Proposition for new element / shapefunction API"

#include <boost/test/unit_test.hpp>
#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Math/MatrixTypes.hpp"

using namespace CF;
using namespace CF::Common;


////////////////////////////////////////////////////////////////////////////////

/// ShapeFunction API
class ShapeFunction : public Common::Component
{
public:
  ShapeFunction(const std::string& name) : Component(name) {};

  virtual RealRowVector value(const RealVector& local_coord) const = 0;
};

////////////////////////////////////////////////////////////////////////////////

/// ShapeFunction intermediary class to bridge dynamic/static implementations
template <typename SFType>
class ShapeFunctionT : public ShapeFunction
{
public:
  ShapeFunctionT(const std::string& name) : ShapeFunction(name) {};

  virtual RealRowVector value(const RealVector& local_coord) const
  {
    return SFType::value( local_coord );
  }
};

////////////////////////////////////////////////////////////////////////////////

/// Concrete ShapeFunction implemented with only static functions
class LineLagrangeP1 : public ShapeFunctionT<LineLagrangeP1>
{
public:
  typedef Eigen::Matrix<Real, 1, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, 2> ValueT;

public:
  LineLagrangeP1(const std::string& name = "LineLagrangeP1") : ShapeFunctionT<LineLagrangeP1>(name) {};

  static ValueT value(const MappedCoordsT& local_coord)
  {
    return ( ValueT() << -1., 1. ).finished();
  }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// Element API
class Element : public Component
{
public:
  Element(const std::string& name) : Component(name) {};

  virtual RealMatrix jacobian(const RealVector& mapped_coordinates, const RealMatrix& nodes) = 0;

  virtual void compute_jacobian(RealMatrix& jacobian, const RealVector& mapped_coordinates, const RealMatrix& nodes) = 0;

  ShapeFunction& shape_function() { return *m_shape_function; }

protected:
  boost::shared_ptr<ShapeFunction> m_shape_function;
};

////////////////////////////////////////////////////////////////////////////////

/// Element intermediary class to bridge dynamic/static implementations
template <typename ET>
class ElementT : public Element
{
public:
  ElementT(const std::string& name) : Element(name)
  {
    m_shape_function = boost::shared_ptr<ShapeFunction>(new typename ET::SF);
  };

  virtual RealMatrix jacobian(const RealVector& mapped_coordinates, const RealMatrix& nodes)
  {
    return ET::jacobian(mapped_coordinates, nodes);
  }

  virtual void compute_jacobian(RealMatrix& jacobian, const RealVector& mapped_coordinates, const RealMatrix& nodes)
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ET::JacobianT& J( const_cast<RealMatrix const&>(jacobian));
    ET::compute_jacobian(const_cast<typename ET::JacobianT&>(J),mapped_coordinates,nodes);
  }
};

////////////////////////////////////////////////////////////////////////////////

/// Concrete Element implemented with only static functions
class Line1DLagrangeP1 : public ElementT<Line1DLagrangeP1>
{
public:
  typedef LineLagrangeP1 SF;
  typedef SF::MappedCoordsT MappedCoordsT;
  typedef Eigen::Matrix<Real, 2, 1> NodesT;
  typedef Eigen::Matrix<Real, 2, 2> JacobianT;

public:

  Line1DLagrangeP1(const std::string& name = "Line1DLagrangeP1") : ElementT<Line1DLagrangeP1>(name) {};

  static JacobianT jacobian(const MappedCoordsT& mapped_coords, const NodesT& nodes)
  {
    return (JacobianT() << 1., 1., 1., 1.).finished();
  }

  static void compute_jacobian(JacobianT& jacobian, const MappedCoordsT& mapped_coords, const NodesT& nodes)
  {
    jacobian << 1.,1.,1.,1.;
  }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct Test_ShapeFunction_Fixture
{
  /// common setup for each test case
  Test_ShapeFunction_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Test_ShapeFunction_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Test_ShapeFunction_TestSuite, Test_ShapeFunction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( static_version )
{

  Line1DLagrangeP1::MappedCoordsT local_coords = (Line1DLagrangeP1::MappedCoordsT() << 0 ).finished();
  Line1DLagrangeP1::NodesT        nodes        = (Line1DLagrangeP1::NodesT() << 0 , 1 ).finished();

  Line1DLagrangeP1::JacobianT     jacobian      = Line1DLagrangeP1::jacobian( local_coords , nodes );
  Line1DLagrangeP1::SF::ValueT    values        = Line1DLagrangeP1::SF::value( local_coords );

  std::cout << "static : jacobian = " << jacobian << std::endl;
  std::cout << "static : values   = " << values   << std::endl;

  Line1DLagrangeP1::compute_jacobian(jacobian,local_coords,nodes);
  std::cout << "static : jacobian = " << jacobian << std::endl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dynamic_version )
{

  std::auto_ptr<Element> element (new Line1DLagrangeP1);

  RealVector local_coords = (RealVector(1) << 0).finished();
  RealMatrix nodes        = (RealMatrix(2,1) << 0 , 1).finished();

  RealMatrix jacobian = element->jacobian(local_coords,nodes);
  RealRowVector values = element->shape_function().value(local_coords);

  std::cout << "dynamic: jacobian = " << jacobian << std::endl;
  std::cout << "dynamic: values   = " << values   << std::endl;

  element->compute_jacobian(jacobian,local_coords,nodes);
  std::cout << "dynamic: jacobian = " << jacobian << std::endl;

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

