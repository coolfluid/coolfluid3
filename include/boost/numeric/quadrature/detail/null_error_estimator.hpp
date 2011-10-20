#ifndef BOOST_NUMERIC_QUADRATURE_NULL_ERROR_ESTIMATOR_H
#define BOOST_NUMERIC_QUADRATURE_NULL_ERROR_ESTIMATOR_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

/*! null error estimation algorithm
  @file   null_error_estimator.hpp
  @brief
  @author Hugo Duncan
  @date   2007-11-17
*/

#include "boost/numeric/quadrature/quadrature_config.hpp"
#include "boost/numeric/quadrature/quadrature_traits.hpp"
#include "boost/numeric/quadrature/detail/local_traits.hpp"

#include <cmath>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {

        //! Error estimation null algorithm, that does nothing
        struct null_error_estimator
        {
          template <typename Weight, typename Image>
          void center_point(Weight, Image) const {}

          template <typename Weight, typename Image>
          void point(Weight, Image, Image) const {}


          template <typename Domain, typename Image,
              typename ImagePoints, typename Weight, typename WeightRange>
          void operator()(
          const Image& resk,
          const Image& resg,
          const Image& center_value,
          const ImagePoints& lower_values,
          const ImagePoints& upper_values,
          Weight center_weight,
          const WeightRange& weights,
          Domain half_length
                                      ) const {}

          static null_error_estimator& instance()
          {
            static null_error_estimator inst;
            return inst;
          }
        };

      }// namespace
    }// namespace
  }// namespace
}// namespace

#endif
