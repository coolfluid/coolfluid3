#ifndef BOOST_NUMERIC_QUADRATURE_INTERVAL_FWD_HPP
#define BOOST_NUMERIC_QUADRATURE_INTERVAL_FWD_HPP

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   interval_fwd.hpp
  @brief  Forward declaration for interval.hpp
*/

#include <boost/numeric/quadrature/quadrature_config.hpp>
#include <boost/call_traits.hpp>
#include <boost/range.hpp>
#include <numeric>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        struct null_interval_kernel_space;
      }

      template <
          typename Domain,
          typename Image,
          typename KernelSpace=detail::null_interval_kernel_space>
      class interval;

    }// namespace
  }// namespace
}// namespace

#endif
