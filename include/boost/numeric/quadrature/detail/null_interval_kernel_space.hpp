#ifndef BOOST_NUMERIC_QUADRATURE_NULL_INTERVAL_KERNEL_SPACE_HPP
#define BOOST_NUMERIC_QUADRATURE_NULL_INTERVAL_KERNEL_SPACE_HPP

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

/*!
  @file   null_interval_kernel_space.hpp
  @brief
  @author Hugo Duncan
  @date   2007-11-17
*/

#include <boost/numeric/quadrature/quadrature_config.hpp>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        struct null_interval_kernel_space
        {
          static null_interval_kernel_space& instance()
          {
            static null_interval_kernel_space inst;
            return inst;
          }
        };
      }
    }// namespace
  }// namespace
}// namespace

#endif
