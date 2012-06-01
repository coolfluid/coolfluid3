// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>
#include <sstream>
#include <iomanip>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "sdm/explicit_rungekutta/ButcherTableau.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {

///////////////////////////////////////////////////////////////////////////////////////

ButcherTableau::ButcherTableau(const std::string& name)
  : common::Component(name)
{
  options().add("a", std::vector<Real>())
      .description("RK coefficients a from Butcher tableau")
      .link_to(&m_coeffs.a)
      .attach_trigger( boost::bind( &ButcherTableau::configure_c, this ) );

  options().add("b", std::vector<Real>())
      .description("RK coefficients b from Butcher tableau")
      .link_to(&m_coeffs.b);

  options().add("order", m_coeffs.order)
      .link_to(&m_coeffs.order);
}

////////////////////////////////////////////////////////////////////////////////

void ButcherTableau::set(const Coefficients& coeffs)
{
  options().set("a",coeffs.a);
  options().set("b",coeffs.b);
  options().set("order",coeffs.order);
}

////////////////////////////////////////////////////////////////////////////////

bool ButcherTableau::check_throw() const
{
  if (m_coeffs.order == 0u)
  {
    throw SetupError( FromHere(), "order of Butcher tableau is not configured" );
    return false;
  }
  if (m_coeffs.nb_stages == 0u)
  {
    throw SetupError( FromHere(), "nb_stages of Butcher tableau is zero, configure coefficients \"a\"" );
    return false;
  }
  if (m_coeffs.b.size() != m_coeffs.nb_stages)
  {
    throw SetupError( FromHere(), "mismatch between b and nb_stages in Butcher tableau" );
    return false;
  }
  if ( (Uint) std::sqrt(m_coeffs.a.size()) != m_coeffs.nb_stages)
  {
    throw SetupError( FromHere(), "mismatch between a and nb_stages in Butcher tableau" );
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void ButcherTableau::configure_c()
{
  m_coeffs.nb_stages = (Uint) std::sqrt( m_coeffs.a.size() );
  m_coeffs.c.assign(m_coeffs.nb_stages,0.);
  for (Uint i=0; i<m_coeffs.nb_stages; ++i) {
    for (Uint j=0; j<m_coeffs.nb_stages-1; ++j) {
      m_coeffs.c[i] += a(i,j);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

std::string ButcherTableau::str() const
{
  std::stringstream ss;
  for (int i=0; i<nb_stages(); ++i)
  {
    ss << std::setw(9) << c(i) << " | ";
    for (int j=0; j<i; ++j)
      ss << std::setw(9) << a(i,j) << " ";
    ss << std::endl;
  }
  ss << "-----------";
  for (int i=0; i<nb_stages(); ++i)
  {
    ss << "----------";
  }
  ss << std::endl;
  ss << "          | ";
  for (int i=0; i<nb_stages(); ++i)
  {
    ss << std::setw(9) << b(i) << " ";
  }
  ss << std::endl;
  return ss.str();
}

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3
