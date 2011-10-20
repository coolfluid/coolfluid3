#ifndef BOOST_NUMERIC_QUADRATURE_PROMOTION_H
#define BOOST_NUMERIC_QUADRATURE_PROMOTION_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   promotion.hpp
  @brief  Promotion of arguments
*/

#include <boost/numeric/quadrature/quadrature_config.hpp>
#include <boost/math/tools/promotion.hpp>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        template <typename T1, typename T2>
        typename boost::math::tools::promote_args<T1,T2>::type
        promote(T1& arg, T2&)
        {
          return static_cast<
            typename boost::math::tools::promote_args<T1,T2>::type>(arg);
        }

      }// namespace

    }// namespace
  }// namespace
}// namespace

#endif
