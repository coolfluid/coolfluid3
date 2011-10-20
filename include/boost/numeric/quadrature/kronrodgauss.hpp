#ifndef BOOST_NUMERIC_QUADRATURE_KRONRODGAUSS_H
#define BOOST_NUMERIC_QUADRATURE_KRONRODGAUSS_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! Kronrod-Gauss Integration
  @file   kronrodgauss.hpp
  @brief  class for performing N point kronrod-gauss quadrature
*/

#include "boost/numeric/quadrature/quadrature_config.hpp"
#include "boost/numeric/quadrature/kronrodgauss_fwd.hpp"
#include "boost/numeric/quadrature/gauss_coefficients.hpp"
#include "boost/numeric/quadrature/kronrod_coefficients.hpp"
#include "boost/numeric/quadrature/quadrature_error.hpp"
#include "boost/numeric/quadrature/quadrature_traits.hpp"

#include "boost/numeric/quadrature/detail/null_interval_kernel_space.hpp"
#include "boost/numeric/quadrature/detail/null_error_estimator.hpp"
#include "boost/numeric/quadrature/detail/null_recorder.hpp"
#include "boost/numeric/quadrature/detail/arg_chaining.hpp"
#include "boost/numeric/quadrature/detail/local_traits.hpp"
#include <boost/numeric/quadrature/promotion.hpp>

#include <boost/range.hpp>

