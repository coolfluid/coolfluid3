#ifndef BOOST_NUMERIC_QUADRATURE_RMS_H
#define BOOST_NUMERIC_QUADRATURE_RMS_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! Recursive Monotone Stable Integration
  @file   rms.hpp
  @brief  Class for performing N point RMS quadrature
*/

#include "boost/numeric/quadrature/quadrature_config.hpp"
#include "boost/numeric/quadrature/rms_coefficients.hpp"
#include "boost/numeric/quadrature/quadrature_error.hpp"
#include "boost/numeric/quadrature/quadrature_traits.hpp"
#include "boost/numeric/quadrature/detail/null_recorder.hpp"
#include "boost/numeric/quadrature/detail/null_interval_kernel_space.hpp"
#include "boost/numeric/quadrature/detail/null_error_estimator.hpp"
#include "boost/numeric/quadrature/detail/local_traits.hpp"
#include "boost/numeric/quadrature/detail/arg_chaining.hpp"
#include <boost/numeric/quadrature/promotion.hpp>

#include <boost/math/tools/config.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include <boost/array.hpp>

#include <cmath>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      template <
          std::size_t N, std::size_t M,
          BOOST_QUADRATURE_ARG_TEMPLATE(ErrorEst),
          BOOST_QUADRATURE_ARG_TEMPLATE(Recorder),
          BOOST_QUADRATURE_ARG_TEMPLATE(Data)
          > class rms;

      namespace detail
      {
        enum interval_states {
          // no values evaluated
            empty,
            // values evaluated from lower->upper of upper->lower
            crossed,
            // values evaluated from lower->lower of upper->upper
            bisected };

        // rms per interval storage
        /* This structure is used to take advantage of the structure of
          the RMS abcissa
        */
        template <std::size_t N, std::size_t M, typename Domain, typename Image>
        struct rms_interval
        {
          rms_interval()
          {
#ifdef BOOST_QUADRATURE_DEBUG
            if (std::numeric_limits<scalar_type>::has_quiet_NaN)
            {
              scalar_type nan=std::numeric_limits<scalar_type>::quiet_NaN();
              assign(m_center,nan);
              assign(m_lower_values,nan);
              assign(m_upper_values,nan);
            }
#endif
            reset();
          }

          typedef typename scalar_component<Image>::type scalar_type;
          typedef typename detail::storage_for_type<Image>::type image;
          typedef std::tr1::array<image, N/2> array;

          array m_lower_values;
          array m_upper_values;
          image m_center;
          interval_states m_lower_status;
          interval_states m_upper_status;

          void reset()
          {
            m_lower_status=empty;
            m_upper_status=empty;
          }
        };

        // specialise traits to use rms_interval
        template <
            std::size_t N, std::size_t M,
            typename ErrorEst, typename Recorder, typename Data,
            typename Domain, typename Image>
        struct kernel_interval<rms<N,M,ErrorEst,Recorder,Data>, Domain, Image>
        {
          typedef rms_interval<N,M,Domain,
              typename storage_for_type<Image>::type> type;
        };

        // bisect the per interval storage
        /*
          @param[in,out] lower on input, the interval to be bisected, on output
                         the lower half of the bisected range
          @param[out]    upper the upper half of the bisected range
         */
        template <std::size_t N, std::size_t M, typename Domain, typename Image>
        void bisect19(rms_interval<N, M, Domain, Image>& lower,
                      rms_interval<N, M, Domain, Image>& upper)
        {
          upper.m_lower_status=crossed;
          upper.m_upper_status=bisected;

          lower.m_lower_status=bisected;
          lower.m_upper_status=crossed;

          // Take values from lower and populate upper
          upper.m_center=lower.m_upper_values[1];
          upper.m_upper_values[5]=lower.m_upper_values[5]; //1->+1
          upper.m_lower_values[5]=lower.m_center;//center -> -1
          upper.m_upper_values[1]=lower.m_upper_values[2]; //0.75->+0.5
          upper.m_lower_values[1]=lower.m_upper_values[0];//0.25->-0.5
          upper.m_upper_values[0]=lower.m_upper_values[7];//0.625->+0.25
          upper.m_lower_values[0]=lower.m_upper_values[6];//0.375->-0.25
          upper.m_upper_values[2]=lower.m_upper_values[3]; //0.875->+0.75
          upper.m_upper_values[3]=lower.m_upper_values[4]; //0.9375->+0.875
          upper.m_upper_values[4]=lower.m_upper_values[8]; //0.96975->+0.9375

          // rearrange the values in lower
          //lower.m_lower_values[5]=lower.m_lower_values[5]; //1->+1
          lower.m_upper_values[5]=lower.m_center;//center -> -1
          lower.m_center=lower.m_lower_values[1];
          lower.m_lower_values[1]=lower.m_lower_values[2]; //0.75->+0.5
          lower.m_upper_values[1]=lower.m_lower_values[0];//0.25->-0.5
          lower.m_lower_values[0]=lower.m_lower_values[7];//0.625->+0.25
          lower.m_upper_values[0]=lower.m_lower_values[6];//0.375->-0.25
          lower.m_lower_values[2]=lower.m_lower_values[3]; //0.875->+0.75
          lower.m_lower_values[3]=lower.m_lower_values[4]; //0.9375->+0.875
          lower.m_lower_values[4]=lower.m_lower_values[8]; //0.96975->+0.9375
        }

        // bisect the per interval storage
        /*
          @param[in,out] lower on input, the interval to be bisected, on output
                         the lower half of the bisected range
          @param[out]    upper the upper half of the bisected range
         */
        template <std::size_t N, std::size_t M, typename Domain, typename Image>
        void bisect27(rms_interval<N, M, Domain, Image>& lower,
                      rms_interval<N, M, Domain, Image>& upper)
        {
          // Take values from lower and populate upper
          upper.m_lower_values[2]=lower.m_upper_values[9]; //0.125->-0.75
          upper.m_upper_values[6]=lower.m_upper_values[10]; //0.6875->+0.375
          upper.m_upper_values[7]=lower.m_upper_values[11]; //0.8125->+0.625
          upper.m_upper_values[8]=lower.m_upper_values[12]; //0.984375->+0.96875

          // rearrange the values in lower
          lower.m_upper_values[2]=lower.m_lower_values[9];
          lower.m_lower_values[6]=lower.m_lower_values[10];
          lower.m_lower_values[7]=lower.m_lower_values[11];
          lower.m_lower_values[8]=lower.m_lower_values[12];
        }

        // bisect the per interval storage
        /*
          @param[in,out] lower on input, the interval to be bisected, on output
                         the lower half of the bisected range
          @param[out]    upper the upper half of the bisected range
         */
        template <std::size_t N, std::size_t M, typename Domain, typename Image>
        void bisect41(rms_interval<N, M, Domain, Image>& lower,
                      rms_interval<N, M, Domain, Image>& upper)
        {
          // Take values from lower and populate upper
          upper.m_lower_values[6]=lower.m_upper_values[14]; //0.3125->-0.375
          upper.m_lower_values[7]=lower.m_upper_values[13]; //0.1875->-0.625
          upper.m_lower_values[9]=lower.m_upper_values[15]; //0.4375->-0.125
          upper.m_upper_values[9]=lower.m_upper_values[16]; //0.5625->+0.125
          upper.m_upper_values[10]=lower.m_upper_values[17]; //0.84375->+0.6875
          upper.m_upper_values[11]=lower.m_upper_values[18]; //0.90625->+0.8125
          upper.m_upper_values[12]=lower.m_upper_values[19]; //0.9921875->+0.984375

          // rearrange the values in lower
          lower.m_upper_values[6]=lower.m_lower_values[14]; //0.3125->-0.375
          lower.m_upper_values[7]=lower.m_lower_values[13]; //0.1875->-0.625
          lower.m_upper_values[9]=lower.m_lower_values[15]; //0.4375->-0.125
          lower.m_lower_values[9]=lower.m_lower_values[16]; //0.5625->+0.125
          lower.m_lower_values[10]=lower.m_lower_values[17]; //0.84375->+0.6875
          lower.m_lower_values[11]=lower.m_lower_values[18]; //0.90625->+0.8125
          lower.m_lower_values[12]=lower.m_lower_values[19]; //0.9921875->+0.984375
        }



        // bisect the per interval storage
        /*
          @param[in,out] lower on input, the interval to be bisected, on output
                         the lower half of the bisected range
          @param[out]    upper the upper half of the bisected range
         */
        template <typename Domain, typename Image>
        void bisect(rms_interval<19, 13, Domain, Image>& lower,
                    rms_interval<19, 13, Domain, Image>& upper)
        {
          BOOST_QUADRATURE_ASSERT(!detail::isnan(lower.m_lower_values));
          BOOST_QUADRATURE_ASSERT(!detail::isnan(lower.m_upper_values));

          bisect19(lower, upper);

#ifdef BOOST_QUADRATURE_DEBUG
          typedef typename scalar_component<Image>::type scalar_type;
          if (std::numeric_limits<scalar_type>::has_quiet_NaN)
          {
              scalar_type nan=std::numeric_limits<scalar_type>::quiet_NaN();
              assign(upper.m_upper_values[6],nan);
              assign(upper.m_upper_values[7],nan);
              assign(upper.m_upper_values[8],nan);

              assign(upper.m_lower_values[2],nan);
              assign(upper.m_lower_values[3],nan);
              assign(upper.m_lower_values[4],nan);
              assign(upper.m_lower_values[6],nan);
              assign(upper.m_lower_values[7],nan);
              assign(upper.m_lower_values[8],nan);

              assign(lower.m_lower_values[6],nan);
              assign(lower.m_lower_values[7],nan);
              assign(lower.m_lower_values[8],nan);

              assign(lower.m_upper_values[2],nan);
              assign(lower.m_upper_values[3],nan);
              assign(lower.m_upper_values[4],nan);
              assign(lower.m_upper_values[6],nan);
              assign(lower.m_upper_values[7],nan);
              assign(lower.m_upper_values[8],nan);

          }
#endif
        }

        template <typename Domain, typename Image>
        void bisect(rms_interval<27, 19, Domain, Image>& lower,
                    rms_interval<27, 19, Domain, Image>& upper)
        {
          BOOST_QUADRATURE_ASSERT(!detail::isnan(lower.m_lower_values));
          BOOST_QUADRATURE_ASSERT(!detail::isnan(lower.m_upper_values));

          bisect19(lower, upper);
          bisect27(lower, upper);

#ifdef BOOST_QUADRATURE_DEBUG
          typedef typename scalar_component<Image>::type scalar_type;
          if (std::numeric_limits<scalar_type>::has_quiet_NaN)
          {
              scalar_type nan=std::numeric_limits<scalar_type>::quiet_NaN();
              assign(upper.m_upper_values[9],nan);
              assign(upper.m_upper_values[10],nan);
              assign(upper.m_upper_values[11],nan);
              assign(upper.m_upper_values[12],nan);

              assign(upper.m_lower_values[3],nan);
              assign(upper.m_lower_values[4],nan);
              assign(upper.m_lower_values[6],nan);
              assign(upper.m_lower_values[7],nan);
              assign(upper.m_lower_values[8],nan);
              assign(upper.m_lower_values[9],nan);
              assign(upper.m_lower_values[10],nan);
              assign(upper.m_lower_values[11],nan);
              assign(upper.m_lower_values[12],nan);

              assign(lower.m_lower_values[9],nan);
              assign(lower.m_lower_values[10],nan);
              assign(lower.m_lower_values[11],nan);
              assign(lower.m_lower_values[12],nan);

              assign(lower.m_upper_values[3],nan);
              assign(lower.m_upper_values[4],nan);
              assign(lower.m_upper_values[6],nan);
              assign(lower.m_upper_values[7],nan);
              assign(lower.m_upper_values[8],nan);
              assign(lower.m_upper_values[9],nan);
              assign(lower.m_upper_values[10],nan);
              assign(lower.m_upper_values[11],nan);
              assign(lower.m_upper_values[12],nan);
          }
#endif
        }

        template <typename Domain, typename Image>
        void bisect(rms_interval<41, 27, Domain, Image>& lower,
                    rms_interval<41, 27, Domain, Image>& upper)
        {
          BOOST_QUADRATURE_ASSERT(!detail::isnan(lower.m_lower_values));
          BOOST_QUADRATURE_ASSERT(!detail::isnan(lower.m_upper_values));

          bisect19(lower, upper);
          bisect27(lower, upper);
          bisect41(lower, upper);

#ifdef BOOST_QUADRATURE_DEBUG
          typedef typename scalar_component<Image>::type scalar_type;
          if (std::numeric_limits<scalar_type>::has_quiet_NaN)
          {
              scalar_type nan=std::numeric_limits<scalar_type>::quiet_NaN();
              assign(upper.m_upper_values[13],nan);
              assign(upper.m_upper_values[14],nan);
              assign(upper.m_upper_values[15],nan);
              assign(upper.m_upper_values[16],nan);
              assign(upper.m_upper_values[17],nan);
              assign(upper.m_upper_values[18],nan);
              assign(upper.m_upper_values[19],nan);

              assign(upper.m_lower_values[3],nan);
              assign(upper.m_lower_values[4],nan);
              assign(upper.m_lower_values[8],nan);
              assign(upper.m_lower_values[10],nan);
              assign(upper.m_lower_values[11],nan);
              assign(upper.m_lower_values[12],nan);
              assign(upper.m_lower_values[13],nan);
              assign(upper.m_lower_values[14],nan);
              assign(upper.m_lower_values[15],nan);
              assign(upper.m_lower_values[16],nan);
              assign(upper.m_lower_values[17],nan);
              assign(upper.m_lower_values[18],nan);
              assign(upper.m_lower_values[19],nan);

              assign(lower.m_lower_values[13],nan);
              assign(lower.m_lower_values[14],nan);
              assign(lower.m_lower_values[15],nan);
              assign(lower.m_lower_values[16],nan);
              assign(lower.m_lower_values[17],nan);
              assign(lower.m_lower_values[18],nan);
              assign(lower.m_lower_values[19],nan);

              assign(lower.m_upper_values[3],nan);
              assign(lower.m_upper_values[4],nan);
              assign(lower.m_upper_values[8],nan);
              assign(lower.m_upper_values[10],nan);
              assign(lower.m_upper_values[11],nan);
              assign(lower.m_upper_values[12],nan);
              assign(lower.m_upper_values[13],nan);
              assign(lower.m_upper_values[14],nan);
              assign(lower.m_upper_values[15],nan);
              assign(lower.m_upper_values[16],nan);
              assign(lower.m_upper_values[17],nan);
              assign(lower.m_upper_values[18],nan);
              assign(lower.m_upper_values[19],nan);
          }
#endif
        }


        // N is assumed odd, so coefficients become symmetric
        template <
            std::size_t N, std::size_t M,
            typename Integrand,
            typename Domain,
            typename Image,
            typename ErrorEstimator,
            typename Recorder>
        quadrature_error rms(
          const Integrand& f, Domain a, Domain b,
          Image& result,
          ErrorEstimator& err,
          Recorder& recorder,
          detail::rms_interval<N,M,Domain,
            typename storage_for_type<Image>::type>& interval
                             )
        {
          typedef typename detail::scalar_component<Image>::type value_type;
          typedef typename detail::storage_for_type<Image>::type image_type;
          typedef typename detail::storage_for_type<Domain>::type domain_type;

          const typename rms_coefficients<value_type,N>::value_type& kcw
            =rms_coefficients<value_type,N>::weights();
          const typename rms_coefficients<domain_type,N>::value_type& kcx
            =rms_coefficients<domain_type,N>::abcissa();
          const typename rms_coefficients<value_type,M>::value_type& gcw
            =rms_coefficients<value_type,M>::weights();
          const typename rms_coefficients<domain_type,M>::value_type& gcx
            =rms_coefficients<domain_type,M>::abcissa();

          const typename rms_coefficients<value_type,N>::flags_type&
            crossed_flags=rms_coefficients<value_type,N>::crossed_flags();
          const typename rms_coefficients<value_type,N>::flags_type&
            bisected_flags=rms_coefficients<value_type,N>::bisected_flags();



          // mid point of the interval
          domain_type center = 0.5*(a+b);
          // half-length of the interval
          domain_type half_length = 0.5*(b-a);

          // compute the 15-point kronrod approximation to the integral, and
          // estimate the absolute error.
          if (interval.m_lower_status==detail::empty)
            eval(f,center,interval.m_center,recorder);

          BOOST_QUADRATURE_ASSERT(!detail::isnan(interval.m_center));

          // resg -result of the M-point rms formula
          image_type resg;
          detail::multiply(resg/* = */,2*gcw[0] /* * */,interval.m_center);
          // resk - result of the N-point rms formula
          image_type resk;
          detail::multiply(resk/* = */,2*kcw[0] /* * */,interval.m_center);

          err.center_point(2*kcw[0],interval.m_center);

          for (std::size_t j=1; j<=M/2; ++j)
          {
            BOOST_ASSERT(kcx[j]==gcx[j]);
            std::size_t jm1=j-1;

            domain_type absc = half_length*kcx[j];

            if (interval.m_lower_status==detail::empty
                || ( interval.m_lower_status==detail::crossed
                     && !crossed_flags[jm1])
                || ( interval.m_lower_status==detail::bisected
                     && !bisected_flags[jm1]))
            {
              image_type fval1;
              eval(f,center-absc,fval1,recorder);
              detail::assign(interval.m_lower_values[jm1],fval1);
            }

            if (interval.m_upper_status==detail::empty
                || ( interval.m_upper_status==detail::crossed
                     && !crossed_flags[jm1])
                || ( interval.m_upper_status==detail::bisected
                     && !bisected_flags[jm1]))
            {
              image_type fval2;
              eval(f,center+absc,fval2,recorder);
              detail::assign(interval.m_upper_values[jm1],fval2);
            }

            image_type fsum;
            detail::sum(fsum,
                        interval.m_lower_values[jm1],
                        interval.m_upper_values[jm1]);
            detail::accumulate_weighted(resg,gcw[j],fsum);
            detail::accumulate_weighted(resk,kcw[j],fsum);

            err.point(kcw[j],
                      interval.m_lower_values[jm1],
                      interval.m_upper_values[jm1]);
          }

          for (std::size_t j=M/2+1; j<=N/2; ++j)
          {
            std::size_t jm1=j-1;
            domain_type absc = half_length*kcx[j];

            if (interval.m_lower_status==detail::empty
                || ( interval.m_lower_status==detail::crossed
                     && !crossed_flags[jm1])
                || ( interval.m_lower_status==detail::bisected
                     && !bisected_flags[jm1]))
            {
              image_type fval1;
              eval(f,center-absc,fval1,recorder);
              detail::assign(interval.m_lower_values[jm1],fval1);
            }

            if (interval.m_upper_status==detail::empty
                || ( interval.m_upper_status==detail::crossed
                     && !crossed_flags[jm1])
                || ( interval.m_upper_status==detail::bisected
                     && !bisected_flags[jm1]))
            {
              image_type fval2;
              eval(f,center+absc,fval2,recorder);
              detail::assign(interval.m_upper_values[jm1],fval2);
            }

            image_type fsum;
            detail::sum(fsum,
                        interval.m_lower_values[jm1],
                        interval.m_upper_values[jm1]);
            detail::accumulate_weighted(resk,kcw[j],fsum);
            err.point(kcw[j],
                      interval.m_lower_values[jm1],
                      interval.m_upper_values[jm1]);
          }

          err(
            resk,resg,
            interval.m_center,
            interval.m_lower_values,
            interval.m_upper_values,
            2*kcw[0],
            boost::iterator_range< typename rms_coefficients<value_type,N>::value_type::const_iterator>(boost::next(kcw.begin()),kcw.end()),
            half_length);

          detail::multiply(result,half_length,resk);

          return quadrature_ok;
        }

        template <typename RMS, typename T>
        struct kernel_data_
        {
          template <typename Domain, typename Range, typename V>
          static T& f(detail::arg_ref<V>& v
                   BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE(Domain)
                   BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE(Range))
          {
            return v.m_ref;
          }

          template <typename Domain, typename Range>
          struct apply
          {
            typedef T& type;
          };

        };

        template <typename RMS>
        struct kernel_data_<RMS,detail::defarg >
        {
          template <typename Domain, typename Range>
          static typename detail::kernel_interval<RMS,Domain,Range>::type
          f(detail::arg_ref<detail::defarg>
                BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE(Domain)
                BOOST_MATH_APPEND_EXPLICIT_TEMPLATE_TYPE(Range))
          {
            typedef typename detail::kernel_interval<RMS,Domain,Range>::type t;
            return t();
          }

          template <typename Domain, typename Range>
          struct apply
          {
            typedef typename detail::kernel_interval<RMS,Domain,Range>::type
            type;
          };

        };

      } // namespace



      //! N,M Recursive Monotone Stable quadrature
      /*! N,M Recursive Monotone Stable quadrature.
        N and M are assumed odd, so coefficients become symmetric.

        rms is defined for rms<19,13>, rms<27,19> and rms<41,27>.
      */
      template <
          std::size_t N, std::size_t M,
          typename ErrorEst,
          typename Recorder,
          typename Data>
      class rms
      {
      public:
        rms(
          BOOST_QUADRATURE_CONSTRUCTOR_REF_ARG(error_estimator, ErrorEst),
          BOOST_QUADRATURE_CONSTRUCTOR_REF_ARG(recorder, Recorder),
          BOOST_QUADRATURE_CONSTRUCTOR_REF_ARG(kernel_data, Data)
                      )
            : m_error_estimator(error_estimator),
              m_recorder(recorder),
              m_kernel_data(kernel_data)
        {}

        //! integrate functor over specified limits
        /*!
          @param integrand   the integrand to be integrated
          @param lower_limit the lower limit of integration
          @param upper_limit the upper limit of integration
          @param result      the approcimation of the integral
          @result error code
         */
        template <typename F, typename DL, typename DU, typename R>
        quadrature_error operator()(
          F integrand,DL lower_limit, DU upper_limit, R& result) const
        {
          typedef detail::kernel_data_<rms<N,M,ErrorEst,Recorder,Data>,Data> k;
          typedef typename boost::math::tools::promote_args<DL,DU>::type domain;
          typedef typename detail::storage_for_type<R>::type image;

          typename k::template apply<domain,image>::type var=
            k::template f<domain,image>(m_kernel_data);

          // instantiate any defaults
          return ::boost::numeric::quadrature::detail::rms<N,M>(
            integrand,
            detail::promote(lower_limit, upper_limit), //promote upper and lower
            detail::promote(upper_limit, lower_limit), //limits to same type
            result,
            error_estimator_(m_error_estimator),
            recorder_(m_recorder),
            var
                                                                );
        }

        //! suppy an error estimator
        /*!
          @param error_estimator   the estimator to be used
          @result an rms integrator object, which includes the estimator
         */
        template <typename ErrorEst_>
        rms<N,M,ErrorEst_,Recorder,Data>
        error_estimator(ErrorEst_& error_estimator) const
        {
          return rms<N,M,ErrorEst_,Recorder,Data>(
            detail::arg_ref<ErrorEst_>(error_estimator),
            m_recorder,
            m_kernel_data);
        }

        //! suppy a recorder
        /*!
          @param recorder   the recorder to be used
          @result an rms integrator object, which includes the recorder
         */
        template <typename Recorder_>
        rms<N,M,ErrorEst,Recorder_,Data>
        recorder(Recorder_& recorder) const
        {
          return rms<N,M,ErrorEst,Recorder_,Data>(
            m_error_estimator,
            detail::arg_ref<Recorder_>(recorder),
            m_kernel_data);
        }

        //! suppy kernel data
        /*!
          @param kernel_data   the kernel_data to be used
          @result an rms integrator object, which includes the kernel data
         */
        template <typename Data_>
        rms<N,M,ErrorEst,Recorder,Data_>
        kernel_data(Data_& kernel_data) const
        {
          return rms<N,M,ErrorEst,Recorder,Data_>(
              m_error_estimator,
              m_recorder,
              detail::arg_ref<Data_>(kernel_data));
        }

      private:
        BOOST_QUADRATURE_REF_ARG(
          error_estimator, ErrorEst, detail::null_error_estimator,
          detail::null_error_estimator::instance())
        BOOST_QUADRATURE_REF_ARG(
          recorder, Recorder, detail::null_recorder,
          detail::null_recorder::instance())

        mutable detail::arg_ref<Data> m_kernel_data;
      };


    }// namespace
  }// namespace
}// namespace

#endif


