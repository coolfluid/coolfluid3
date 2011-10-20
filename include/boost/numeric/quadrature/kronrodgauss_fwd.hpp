#ifndef BOOST_NUMERIC_QUADRATURE_KRONRODGAUSS_FWD_H
#define BOOST_NUMERIC_QUADRATURE_KRONRODGAUSS_FWD_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! Kronrod-Gauss Integration forward declaration
  @file   kronrodgauss_fwd.hpp
  @brief  Forward declaration of kronrod-gauss quadrature
*/

#include <boost/numeric/quadrature/detail/arg_chaining.hpp>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      //! N point Kronrod-Gauss quadrature
      /*! N point Kronrod quadrature with N/2 point Gauss interpolation.
        N is assumed odd, so coefficients become symmetric.
        See http://en.wikipedia.org/wiki/Gaussian_quadrature
        Adapted from quadpack.
      */
      template <
          std::size_t N,
          BOOST_QUADRATURE_ARG_TEMPLATE(ErrorEst),
          BOOST_QUADRATURE_ARG_TEMPLATE(Recorder),
          BOOST_QUADRATURE_ARG_TEMPLATE(Data)
          > class kronrod_gauss;

    }// namespace
  }// namespace
}// namespace

#endif