#include <cmath>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {

        // N is assumed odd, so coefficients become symmetric
        template <
            std::size_t N,
            typename Integrand,
            typename Domain,
            typename Image,
            typename ErrorEstimator,
            typename Recorder>
        quadrature_error kronrod_gauss(
          const Integrand& f,
          Domain a, Domain b,
          Image& result,
          ErrorEstimator& err,
          Recorder& recorder,
          detail::null_interval_kernel_space& interval
                                                      )
        {
          BOOST_STATIC_ASSERT(N%2==1);
          typedef typename detail::scalar_component<Image>::type value_type;
          typedef typename detail::storage_for_type<Image>::type image_type;
          typedef typename detail::storage_for_type<Domain>::type domain_type;

          const typename kronrod_coefficients<value_type,N>::value_type& kcw
            =kronrod_coefficients<value_type,N>::weights();
          const typename kronrod_coefficients<domain_type,N>::value_type& x
            =kronrod_coefficients<domain_type,N>::abcissa();
          const typename gauss_coefficients<value_type,N/2>::value_type& gcw
            =gauss_coefficients<value_type,N/2>::weights();


          image_type fv1[N/2],fv2[N/2];
          // mid point of the interval
          domain_type center = 0.5*(a+b);
          // half-length of the interval
          domain_type half_length = 0.5*(b-a);

          // compute the 15-point kronrod approximation to the integral, and
          // estimate the absolute error.
          image_type fc;
          eval(f,center,fc,recorder);

          // resg -result of the N/2-point gauss formula
          image_type resg;
          std::size_t n2;
          if (N/2%2==1)
          {
            detail::multiply(resg/* = */,gcw[N/2/2] /* * */,fc);
            n2=N/2/2+1;
          }
          else
          {
            detail::assign(resg,0.);
            n2=N/2/2;
          }

          // resk - result of the N-point kronrod formula
          image_type resk;
          detail::multiply(resk/* = */,kcw[N/2] /* * */,fc);

          err.center_point(kcw[N/2],fc);

          for (std::size_t j=0; j<N/2/2; ++j)
          {
            std::size_t jtw = j*2+1;
            domain_type absc = half_length*x[jtw];
            image_type fval1;
            image_type fval2;
            eval(f,center-absc,fval1,recorder);
            eval(f,center+absc,fval2,recorder);

            detail::assign(fv1[jtw],fval1);
            detail::assign(fv2[jtw],fval2);
            image_type fsum;
            detail::sum(fsum,fval1,fval2);
            detail::accumulate_weighted(resg,gcw[j],fsum);
            detail::accumulate_weighted(resk,kcw[jtw],fsum);
            err.point(kcw[jtw],fval1,fval2);
          }

          for (std::size_t j=0; j<n2; ++j)
          {
            std::size_t jtwm1 = j*2;
            domain_type absc = half_length*x[jtwm1];
            image_type fval1;
            image_type fval2;
            eval(f,center-absc,fval1,recorder);
            eval(f,center+absc,fval2,recorder);
            detail::assign(fv1[jtwm1],fval1);
            detail::assign(fv2[jtwm1],fval2);

            image_type fsum;
            detail::sum(fsum,fval1,fval2);
            detail::accumulate_weighted(resk,kcw[jtwm1],fsum);
            err.point(kcw[jtwm1],fval1,fval2);
          }

          err(
            resk,resg,fc,fv1,fv2,kcw[N/2],
            boost::iterator_range< typename kronrod_coefficients<value_type,N>::value_type::const_iterator>(kcw.begin(),boost::prior(kcw.end())),
            half_length);

          detail::multiply(result,half_length,resk);

          return quadrature_ok;
        }

      } //namespace



      //! N point Kronrod-Gauss quadrature
      /*! N point Kronrod quadrature with N/2 point Gauss interpolation.
        N is assumed odd, so coefficients become symmetric.
        See http://en.wikipedia.org/wiki/Gaussian_quadrature.

        Adapted from QUADPACK, http://www.netlib.org/quadpack.

        N is defined for 15, 21, 31, 41 and 51.
      */
      template <std::size_t N,
          typename ErrorEst, typename Recorder, typename Data>
      class kronrod_gauss
      {
      public:
        kronrod_gauss(
          BOOST_QUADRATURE_CONSTRUCTOR_REF_ARG(error_estimator, ErrorEst),
          BOOST_QUADRATURE_CONSTRUCTOR_REF_ARG(recorder, Recorder),
          BOOST_QUADRATURE_CONSTRUCTOR_REF_ARG(kernel_data, Data)
                      )
            : m_error_estimator(error_estimator),
              m_recorder(recorder),
              m_kernel_data(kernel_data)
        {}

        template <typename F, typename DL, typename DU, typename R>
        quadrature_error operator()(
          F integrand,DL lower_limit, DU upper_limit, R& result) const
        {
          // instantiate any defaults
          return ::boost::numeric::quadrature::detail::kronrod_gauss<N>(
            integrand,
            detail::promote(lower_limit, upper_limit), //promote upper and lower
            detail::promote(upper_limit, lower_limit), //limits to same type
            result,
            error_estimator_(m_error_estimator),
            recorder_(m_recorder),
            kernel_data_(m_kernel_data));
        }


        template <typename ErrorEst_>
        kronrod_gauss<N,ErrorEst_,Recorder,Data>
        error_estimator(ErrorEst_& error_estimator) const
        {
          return kronrod_gauss<N,ErrorEst_,Recorder,Data>(
            detail::arg_ref<ErrorEst_>(error_estimator),
            m_recorder,
            m_kernel_data);
        }

        template <typename Recorder_>
        kronrod_gauss<N,ErrorEst,Recorder_,Data>
        recorder(Recorder_& recorder) const
        {
          return kronrod_gauss<N,ErrorEst,Recorder_,Data>(
            m_error_estimator,
            detail::arg_ref<Recorder_>(recorder),
            m_kernel_data);
        }

        template <typename Data_>
        kronrod_gauss<N,ErrorEst,Recorder,Data_>
        kernel_data(Data_& kernel_data) const
        {
          return kronrod_gauss<N,ErrorEst,Recorder,Data_>(
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
        BOOST_QUADRATURE_REF_ARG(
          kernel_data, Data, detail::null_interval_kernel_space,
          detail::null_interval_kernel_space::instance())

      };

    }// namespace
  }// namespace
}// namespace

#endif
