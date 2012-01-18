#ifndef BOOST_NUMERIC_QUADRATURE_BOOST_INTERVAL_H
#define BOOST_NUMERIC_QUADRATURE_BOOST_INTERVAL_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! Trait specialisations for using boost::interval
  @file   boost_interval.hpp
  @brief  Class for estimating the integration error
*/

#include "boost/numeric/quadrature/quadrature_config.hpp"
#include "boost/numeric/quadrature/is_arithmetic_scalar.hpp"
#include "boost/numeric/interval/interval.hpp"
#include "boost/numeric/interval/limits.hpp"
#include "boost/numeric/interval/utility.hpp"
#include "boost/numeric/interval/arith2.hpp"
#include "boost/mpl/bool.hpp"
#include "boost/math/tools/precision.hpp"
#include "boost/math/special_functions/fpclassify.hpp"
#include <cmath>

namespace boost
{
  namespace math
  {
#ifdef BOOST_MSVC
    template <>
    bool isnan BOOST_NO_MACRO_EXPAND(numeric::interval<double> value)
    {
      return (isnan)(value.upper())||(isnan)(value.lower());
    }
#endif
  } // namespace math

  namespace numeric
  {
    namespace quadrature
    {
      template <class T, class Policies>
      struct is_arithmetic_scalar< ::boost::numeric::interval<T,Policies> >
        : boost::mpl::bool_<true>
      {};

      namespace detail
      {
        // interval doesn't seem to have non-integral pow function
        template <typename Value>
        void error_estimate(
          numeric::interval<Value>& abserr,
          const numeric::interval<Value>& resasc)
        {
          using namespace std;
          if (resasc!=0.0 && abserr!=0)
            abserr= resasc*(min)(
              static_cast<Value>(1.),
              (pow)(static_cast<Value>(200.)*abserr/resasc,2)/
              (pow)(static_cast<Value>(200.)*abserr/resasc,3));
        }
      } // namespace detail


    }// namespace
  }// namespace
}// namespace

#endif
