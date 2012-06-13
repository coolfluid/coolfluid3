#ifndef BOOST_NUMERIC_QUADRATURE_QUADRATURE_ERROR_HPP
#define BOOST_NUMERIC_QUADRATURE_QUADRATURE_ERROR_HPP

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! Provides standardised error enum
  @file   quadrature_error.hpp
  @brief  Error definitions for quadrature routines
*/

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      //! Error return valus for quadrature routines
      enum quadrature_error {
        quadrature_ok=0,
        /*! maximum number of subdivisions allowed has been achieved. one
          can allow more sub-divisions by increasing the value of limit
          (and taking the according dimension adjustments into
          account. however, if this yields no improvement it is advised
          to analyze the integrand in order to determine the integration
          difficulties. if the position of a local difficulty can be
          determined (e.g. singularity, discontinuity within the
          interval) one will probably gain from splitting up the
          interval at this point and calling the integrator on the
          subranges. if possible, an appropriate special-purpose
          integrator should be used, which is designed for handling the
          type of difficulty involved.
        */
        max_intervals_reached,
        /** the occurrence of roundoff error is detected, which prevents
            the requested tolerance from being achieved.  the error may be
            under-estimated.
        */
        roundoff_detected,
        /** extremely bad integrand behaviour occurs at some points of
            the integration interval.
        */
        bad_integrand_behaviour,
        /** the algorithm does not converge. roundoff error is detected in the
            extrapolation table. it is presumed that
            the requested tolerance cannot be
            achieved, and that the returned result is
            the best which can be obtained.
        */
        no_convergence,
        /*! the integral is probably divergent, or slowly convergent. */
        divergent,
        /*! the input is invalid */
        invalid_input
      };


#ifndef BOOST_QUADRATURE_USER_ERROR_HANDLER
      inline quadrature_error quadrature_error_handler(
        const char* function,
        quadrature_error err)
      {
        return err;
      }
#endif
    }// namespace
  }// namespace
}// namespace

#endif
