// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_explicit_rungekutta_ButcherTableau_hpp
#define cf3_sdm_explicit_rungekutta_ButcherTableau_hpp

#include "sdm/explicit_rungekutta/LibExplicitRungeKutta.hpp"

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {

////////////////////////////////////////////////////////////////////////////////

class sdm_explicit_rungekutta_API ButcherTableau : public common::Component
{
public:

  struct Coefficients
  {
    std::vector<Real> a;
    std::vector<Real> b;
    std::vector<Real> c;

    Uint order;
    Uint nb_stages;
  };

public:
  static std::string type_name() { return "ButcherTableau"; }
  ButcherTableau(const std::string& name);

  void set (const Coefficients& coeffs);

  // Vector access
  const std::vector<Real>& a() const { return m_coeffs.a; }
  const std::vector<Real>& b() const { return m_coeffs.b; }
  const std::vector<Real>& c() const { return m_coeffs.c; }

  // Coefficient access
  const Real& a(const Uint i, const Uint j) const { return m_coeffs.a[i*m_coeffs.nb_stages + j]; }
  const Real& b(const Uint i) const               { return m_coeffs.b[i]; }
  const Real& c(const Uint i) const               { return m_coeffs.c[i]; }

  // Number of stages
  Uint nb_stages() const { return m_coeffs.nb_stages; }

  Uint order() const { return m_coeffs.order; }

  bool check_throw() const;

  std::string str() const;

private: // functions

  void configure_c();

private: // data

  Coefficients m_coeffs;

};

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3

#endif // cf3_sdm_explicit_rungekutta_ButcherTableau_hpp
