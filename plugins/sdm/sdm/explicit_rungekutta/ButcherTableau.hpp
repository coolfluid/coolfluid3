// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file ButcherTableau.hpp
/// @author Willem Deconinck
///
/// This file defines a Butcher Tableau component

#ifndef cf3_sdm_explicit_rungekutta_ButcherTableau_hpp
#define cf3_sdm_explicit_rungekutta_ButcherTableau_hpp

#include "sdm/explicit_rungekutta/LibExplicitRungeKutta.hpp"

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {

////////////////////////////////////////////////////////////////////////////////

/// @brief Component to hold a Butcher tableau, used in Runge-Kutta integration methods.
/// @author Willem Deconinck
///
/// The Butcher tableau is used in Runge Kutta time integration methods
/// @verbatim
/// 0  |
/// c2 | a21
/// c3 | a31  a32
///  : |  :       `-.
/// cs | as1  as2  ..  as,s-1
/// ------------------------------
///    | b1   b2   ..   bs-1    bs
/// @endverbatim
/// This component is configurable with a vector of coefficients "a", and "b",
/// and an informative option "order", describing the order of the RK method,
/// using this tableau.
/// The coefficients "c" are deduced from the coefficients "a",
/// as the coefficients "c" are always the summation of the corresponding row
/// of coefficients "a".
class sdm_explicit_rungekutta_API ButcherTableau : public common::Component
{
public:

  /// @brief Coefficients from the Butcher tableau
  struct Coefficients
  {
    std::vector<Real> a;  ///< coefficients "a"
    std::vector<Real> b;  ///< coefficients "b"
    std::vector<Real> c;  ///< coefficients "c"

    Uint order;      ///< order of the table
    Uint nb_stages;  ///< number of stages of the RK method
  };

public:

  /// @brief Type name
  static std::string type_name() { return "ButcherTableau"; }

  /// @brief Constructor
  ButcherTableau(const std::string& name);

  /// @brief Configure the internal coefficients through a given Coefficients object
  void set (const Coefficients& coeffs);

  // Coefficient access
  /// @brief Access to element of the matrix of "a" coefficients
  const Real& a(const Uint i, const Uint j) const { return m_coeffs.a[i*m_coeffs.nb_stages + j]; }

  /// @brief Access to element of the "b" coefficients
  const Real& b(const Uint i) const               { return m_coeffs.b[i]; }

  /// @brief Access to element of the "c" coefficients
  const Real& c(const Uint i) const               { return m_coeffs.c[i]; }

  /// @brief Number of stages
  Uint nb_stages() const { return m_coeffs.nb_stages; }

  /// @brief Order
  Uint order() const { return m_coeffs.order; }

  /// @brief Check if Butcher tableau is consistent, and throw error if not.
  bool check_throw() const;

  /// @brief String representation of the Butcher tableau:
  /// @verbatim
  /// 0  |
  /// c2 | a21
  /// c3 | a31  a32
  ///  : |  :       `-.
  /// cs | as1  as2  ..  as,s-1
  /// ------------------------------
  ///    | b1   b2   ..   bs-1    bs
  /// @endverbatim
  std::string str() const;

private: // functions

  /// @brief configure "c" coefficients when "a" coefficients are configured
  void configure_c();

private: // data

  Coefficients m_coeffs; ///< Internal coefficients storage

};

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3

#endif // cf3_sdm_explicit_rungekutta_ButcherTableau_hpp
