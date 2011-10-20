#ifndef BOOST_NUMERIC_QUADRATURE_ARRAY_TRAITS_H
#define BOOST_NUMERIC_QUADRATURE_ARRAY_TRAITS_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

/*!
  @file   array_traits.hpp
  @brief  traits for use internally in quadrature routines
  @author Hugo Duncan
  @date   2007-11-21
*/

#include <boost/numeric/quadrature/arithmetic_vector_traits.hpp>
#include <boost/numeric/quadrature/is_arithmetic_scalar.hpp>
#include <boost/numeric/quadrature/detail/local_traits.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/tr1/array.hpp>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        template <typename Value, std::size_t N>
        struct array_storage_for_scalar_type
        {
          typedef typename std::tr1::array<
              typename storage_for_scalar_type<Value>::type, N> type;
        };

        template <typename Value, std::size_t N>
        struct array_storage_for_vector_type
        {
          typedef typename std::tr1::array<
              typename arithmetic_vector_value<Value>::type, N > arithmetic_array_type;
          typedef typename boost::array<
              arithmetic_array_type,arithmetic_vector_size<Value>::value > type;
        };


        //! Compute the type of a array for storing the specified Value type
        /*! Store array values as an array of array of components rather than
            as a array of arrays
        */
        template <typename Value, std::size_t N>
        struct array_storage_for_type
          : mpl::if_<
            is_scalar<Value>,
            array_storage_for_scalar_type<Value, N>,
            array_storage_for_vector_type<Value, N>
            >::type
        {
        };

      }// namespace

    }// namespace
  }// namespace
}// namespace

#endif
