// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//#include <boost/math/special_functions/bessel.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Space.hpp"

#include "SFDM/lineuler/InitAcousticVorticityPulse.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace lineuler {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitAcousticVorticityPulse, common::Action, SFDM::lineuler::LibLinEuler> InitAcousticVorticityPulse_Builder;

//////////////////////////////////////////////////////////////////////////////

InitAcousticVorticityPulse::InitAcousticVorticityPulse( const std::string& name )
  : common::Action(name)
{

  properties()["brief"] = std::string("Initialize a field with a constant value");
  std::string desc;
  desc = "  Usage: InitAcousticVorticityPulse constant \n";
  properties()["description"] = desc;

  options().add_option("field", m_field)
      .description("Field to initialize")
      .pretty_name("Field")
      .link_to(&m_field)
      .mark_basic();

  options().add_option("time", 0.).description("time after pulse").mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void InitAcousticVorticityPulse::execute()
{
  RealVector2 coord;
  Real time = options().option("time").value<Real>();

  cf3_assert(m_field);
  cf3_assert(m_field->coordinates().row_size()>=DIM_2D);
  cf3_assert(m_field->row_size()==4);
  for (Uint i=0; i<m_field->size(); ++i)
  {
    cf3_assert(i<m_field->coordinates().size());
    coord[XX] = m_field->coordinates()[i][XX];
    coord[YY] = m_field->coordinates()[i][YY];

//    std::cout << i << ": " << coord.transpose() << "   p=" << compute_pressure(coord,time) << std::endl;

    m_field->array()[i][3] = compute_pressure(coord,time);
    m_field->array()[i][0] = compute_density(m_field->array()[i][3],coord,time);
  }

}

//////////////////////////////////////////////////////////////////////////////

InitAcousticVorticityPulse::Data::Data()
{
    u0 = 0.5;
    alpha1 = std::log(2.)/9.;
    alpha2 = std::log(2.)/25.;

    s0 = 0;
    s1 = 1;
    while (std::exp(-s1*s1/(4.*alpha1)) > 1e-60)
    {
      s1+=1;
    }
}

Real InitAcousticVorticityPulse::Func::eta(const RealVector& coord, const Real& t) const
{
  return std::sqrt( (coord[XX]-m_data.u0*t)*(coord[XX]-m_data.u0*t) + coord[YY]*coord[YY]);
}

/// Actual function to be integrated
Real InitAcousticVorticityPulse::Func::operator()(Real lambda) const
{
  return std::exp(-lambda*lambda/(4.*m_data.alpha1))*std::cos(lambda*m_time)*j0(lambda*eta(m_coord,m_time))*lambda;
}


Real InitAcousticVorticityPulse::compute_pressure(const RealVector& coord, const Real& t)
{
  return 1./(2.*m_data.alpha1) * integrate( Func(coord, t, m_data), m_data.s0,m_data.s1);
}

Real InitAcousticVorticityPulse::compute_density(const Real& pressure, const RealVector& coord, const Real& t)
{
  Real x = (coord[XX]-67.) - m_data.u0*t;
  return pressure + 0.1*std::exp(-m_data.alpha2*(x*x+coord[YY]*coord[YY]));
}


//////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // physics
} // cf3
