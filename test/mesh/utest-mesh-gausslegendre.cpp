// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh interpolation"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"

#include "mesh/Quadrature.hpp"
#include "mesh/gausslegendre/Line.hpp"
#include "mesh/LagrangeP1/Line1D.hpp"
#include "mesh/LagrangeP2/Line1D.hpp"
#include "mesh/LagrangeP1/Line2D.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/ShapeFunction.hpp"

using namespace boost;
using namespace boost::assign;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct Fixture
{
  /// common setup for each test case
  Fixture()
  {
     int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Fixture()
  {
  }

  /// possibly common functions used on the tests below
  template <Uint P>
  double integrate(double (*f)(double), double a, double b)
  {
  	double c1 = (b - a) / 2, c2 = (b + a) / 2, sum = 0;
  	for (int i = 0; i < P; i++)
  		sum += gausslegendre::Line<P>::weights()[i] * f(c1 * gausslegendre::Line<P>::local_coordinates()[i] + c2);
  	return c1 * sum;
  }
 
  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestSuite, Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Line_static )
{  
  CFinfo << 1 << "  roots   " << gausslegendre::Line<1>::local_coordinates().transpose() << CFendl;
  CFinfo << 1 << "  weights " << gausslegendre::Line<1>::weights() << CFendl;
  
  CFinfo << 2 << "  roots   " << gausslegendre::Line<2>::local_coordinates().transpose() << CFendl;
  CFinfo << 2 << "  weights " << gausslegendre::Line<2>::weights() << CFendl;

  CFinfo << 3 << "  roots   " << gausslegendre::Line<3>::local_coordinates().transpose() << CFendl;
  CFinfo << 3 << "  weights " << gausslegendre::Line<3>::weights() << CFendl;

  CFinfo << 4 << "  roots   " << gausslegendre::Line<4>::local_coordinates().transpose() << CFendl;
  CFinfo << 4 << "  weights " << gausslegendre::Line<4>::weights() << CFendl;

  CFinfo << 5 << "  roots   " << gausslegendre::Line<5>::local_coordinates().transpose() << CFendl;
  CFinfo << 5 << "  weights " << gausslegendre::Line<5>::weights() << CFendl;

  CFinfo << 8 << "  roots   " << gausslegendre::Line<8>::local_coordinates().transpose() << CFendl;
  CFinfo << 8 << "  weights " << gausslegendre::Line<8>::weights() << CFendl;

  CFinfo << 10 << " roots   " << gausslegendre::Line<10>::local_coordinates().transpose() << CFendl;
  CFinfo << 10 << " weights " << gausslegendre::Line<10>::weights() << CFendl;

  CFinfo << integrate<1 >(std::exp, 0.,1.) << CFendl;
  CFinfo << integrate<2 >(std::exp, 0.,1.) << CFendl;
  CFinfo << integrate<3 >(std::exp, 0.,1.) << CFendl;
  CFinfo << integrate<4 >(std::exp, 0.,1.) << CFendl;
  CFinfo << integrate<5 >(std::exp, 0.,1.) << CFendl;
  CFinfo << integrate<8 >(std::exp, 0.,1.) << CFendl;
  CFinfo << integrate<10>(std::exp, 0.,1.) << CFendl;
}


BOOST_AUTO_TEST_CASE( Line_dynamic )
{
  Handle<mesh::Quadrature> lineP1 = Core::instance().root().create_component<mesh::Quadrature>("lineP1","cf3.mesh.gausslegendre.LineP1");
  CFinfo << 1 << "  roots   " << lineP1->local_coordinates().transpose() << CFendl;
  CFinfo << 1 << "  weights " << lineP1->weights() << CFendl;

  Handle<mesh::Quadrature> lineP2 = Core::instance().root().create_component<mesh::Quadrature>("lineP2","cf3.mesh.gausslegendre.LineP2");  
  CFinfo << 2 << "  roots   " << lineP2->local_coordinates().transpose() << CFendl;
  CFinfo << 2 << "  weights " << lineP2->weights() << CFendl;
 
  Handle<mesh::Quadrature> lineP3 = Core::instance().root().create_component<mesh::Quadrature>("lineP3","cf3.mesh.gausslegendre.LineP3"); 
  CFinfo << 3 << "  roots   " << lineP3->local_coordinates().transpose() << CFendl;
  CFinfo << 3 << "  weights " << lineP3->weights() << CFendl;
  
  Handle<mesh::Quadrature> lineP4 = Core::instance().root().create_component<mesh::Quadrature>("lineP4","cf3.mesh.gausslegendre.LineP4");
  CFinfo << 4 << "  roots   " << lineP4->local_coordinates().transpose() << CFendl;
  CFinfo << 4 << "  weights " << lineP4->weights() << CFendl;
  
  Handle<mesh::Quadrature> lineP5 = Core::instance().root().create_component<mesh::Quadrature>("lineP5","cf3.mesh.gausslegendre.LineP5");
  CFinfo << 5 << "  roots   " << lineP5->local_coordinates().transpose() << CFendl;
  CFinfo << 5 << "  weights " << lineP5->weights() << CFendl;
}

BOOST_AUTO_TEST_CASE( LagrangeP2_Line_integral_static )
{
  // A third order polynomial function will be integrated exactly by a gauss legendre quadrature of order 2.
  // Exact integral for polynomial order 2*p-1
  typedef LagrangeP2::Line1D ETYPE;
  typedef gausslegendre::Line<2> QDR;
  Real a=0, b=4;
  ETYPE::NodesT                             elem_coords;  elem_coords <<  a,  b,  0.5*(a+b) ;
  Eigen::Matrix<Real, ETYPE::nb_nodes, 1>   func;

  // Set function to x^2
  for (Uint n=0; n<ETYPE::nb_nodes; ++n)
  {
    Real x = elem_coords[n];
    func[n] = std::pow(x,2.);
  }

  Real integral(0.);
  for( Uint n=0; n<QDR::nb_nodes; ++n)
  {
    ETYPE::SF::ValueT interpolate = ETYPE::SF::value( QDR::local_coordinates().row(n) );
    Real Jdet = ETYPE::jacobian_determinant( QDR::local_coordinates().row(n), elem_coords );
    integral += Jdet * QDR::weights()[n] * interpolate * func;
  }
  BOOST_CHECK_CLOSE(integral, std::pow(b,3.)/3.,1e-10);  // integral(x^2) = x^3/3
}

BOOST_AUTO_TEST_CASE( LagrangeP2_Line_integral_dynamic )
{
  // A third order polynomial function will be integrated exactly by a gauss legendre quadrature of order 2.
  // Exact integral for polynomial order 2*p-1
  Handle<mesh::ElementType> etype = Core::instance().root().create_component<mesh::ElementType>("etype","cf3.mesh.LagrangeP2.Line2D");
  Handle<mesh::Quadrature > qdr   = Core::instance().root().create_component<mesh::Quadrature >("qdr",  "cf3.mesh.gausslegendre.LineP2");

  Real a=0, b=4;
  Real ax=a*std::sqrt(2.)/2., ay=a*std::sqrt(2.)/2.;
  Real bx=b*std::sqrt(2.)/2., by=b*std::sqrt(2.)/2.;
  RealMatrix elem_coords(etype->nb_nodes(),etype->dimension());
  RealVector func(etype->nb_nodes());

  elem_coords <<
      ax,          ay,
      bx,          by,
      0.5*(ax+bx), 0.5*(ay+by) ;

  // Set function to x^2
  for (Uint n=0; n<etype->nb_nodes(); ++n)
  {
    Real x = elem_coords(n,XX);
    Real y = elem_coords(n,YY);
    Real r = std::sqrt(x*x+y*y);
    func[n] = std::pow(r,2.);
  }

  Real integral(0.);
  for( Uint n=0; n<qdr->nb_nodes(); ++n)
  {
    RealRowVector interpolate = etype->shape_function().value( qdr->local_coordinates().row(n) );
    Real Jdet = etype->jacobian_determinant( qdr->local_coordinates().row(n), elem_coords );
    integral += Jdet * qdr->weights()[n] * interpolate * func;
  }
  BOOST_CHECK_CLOSE(integral, std::pow(b,3.)/3.,1e-10);  // integral(x^2) = x^3/3
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

