// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//#include <boost/math/special_functions/bessel.hpp>

#include <boost/math/special_functions/bessel.hpp>

#include <complex>
#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "math/Consts.hpp"

#include "solver/Time.hpp"

#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"

#include "sdm/lineuler/InitCorotatingVortexPair.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitCorotatingVortexPair, common::Action, sdm::lineuler::LibLinEuler> InitCorotatingVortexPair_Builder;

//////////////////////////////////////////////////////////////////////////////

InitCorotatingVortexPair::InitCorotatingVortexPair( const std::string& name )
  : common::Action(name)
{

  properties()["brief"] = std::string("Initialize a field with a constant value");
  std::string desc;
  desc = "  Usage: InitCorotatingVortexPair constant \n";
  properties()["description"] = desc;

  options().add("field", m_field)
      .description("Field to initialize")
      .pretty_name("Field")
      .link_to(&m_field)
      .mark_basic();

  options().add("mean_flow", m_mean_flow)
      .description("Mean flow field")
      .link_to(&m_mean_flow)
      .mark_basic();

  options().add("time", m_time)
      .link_to(&m_time)
      .description("Time component")
      .mark_basic();

  m_gamma = 1.4;
  options().add("gamma", m_gamma)
      .description("heat capacity ratio")
      .link_to(&m_gamma)
      .mark_basic();
  
  m_omega = 2.*math::Consts::pi()/30.;
  options().add("omega",m_omega)
      .description("Angular frequency")
      .link_to(&m_omega)
      .mark_basic();
    
  m_r0 = 1.0;
  options().add("r0",m_r0)
      .link_to(&m_r0)
      .mark_basic()
      .description("Large rotation radius");
}

////////////////////////////////////////////////////////////////////////////////

void InitCorotatingVortexPair::execute()
{
  using namespace std;
  using namespace math::Consts;
  using namespace boost::math;

  RealVector2 coord;
  Real t = m_time->current_time();

  cf3_assert(m_field);
  cf3_assert(m_field->coordinates().row_size()>=DIM_2D);
  cf3_assert(m_field->row_size()==4);
  
  Real G = m_omega*(4.*pi()*m_r0*m_r0);

    
  for (Uint i=0; i<m_field->size(); ++i)
  {
    cf3_assert(i<m_field->coordinates().size());
    coord[XX] = m_field->coordinates()[i][XX];
    coord[YY] = m_field->coordinates()[i][YY];

    Real r = coord.norm();
    if (std::abs(r) < eps()) r = 10*eps()*sign(r);
    Real theta = atan2(coord[YY],coord[XX]);

    Real rho0 = m_mean_flow->array()[i][0];
    Real p0 = m_mean_flow->array()[i][3];
    Real c0 = sqrt(m_gamma*p0/rho0);

//    complex<Real> z(coord[XX],coord[YY]);
//    complex<Real> b = r*std::exp(I*m_omega*t);
////    CFinfo << "z = " << z << CFendl;
//    CFinfo << "b = " << b << CFendl;
//    complex<Real> u_min_Iv = G/(I*pi())*z/(z*z-b*b);
//    CFinfo << "u_min_Iv = " << u_min_Iv << CFendl;
//    Real u = u_min_Iv.real();
//    Real v = - u_min_Iv.imag();
    Real u=0;
    Real v=0;

    Real k = 0.1592;
//    CFinfo << "r = " << r << CFendl;
    Real p;
    try
    {
      p = -rho0*pow(G,4)/(pow(4.*pi(),3)*pow(m_r0,4)*pow(c0,2))
             * (   cyl_bessel_j(2,k*r)*sin(2.*theta-2.*m_omega*t)
                 + cyl_neumann(2,k*r)*cos(2.*theta-2.*m_omega*t) );
    // cyl_bessel_j(2,..) = 2nd order bessel function of first kind
    // cyl_neumann(2,..)  = 2nd order bessel funciton of second kind
    }
    catch(...)
    {
      p = 0.;
    }

//    CFinfo << "p = " << p << CFendl;
    m_field->array()[i][0] = 1/(c0*c0) * p;
    m_field->array()[i][1] = rho0 * u;
    m_field->array()[i][2] = rho0 * v;
    m_field->array()[i][3] = p;
  }

}


//void InitCorotatingVortexPair::execute()
//{
//  using namespace math::Consts;
//  using namespace std;

//  RealVector2 coord;
//  Real t = m_time->current_time();

//  cf3_assert(m_field);
//  cf3_assert(m_field->coordinates().row_size()>=DIM_2D);
//  cf3_assert(m_field->row_size()==4);

//  Real G = m_omega*(4.*pi()*m_r0*m_r0);

//  std::complex<Real> I(0.,1.);

//  for (Uint i=0; i<m_field->size(); ++i)
//  {
//    cf3_assert(i<m_field->coordinates().size());
//    coord[XX] = m_field->coordinates()[i][XX];
//    coord[YY] = m_field->coordinates()[i][YY];

//    Real rho0 = m_mean_flow->array()[i][0];
//    Real p0 = m_mean_flow->array()[i][3];
//    Real c0 = sqrt(m_gamma*p0/rho0);

//    Real r = coord.norm();
//    complex<Real> z(coord[XX],coord[YY]);
//    complex<Real> b = r*std::exp(I*m_omega*t);
//    CFinfo << "z = " << z << CFendl;
//    CFinfo << "b = " << b << CFendl;
//    complex<Real> u_min_Iv = G/(I*pi())*z/(z*z-b*b);
//    CFinfo << "u_min_Iv = " << u_min_Iv << CFendl;
//    Real u = u_min_Iv.real();
//    Real v = - u_min_Iv.imag();
//    Real p = -rho0*G*m_omega/pi()*real(b*b/(z*z-b*b)) - 0.5*rho0*(u*u+v*v);
//    CFinfo << " p = " << p << CFendl;

//    m_field->array()[i][0] = 1/(c0*c0) * p;
//    m_field->array()[i][1] = rho0 * u;
//    m_field->array()[i][2] = rho0 * v;
//    m_field->array()[i][3] = p;
//  }

//}



//////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // physics
} // cf3
