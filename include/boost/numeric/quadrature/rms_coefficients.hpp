#ifndef BOOST_NUMERIC_QUADRATURE_RMS_COEFFICIENTS_HPP
#define BOOST_NUMERIC_QUADRATURE_RMS_COEFFICIENTS_HPP

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   rms_coefficients.hpp
  @brief  Coefficients for RMS integration formulae
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

      //! Coefficienst for Recusrsive Monotone Stable intergration
      /*! Coefficients for N point rms intergration
       */
      template <typename T, std::size_t N>
      struct rms_coefficients
#ifdef BOOST_QUADRATURE_DOXYGEN
      {
        typedef std::tr1::array<T,N/2+1> value_type;
        // Weights
        static const value_type& weights();
        // Abscissa
        static const value_type& abcissa();
      }
#endif
      ;

      template <typename T>
      struct rms_coefficients<T,13>
      {
        typedef std::tr1::array<T,7> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(1.303262173284849021810473057638590518409112513421e-1),
            SC_(2.390632866847646220320329836544615917290026806242e-1),
            SC_(2.630626354774670227333506083741355715758124943143e-1),
            SC_(2.186819313830574175167853094864355208948886875898e-1),
            SC_(2.757897646642836865859601197607471574336674206700e-2),
            SC_(1.055750100538458443365034879086669791305550493830e-1),
            SC_(1.571194260595182254168429283636656908546309467968e-2)
          };
          return w;
        }

        static const value_type& abcissa()
        {
          static const value_type x={
            SC_(.0),
            SC_(.25000000000000000000),
            SC_(.50000000000000000000),
            SC_(.75000000000000000000),
            SC_(.87500000000000000000),
            SC_(.93750000000000000000),
            1.
          };
          return x;
        }
      };


      template <typename T>
      struct rms_coefficients<T,19>
      {
        typedef std::tr1::array<T,10> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(1.298751627936015783241173611320651866834051160074e-1),
            SC_(2.249996826462523640447834514709508786970828213187e-1),
            SC_(1.680415725925575286319046726692683040162290325505e-1),
            SC_(1.415567675701225879892811622832845252125600939627e-1),
            SC_(1.006482260551160175038684459742336605269707889822e-1),
            SC_(2.510604860724282479058338820428989444699235030871e-2),
            SC_(9.402964360009747110031098328922608224934320397592e-3),
            SC_(5.542699233295875168406783695143646338274805359780e-2),
            SC_(9.986735247403367525720377847755415293097913496236e-2),
            SC_(4.507523056810492466415880450799432587809828791196e-2)
          };
          return w;
        }

        static const value_type& abcissa()
        {
          static const value_type x={
            SC_(.0),
            SC_(.25000000000000000000e+00),
            SC_(.50000000000000000000e+00),
            SC_(.75000000000000000000e+00),
            SC_(.87500000000000000000e+00),
            SC_(.93750000000000000000e+00),
            SC_(.10000000000000000000e+01),
            SC_(.37500000000000000000e+00),
            SC_(.62500000000000000000e+00),
            SC_(.96875000000000000000e+00)
          };
          return x;
        }

        typedef std::tr1::array<bool,9> flags_type;
        static const flags_type& crossed_flags()
        {
          static const flags_type crossed_values={
            true,true,false,false,false,true,false,false,false
          };
          return crossed_values;
        }

        static const flags_type& bisected_flags()
        {
          static const flags_type bisected_values={
            true,true,true,true,true,true,false,false,false
          };
          return bisected_values;
        }

      };


      template <typename T>
      struct rms_coefficients<T,27>
      {
        typedef std::tr1::array<T,14> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(6.300942249647773931746170540321811473310938661469e-2),
            SC_(1.261383225537664703012999637242003647020326905948e-1),
            SC_(1.273864433581028272878709981850307363453523117880e-1),
            SC_(8.576500414311820514214087864326799153427368592787e-2),
            SC_(7.102884842310253397447305465997026228407227220665e-2),
            SC_(5.026383572857942403759829860675892897279675661654e-2),
            SC_(4.683670010609093810432609684738393586390722052124e-3),
            SC_(1.235837891364555000245004813294817451524633100256e-1),
            SC_(1.148933497158144016800199601785309838604146040215e-1),
            SC_(1.252575774226122633391477702593585307254527198070e-2),
            SC_(1.239572396231834242194189674243818619042280816640e-1),
            SC_(2.501306413750310579525950767549691151739047969345e-2),
            SC_(4.915957918146130094258849161350510503556792927578e-2),
            SC_(2.259167374956474713302030584548274729936249753832e-2)
          };
          return w;
        }

        static const value_type& abcissa()
        {
          static const value_type x={
            SC_(.0)                       ,
            SC_(.25000000000000000000e+00),
            SC_(.50000000000000000000e+00),
            SC_(.75000000000000000000e+00),
            SC_(.87500000000000000000e+00),
            SC_(.93750000000000000000e+00),
            SC_(.10000000000000000000e+01),
            SC_(.37500000000000000000e+00),
            SC_(.62500000000000000000e+00),
            SC_(.96875000000000000000e+00),
            SC_(.12500000000000000000e+00),
            SC_(.68750000000000000000e+00),
            SC_(.81250000000000000000e+00),
            SC_(.98437500000000000000e+00)
          };
          return x;
        }

        typedef std::tr1::array<bool,13> flags_type;
        static const flags_type& crossed_flags()
        {
          static const flags_type crossed_values={
            true,true,true,false,false,true,false,false,false,false,
            false,false,false
          };
          return crossed_values;
        }

        static const flags_type& bisected_flags()
        {
          static const flags_type bisected_values={
            true,true,true,true,true,true,true,true,true,false,
            false,false,false
          };
          return bisected_values;
        }
      };


      template <typename T>
      struct rms_coefficients<T,41>
      {
        typedef std::tr1::array<T,21> value_type;

        static const value_type& weights()
        {
          static const value_type w={
            SC_(6.362762978782724559269342300509058175967124446839e-2),
            SC_(9.950065827346794643193261975720606296171462239514e-2),
            SC_(7.048220002718565366098742295389607994441704889441e-2),
            SC_(6.512297339398335645872697307762912795346716454337e-2),
            SC_(3.998229150313659724790527138690215186863915308702e-2),
            SC_(3.456512257080287509832054272964315588028252136044e-2),
            SC_(2.212167975884114432760321569298651047876071264944e-3),
            SC_(8.140326425945938045967829319725797511040878579808e-2),
            SC_(6.583213447600552906273539578430361199084485578379e-2),
            SC_(2.592913726450792546064232192976262988065252032902e-2),
            SC_(1.187141856692283347609436153545356484256869129472e-1),
            SC_(5.999947605385971985589674757013565610751028128731e-2),
            SC_(5.500937980198041736910257988346101839062581489820e-2),
            SC_(5.264422421764655969760271538981443718440340270116e-3),
            SC_(1.533126874056586959338368742803997744815413565014e-2),
            SC_(3.527159369750123100455704702965541866345781113903e-2),
            SC_(5.000556431653955124212795201196389006184693561679e-2),
            SC_(5.744164831179720106340717579281831675999717767532e-2),
            SC_(1.598823797283813438301248206397233634639162043386e-2),
            SC_(2.635660410220884993472478832884065450876913559421e-2),
            SC_(1.196003937945541091670106760660561117114584656319e-2)
          };
          return w;
        }

        static const value_type& abcissa()
        {
          static const value_type x={
            SC_(.0),
            SC_(.25000000000000000000e+00),
            SC_(.50000000000000000000e+00),
            SC_(.75000000000000000000e+00),
            SC_(.87500000000000000000e+00),
            SC_(.93750000000000000000e+00),
            SC_(.10000000000000000000e+01),
            SC_(.37500000000000000000e+00),
            SC_(.62500000000000000000e+00),
            SC_(.96875000000000000000e+00),
            SC_(.12500000000000000000e+00),
            SC_(.68750000000000000000e+00),
            SC_(.81250000000000000000e+00),
            SC_(.98437500000000000000e+00),
            SC_(.18750000000000000000e+00),
            SC_(.31250000000000000000e+00),
            SC_(.43750000000000000000e+00),
            SC_(.56250000000000000000e+00),
            SC_(.84375000000000000000e+00),
            SC_(.90625000000000000000e+00),
            SC_(.99218750000000000000e+00)
          };
          return x;
        }

        typedef std::tr1::array<bool,20> flags_type;
        static const flags_type& crossed_flags()
        {
          static const flags_type crossed_values={
            true,true,true,false,false,true,true,true,false,true,
            false,false,false,false,false,false,false,false,false,false
          };
          return crossed_values;
        }

        static const flags_type& bisected_flags()
        {
          static const flags_type bisected_values={
            true,true,true,true,true,true,true,true,true,true,
            true,true,true,false,false,false,false,false,false,false
          };
          return bisected_values;
        }
      };

#undef SC_
    }// namespace
  }// namespace
}// namespace

#endif
