#ifndef BOOST_NUMERIC_QUADRATURE_ARITHMETIC_VECTOR_TRAITS_H
#define BOOST_NUMERIC_QUADRATURE_ARITHMETIC_VECTOR_TRAITS_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   arithmetic_vector_traits.hpp
  @brief  Traits for use in quadrature routines
*/

#include <boost/range.hpp>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {

      //! trait that can be specialised for vector types
      template <typename T>
      struct arithmetic_vector_value
        : boost::range_value<T>
      {};

      //! trait that can be specialised for vector types
      template <typename T>
      struct arithmetic_vector_size;

    }// namespace
  }// namespace
}// namespace

#endif
