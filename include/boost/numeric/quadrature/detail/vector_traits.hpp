#ifndef BOOST_NUMERIC_QUADRATURE_VECTOR_TRAITS_H
#define BOOST_NUMERIC_QUADRATURE_VECTOR_TRAITS_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

/*!
  @file   vector_traits.hpp
  @brief  traits for use internally in quadrature routines
  @author Hugo Duncan
  @date   2007-11-21
*/

#include <boost/numeric/quadrature/arithmetic_vector_traits.hpp>
#include <boost/numeric/quadrature/detail/local_traits.hpp>
#include <boost/mpl/identity.hpp>
#include <vector>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        template <typename Value>
        struct vector_storage_for_scalar_type
        {
          typedef typename std::vector<
              typename storage_for_scalar_type<Value>::type > type;
        };

        template <typename Value>
        struct vector_storage_for_vector_type
        {
          typedef typename std::vector<
              typename arithmetic_vector_value<Value>::type > arithmetic_vector_type;
          typedef typename boost::array<
              arithmetic_vector_type,arithmetic_vector_size<Value>::value > type;
        };


        //! Compute the type of a vector for storing the specified Value type
        /*! Store array values as an array of vector of components rather than
            as a vectory of arrays
        */
        template <typename Value>
        struct vector_storage_for_type
          : mpl::if_<
            is_scalar<Value>,
             vector_storage_for_scalar_type<Value>,
             vector_storage_for_vector_type<Value>
            >::type
        {
        };

      }// namespace

    }// namespace
  }// namespace
}// namespace

#endif
