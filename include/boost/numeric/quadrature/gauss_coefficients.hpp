#ifndef BOOST_NUMERIC_QUADRATURE_GAUSS_COEFFICIENTS_HPP
#define BOOST_NUMERIC_QUADRATURE_GAUSS_COEFFICIENTS_HPP

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   gauss_coefficients.hpp
  @brief  Coefficients for Gauss integration formulae
*/

#include "boost/numeric/quadrature/quadrature_config.hpp"
#include <boost/tr1/array.hpp>

#define SC_(x) static_cast<T>(BOOST_JOIN(x, L))

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      //! Coefficienst for gauss quadrature
      /*! Coefficients for gauss quadrature to use with N point kronrod
       */
      template <typename T, std::size_t N>
      struct gauss_coefficients
#ifdef BOOST_QUADRATURE_DOXYGEN
        {
          typedef std::tr1::array<T,N/2+1> value_type;
          //! Weights for integrand values
          static const value_type& weights();
        }
#endif
      ;


      template <typename T>
      struct gauss_coefficients<T,7>
      {
        typedef std::tr1::array<T,4> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(0.129484966168869693270611432679082) ,
            SC_(0.279705391489276667901467771423780) ,
            SC_(0.381830050505118944950369775488975) ,
            SC_(0.417959183673469387755102040816327)
          };
          return w;
        }
      };

      template <typename T>
      struct gauss_coefficients<T,10>
      {
        typedef std::tr1::array<T,5> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(0.066671344308688137593568809893332),
            SC_(0.149451349150580593145776339657697),
            SC_(0.219086362515982043995534934228163),
            SC_(0.269266719309996355091226921569469),
            SC_(0.295524224714752870173892994651338)
          };
          return w;
        }
      };

      template <typename T>
      struct gauss_coefficients<T,15>
      {
        typedef std::tr1::array<T,8> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(0.030753241996117268354628393577204),
            SC_(0.070366047488108124709267416450667),
            SC_(0.107159220467171935011869546685869),
            SC_(0.139570677926154314447804794511028),
            SC_(0.166269205816993933553200860481209),
            SC_(0.186161000015562211026800561866423),
            SC_(0.198431485327111576456118326443839),
            SC_(0.202578241925561272880620199967519)
          };
          return w;
        }
      };

      template <typename T>
      struct gauss_coefficients<T,20>
      {
        typedef std::tr1::array<T,10> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(0.017614007139152118311861962351853),
            SC_(0.040601429800386941331039952274932),
            SC_(0.062672048334109063569506535187042),
            SC_(0.083276741576704748724758143222046),
            SC_(0.101930119817240435036750135480350),
            SC_(0.118194531961518417312377377711382),
            SC_(0.131688638449176626898494499748163),
            SC_(0.142096109318382051329298325067165),
            SC_(0.149172986472603746787828737001969),
            SC_(0.152753387130725850698084331955098)
          };
          return w;
        }
      };

      template <typename T>
      struct gauss_coefficients<T,25>
      {
        typedef std::tr1::array<T,13> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(0.011393798501026287947902964113235),
            SC_(0.026354986615032137261901815295299),
            SC_(0.040939156701306312655623487711646),
            SC_(0.054904695975835191925936891540473),
            SC_(0.068038333812356917207187185656708),
            SC_(0.080140700335001018013234959669111),
            SC_(0.091028261982963649811497220702892),
            SC_(0.100535949067050644202206890392686),
            SC_(0.108519624474263653116093957050117),
            SC_(0.114858259145711648339325545869556),
            SC_(0.119455763535784772228178126512901),
            SC_(0.122242442990310041688959518945852),
            SC_(0.123176053726715451203902873079050)
          };
          return w;
        }
      };

#undef SC_
    }// namespace
  }// namespace
}// namespace

#endif
