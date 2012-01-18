#ifndef BOOST_NUMERIC_QUADRATURE_IS_ARITHMETIC_SCALAR_H
#define BOOST_NUMERIC_QUADRATURE_IS_ARITHMETIC_SCALAR_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   is_arithmetic_scalar.hpp
  @brief  Traits for use in quadrature routines
*/

#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_pointer.hpp>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      //! trait that can be specialised for other scalar types
      /*! overide this to use a type such as NTL::quad_float,
        or boost::numeric::interval
       */
      template <typename T>
      struct is_arithmetic_scalar
        : ::boost::mpl::and_<is_arithmetic<T>,
          ::boost::mpl::not_<is_pointer<T> > >
      { };

      template <typename T>
      struct is_arithmetic_scalar_
        : ::boost::mpl::and_<is_arithmetic<typename T::type>,
          ::boost::mpl::not_<is_pointer<typename T::type> > >
      {};

    }// namespace
  }// namespace
}// namespace

#endif
