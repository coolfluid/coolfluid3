#ifndef BOOST_NUMERIC_QUADRATURE_ERROR_ESTIMATOR_H
#define BOOST_NUMERIC_QUADRATURE_ERROR_ESTIMATOR_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! Error estimator
  @file   error_estimator.hpp
  @brief  Class for estimating the integration error
*/

#include "boost/numeric/quadrature/quadrature_config.hpp"
#include "boost/numeric/quadrature/quadrature_traits.hpp"
#include "boost/numeric/quadrature/detail/local_traits.hpp"
#include "boost/math/tools/precision.hpp"

#include <cmath>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        template <typename Value, typename Value2>
        void assign_weighted_abs(
          Value& out, Value2 weight, Value2 in,
          typename boost::enable_if< is_arithmetic_scalar<Value> >::type*v=0)
        {
          using namespace std;
          out=weight*(abs)(in);
        }

        template <typename OutputRange, typename Value, typename InputRange>
        void assign_weighted_abs(
          OutputRange& output,  Value weight, InputRange input,
          typename boost::disable_if<
          boost::mpl::or_<quadrature::is_arithmetic_scalar<InputRange>,
          quadrature::is_arithmetic_scalar<OutputRange> > >::type*v=0)
        {
          using namespace std;
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<OutputRange>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          for (; in!=in_end; ++out, ++in)
            *out=weight*(abs)(*in);
        }

         template <typename Value>
         void output(
           const Value& out,
           typename boost::enable_if< is_arithmetic_scalar<Value> >::type*v=0)
         {
           std::cout << std::setprecision(15);
           std::cout << out << std::endl;
         }

         template <typename OutputRange>
         void output(
           const OutputRange& output,
           typename boost::disable_if<is_arithmetic_scalar<OutputRange> >::type*v=0)
         {
           std::cout << std::setprecision(15);
           typename boost::range_const_iterator<OutputRange>::type
             out=boost::begin(output),
             out_end=boost::end(output);
           for (; out!=out_end; ++out)
             std::cout << *out << std::endl;
         }


        template <typename Value>
        void error_estimate(
          Value& abserr, Value resasc,
          typename boost::enable_if<is_arithmetic_scalar<Value> >::type*v=0 )
        {
          using namespace std;
          if (resasc!=0.0 && abserr!=0)
            abserr= resasc*(min)(
              static_cast<Value>(1.),
              (pow)(static_cast<Value>(200.)*abserr/resasc,
                    static_cast<Value>(1.5)));
        }

        template <typename OutputRange, typename InputRange>
        void error_estimate(
          OutputRange& abserr, InputRange& resasc,
          typename boost::disable_if<
          is_arithmetic_scalar<InputRange> >::type*v=0)
        {
          using namespace std;
          typedef typename boost::range_value<OutputRange>::type value_type;

          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(abserr);
          typename boost::range_const_iterator<OutputRange>::type
            in=boost::begin(resasc),
            in_end=boost::end(resasc);
          for (; in!=in_end; ++out, ++in)
            if (*out!=0 && *in!=0)
              *out=*in*(min)(
                static_cast<value_type>(1.),
                (pow)(static_cast<value_type>(200.)*(*out)/(*in),
                      static_cast<value_type>(1.5)));
        }


        template <typename Value, typename MinErr>
        void error_estimate2(
          Value& abserr, Value resabs, MinErr minerr,
          typename enable_if<is_arithmetic_scalar<Value> >::type*v=0)
        {
          using namespace std;
          // smallest positive magnitude
          static const Value uflow = boost::math::tools::min_value<Value>();
          if (resabs>uflow/minerr)
            abserr = (max)(minerr*resabs,abserr);
        }

        template <typename OutputRange, typename InputRange, typename MinErr>
        void error_estimate2(
          OutputRange& abserr, InputRange& resasc, MinErr minerr,
          typename disable_if<is_arithmetic_scalar<InputRange> >::type*v=0)
        {
          using namespace std;
          typedef typename boost::range_value<OutputRange>::type vtype;

          // smallest positive magnitude
          static const vtype uflow = boost::math::tools::min_value<vtype>();
          static const vtype errmin = uflow/minerr;

          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(abserr);
          typename boost::range_const_iterator<OutputRange>::type
            in=boost::begin(resasc),
            in_end=boost::end(resasc);
          for (; in!=in_end; ++out, ++in)
            if (*in>errmin)
              *out = (max)(minerr*(*in),*out);
        }


        template <typename Value, typename MinErr>
        void roundoff(
          bool& roundoff, bool& residual_roundoff,
          Value abserr, Value resabs, Value resasc,
          MinErr minerr,
          typename boost::enable_if<is_arithmetic_scalar<Value> >::type*v=0 )
        {
          using namespace std;
          roundoff=(abserr<=minerr*resabs);
          residual_roundoff=(abserr!=resasc);
        }

        template <typename FlagRange,
            typename InputRange1, typename InputRange2,
            typename InputRange3, typename MinErr>
        void roundoff(
          FlagRange& roundoff, FlagRange& residual_roundoff,
          const InputRange1& abserr, const InputRange2& resabs,
          const InputRange3& resasc,
          MinErr minerr,
          typename boost::disable_if<is_arithmetic_scalar<InputRange1> >::type*v=0)
        {
          using namespace std;

          typename boost::range_mutable_iterator<FlagRange>::type
            roff=boost::begin(roundoff),
            roff2=boost::begin(residual_roundoff);
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(abserr),
            in_end=boost::end(abserr);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(resabs);
          typename boost::range_const_iterator<InputRange3>::type
            in3=boost::begin(resasc);

          for (; in!=in_end; ++in, ++in2, ++in3, ++roff, ++roff2)
          {
            *roff=(*in<=minerr*(*in2));
            *roff2=(*in!=*in3);
          }
        }





        template <typename Value>
        bool roundoff_in_area(
          Value largest_error_area, Value largest_error,
          Value interval_area, Value interval_error,
          Value dummy)
        {
          using namespace std;
          return
            (abs(largest_error_area-interval_area)<=1e-05*abs(interval_area))
            && (interval_error>=0.99*largest_error);
        }

        template <
            typename InputRange1, typename InputRange2,
            typename InputRange3, typename InputRange4,
            typename Value>
        bool roundoff_in_area(
          InputRange1 largest_error_area, InputRange2 largest_error,
          InputRange3 interval_area, InputRange4 interval_error,
          Value dummy)
        {
          using namespace std;
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(largest_error_area),
            in_end=boost::end(largest_error_area);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(largest_error);
          typename boost::range_const_iterator<InputRange3>::type
            in3=boost::begin(interval_area);
          typename boost::range_const_iterator<InputRange4>::type
            in4=boost::begin(interval_error);

          for (; in!=in_end; ++in, ++in2, ++in3, ++in4)
            if ( (abs(*in-*in3)<=1e-05*abs(*in3))
                 && (*in4>=0.99*(*in2)))
              return true;
          return false;
        }




      }


      //! Error estimation
      /*! Error estimation is for the integral I(x) of f(x), is based
        on the integral of abs(f(x)) and on the integral of I(x)-f(x).
        Adapted from quadpack.

        Note that the calls to center_point and to point could have
        been computed in the call to operator(), but that was found to
        give different rounnding behaviour in the coefficients.
      */
      template <typename Domain, typename Image>
      struct error_estimator
      {
        typedef typename detail::storage_for_type<Image>::type image_type;
        typedef typename detail::storage_for_type<Domain>::type domain_type;
        typedef typename detail::scalar_component<image_type>::type scalar_image_type;
        typedef typename detail::storage_for_type<image_type,bool>::type flag_type;

        //! the minimum relative error that will be returned
        static scalar_image_type minimum_relative_error()
        {
          static scalar_image_type minerr
            =50.*boost::math::tools::epsilon<scalar_image_type>();
          return minerr;
        }

        //! Initialise the computation of integral(abs(integrand))
        /*!
          @param center_weight weight at centre of interval
          @param center_value  value of integrand at centre of interval
        */
        template <typename Weight>
        void center_point(
          Weight center_weight,
          typename call_traits<image_type>::param_type center_value)
        {
          detail::assign_weighted_abs(resabs, center_weight, center_value);
        }

        //! At a +/- point to the computation of integral(abs(integrand))
        /*!
          @param weight weight at centre of interval
          @param lower  value of integrand at lower offset of interval
          @param upper  value of integrand at upper offset of interval
        */
        template <typename Weight>
        void point(
          Weight weight,
          typename call_traits<image_type>::param_type lower,
          typename call_traits<image_type>::param_type upper)
        {
          detail::acc_weighted_sum_abs(resabs,weight,lower,upper);
        }

        //! Estimate the error, and check for roundoff
        /*!
          @param resk
          @param resg
          @param center_value
          @param lower_values
          @param upper_values
          @param center_weight
          @param weights
          @param half_length
        */
        template <typename ImagePoints, typename Weight, typename WeightRange>
        void operator()(
          typename call_traits<image_type>::param_type resk,
          typename call_traits<image_type>::param_type resg,
          typename call_traits<image_type>::param_type center_value,
          const ImagePoints& lower_values,
          const ImagePoints& upper_values,
          Weight center_weight,
          const WeightRange& weights,
          Domain half_length
                                    )
        {
          BOOST_ASSERT(boost::size(lower_values)==boost::size(upper_values));
          BOOST_ASSERT(boost::size(lower_values)==boost::size(weights));
          using namespace std;

          typedef typename boost::range_const_iterator<ImagePoints>::type
            point_iterator;
          typedef typename boost::range_const_iterator<WeightRange>::type
            weight_iterator;

          typedef typename detail::scalar_component<image_type>::type vtype;

          image_type reskh;
          detail::multiply(reskh, static_cast<vtype>(0.5),resk);

          detail::multiply_abs_difference(
            resasc, center_weight, center_value, reskh);

          point_iterator
            l=boost::begin(lower_values),
            l_end=boost::end(lower_values),
            u=boost::begin(upper_values);
          weight_iterator w=boost::begin(weights);

          for (;l!=l_end; ++l, ++u, ++w)
            detail::acc_mult_sum_abs_diff(resasc,*w,*l,*u,reskh);

          domain_type dhalf_length = (std::abs)(half_length);
          detail::multiply(resabs,dhalf_length);
          detail::multiply(resasc,dhalf_length);

          //detail::output(resasc);

          detail::abs_mult_diff(abserr,half_length,resk,resg);
          detail::error_estimate(abserr,resasc);
          detail::error_estimate2(abserr, resabs, minimum_relative_error());

          // check for roundoff problems
          detail::roundoff(m_roundoff,m_residual_roundoff,abserr,resabs,resasc,
                           minimum_relative_error());
        }

        //! Return the esitmate of the absolute error
        const image_type& absolute_error() const
        {
          return abserr;
        }

        const image_type& integral_abs() const
        {
          return resabs;
        }

        typename call_traits<flag_type>::const_reference
        residual_roundoff() const
        {
          return m_residual_roundoff;
        }

        //! Flag for detection of roundoff in the integral estimate
        typename call_traits<flag_type>::const_reference
        roundoff() const
        {
          return m_roundoff;
        }

        image_type abserr;
        image_type resabs;
        image_type resasc;

        flag_type m_residual_roundoff,m_roundoff;
      };


    }// namespace
  }// namespace
}// namespace

#endif
