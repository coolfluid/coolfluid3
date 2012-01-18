#ifndef BOOST_NUMERIC_QUADRATURE_QUADRATURE_TRAITS_H
#define BOOST_NUMERIC_QUADRATURE_QUADRATURE_TRAITS_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   quadrature_traits.hpp
  @brief  Traits for use in quadrature routines
*/

#include <boost/numeric/quadrature/interval_fwd.hpp>
#include <boost/numeric/quadrature/detail/local_traits.hpp>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {

      namespace detail
      {
        struct null_workspace {};

        //! Kernel interval trait
        /*! Used to specify the type of interval storage required */
        template <typename Kernel, typename Domain, typename Image>
        struct kernel_interval
        {
          typedef null_interval_kernel_space type;
        };

      }

      // traits for integration kernels

      //! Kernel workspace trait
      /*! Used to specify the type of workspace storage required */
      template <typename Kernel, typename Domain, typename Image>
      struct kernel_workspace
      {
        typedef detail::null_workspace type;
      };

      //! Kernel interval trait
      /*! Used to specify the type of interval storage required.
        Vector types are set use array here, to reduce the number
        of template instantiations.
       */
      template <typename Kernel, typename Domain, typename Image>
      struct kernel_interval
      {
        typedef typename boost::numeric::quadrature::interval<
            typename detail::storage_for_type<Domain>::type,
            typename detail::storage_for_type<Image>::type,
            typename detail::kernel_interval<
                Kernel,
                typename detail::storage_for_type<Domain>::type,
                typename detail::storage_for_type<Image>::type >::type
            > type;
      };



    }// namespace
  }// namespace
}// namespace

#endif
