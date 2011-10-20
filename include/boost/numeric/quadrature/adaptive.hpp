#ifndef BOOST_NUMERIC_QUADRATURE_ADAPTIVE_H
#define BOOST_NUMERIC_QUADRATURE_ADAPTIVE_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   adaptive.hpp
  @brief  Globally adaptive quadrature routines
*/

#include <boost/numeric/quadrature/quadrature_config.hpp>
#include <boost/numeric/quadrature/quadrature_error.hpp>
#include <boost/numeric/quadrature/quadrature_traits.hpp>
#include <boost/numeric/quadrature/detail/null_recorder.hpp>
#include <boost/numeric/quadrature/detail/local_traits.hpp>
#include <boost/numeric/quadrature/detail/arg_chaining.hpp>
#include <boost/numeric/quadrature/interval.hpp>
#include <boost/numeric/quadrature/kronrodgauss.hpp>
#include <boost/numeric/quadrature/promotion.hpp>
#include <boost/numeric/quadrature/error_estimator.hpp>

#include <boost/range.hpp>
#include <boost/current_function.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/type_traits/is_same.hpp>

#include <list>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      /*! Info structure for recording information about the adaptive function.
       */
      class adaptive_info
      {
      public:
        /*! Reset the number of evaluations and intervals
         */
        void reset()
        {
          m_num_kernel_evaluations=0;
          m_num_intervals=0;
        }

        //! register a kernel evaluation
        void kernel_evaluation()
        {
          ++m_num_kernel_evaluations;
        }

        //! register several kernel evaluations
        /* @param n number of kernel evaluations to add
         */
        void kernel_evaluation(std::size_t n)
        {
          m_num_kernel_evaluations+=n;
        }

        //! set the number of intervals used
        /* @param n number of intervals
         */
        void number_of_intervals(std::size_t n)
        {
          m_num_intervals=n;
        }

        /*! Number of kernal evaluations used by the integration
         */
        std::size_t num_kernel_evaluations() const
        {
          return m_num_kernel_evaluations;
        }

        /*! number of intervals used by the integration
         */
        std::size_t num_intervals() const
        {
          return m_num_intervals;
        }

      private:
        std::size_t m_num_kernel_evaluations;
        std::size_t m_num_intervals;
      };

      namespace detail
      {
        // Info structure that does nothing
        /* Used to provide a default object for routine requiring
           an Info value.
        */
        struct null_adaptive_info
        {
          void reset() const {}
          void kernel_evaluation() const {}
          void kernel_evaluation(std::size_t) const {}
          void number_of_intervals(std::size_t) const {}
          static null_adaptive_info& instance()
          {
            static null_adaptive_info info;
            return info;
          }
        };

        struct null_intervals
        {
          static null_intervals& instance()
          {
            static null_intervals info;
            return info;
          }
        };

        // Accelerator structure that does nothing
        /* Used to provide a default object for routine requiring
           an Accelerator value.
        */
        struct null_adaptive_accelerator {
          static null_adaptive_accelerator& instance()
          {
            static null_adaptive_accelerator accel;
            return accel;
          }
          template <typename V>
          bool operator()(const V&, V&, V&) const
          {
            return false;
          }

        };




        // test if roundoff occured and the error tolerance is not satisfied
        template <typename Flag, typename Value>
        bool check_roundoff(
          Flag roundoff, Value abserr, Value errbnd,
          typename enable_if<is_arithmetic_scalar<Value> >::type*v=0)
        {
          return roundoff && abserr>errbnd;
        }

        // test if roundoff occured and the error tolerance is not satisfied
        /* Returns true if found for any component.
           Not sure if this is correct/desirable behaviour
        */
        template <typename FlagRange, typename InputRange1,typename InputRange2>
        bool check_roundoff(
          const FlagRange& roundoff,
          const InputRange1& abserr, const InputRange2& errbnd,
          typename disable_if<is_arithmetic_scalar<InputRange1> >::type*v=0)
        {
          typename boost::range_const_iterator<FlagRange>::type
            roff=boost::begin(roundoff);
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(abserr),
            in_end=boost::end(abserr);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(errbnd);

          for (; in!=in_end; ++in, ++in2, ++roff)
            if (*roff && *in>*in2)
              return true;
          return false;
        }



        // test if roundoff occured and the error tolerance is not satisfied
        template <typename Flag, typename Value>
        bool check_residual_roundoff(
          Flag residual_roundoff, Value abserr, Value errbnd,
          typename enable_if<is_arithmetic_scalar<Value> >::type*v=0)
        {
          return abserr>errbnd || !residual_roundoff;
        }

        // test if roundoff occured and the error tolerance is not satisfied
        /* Returns true if true for any component.
        */
        template <typename FlagRange, typename InputRange1,typename InputRange2>
        bool check_residual_roundoff(
          const FlagRange& residual_roundoff,
          const InputRange1& abserr, const InputRange2& errbnd,
          typename disable_if<is_arithmetic_scalar<InputRange1> >::type*v=0)
        {
          typename boost::range_const_iterator<FlagRange>::type
            roff=boost::begin(residual_roundoff);
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(abserr),
            in_end=boost::end(abserr);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(errbnd);

          bool rv=false;
          for (; in!=in_end; ++in, ++in2, ++roff)
            rv= rv || (*in>*in2 || !*roff);
          return rv;
        }



        template <typename Value>
        bool divergent(
          Value largest_error, Value interval_error,
          typename enable_if<is_arithmetic_scalar<Value> >::type*v=0)
        {
          return interval_error>largest_error;
        }

        // test if any components are divergent
        template <typename InputRange1, typename InputRange2>
        bool divergent(
          const InputRange1& largest_error, const InputRange2& interval_error,
          typename disable_if<is_arithmetic_scalar<InputRange1> >::type*v=0)
        {
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(largest_error),
            in_end=boost::end(largest_error);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(interval_error);

          for (; in!=in_end; ++in, ++in2)
            if (*in2>*in)
              return true;
          return false;
        }


        template <typename Value>
        bool divergent2(
          Value abserr, Value errsum,
          typename enable_if<is_arithmetic_scalar<Value> >::type*v=0)
        {
          return abserr<.001*errsum;
        }

        template <typename InputRange1, typename InputRange2>
        bool divergent2(
          const InputRange1& abserr, const InputRange2& errsum,
          typename disable_if<is_arithmetic_scalar<InputRange1> >::type*v=0)
        {
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(abserr),
            in_end=boost::end(abserr);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(errsum);

          for (; in!=in_end; ++in, ++in2)
            if (*in<0.001*(*in2))
              return true;
          return false;
        }



        // absolute error bound based on absolute and relative tolerance
        /* @return true if error bound satisfied
        */
        template <typename Value>
        bool error_bound(Value& errbnd, Value epsabs, Value epsrel,
                         Value result, Value abserr)
        {
          using namespace std;
          errbnd = (max)(epsabs, epsrel*(abs)(result));
          return abserr<errbnd;
        }

        // absolute error bound based on absolute and relative tolerance
        /* @return true if error bound satisfied
        */
        template <typename OutputRange, typename Value,
            typename InputRange1, typename InputRange2>
        bool error_bound(OutputRange& errbnd, Value epsabs, Value epsrel,
                         const InputRange1& result, const InputRange2& abserr)
        {
          using namespace std;
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(result),
            in_end=boost::end(result);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(abserr);
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(errbnd);

          bool satisfied=true;
          for (; in!=in_end; ++in, ++in2, ++out)
          {
            *out = (max)(epsabs, epsrel*(abs)(*in));
            satisfied=satisfied && *in2<*out;
          }
          return satisfied;
        }



        template <
            typename F,
            typename Domain,
            typename Image,
            typename Q,
            typename IntervalStorageSize,
            typename IntervalStorage,
            typename Recorder,
            typename Info
            >
        quadrature_error adaptive(
          const F& f,
          Domain a, Domain b,
          Image& result,
          Image& abserr,
          typename scalar_component<Image>::type epsabs,
          typename scalar_component<Image>::type epsrel,
          const Q& integrator,
          IntervalStorageSize max_intervals,
          IntervalStorage& interval_storage,
          Recorder& recorder,
          Info& info,
          null_adaptive_accelerator&
          , typename boost::disable_if<
          boost::is_same<IntervalStorage,null_intervals> >::type*v=0
                                  )
        {
          using namespace std;

          typedef typename detail::storage_for_type<Image>::type image_type;
          typedef typename detail::storage_for_type<Domain>::type domain_type;
          typedef typename scalar_component<image_type>::type scalar_image_type;

          // largest relative spacing.
          const scalar_image_type epmach
            = std::numeric_limits<scalar_image_type>::epsilon();
          // smallest positive magnitude
          const scalar_image_type uflow
            = std::numeric_limits<scalar_image_type>::min();

          detail::interval_list<IntervalStorage> intervals(interval_storage);
          error_estimator<domain_type,image_type> errest1,errest2;

          // intialise results
          quadrature_error return_value=quadrature_ok;
          assign(result,0.0);
          assign(abserr,0.0);
          info.reset(); // reset the statistics

          // test on validity of requested accuracy parameters
          if (epsabs<=0.0
              && epsrel<(max)(errest1.minimum_relative_error(),0.5e-28))
            return invalid_input;

          // check we can do at least one interval
          if (!intervals.current_is_valid())
            return invalid_input;

          intervals.current()->set_domain(a,b);

          // first approximation to the integral
          integrator
            .kernel_data(intervals.current()->kernel_space())
            .error_estimator(errest1)
            .recorder(recorder)
            (f, a, b, result);
          info.kernel_evaluation();

          assign(abserr,errest1.absolute_error());

          // calculate requested accuracy.
          image_type errbnd;
          bool satisfied=error_bound(errbnd,epsabs,epsrel,result, abserr);

          intervals.current()->set(result, errest1.absolute_error(),errbnd);

          if (check_roundoff(errest1.roundoff(),abserr,errbnd))
            return_value=roundoff_detected;
          if (!intervals.has_capacity())
            return_value=max_intervals_reached;

          if (return_value==quadrature_ok
              && check_residual_roundoff(errest1.residual_roundoff(),
                                         abserr, errbnd)
              && !equal(errest1.absolute_error(),0.0) )
          {
            // initialization
            image_type errmax;
            assign(errmax,abserr);
            image_type area;
            assign(area,result);
            image_type errsum;
            assign(errsum,abserr);

            int iroff1 = 0;
            int iroff2 = 0;

            // main loop
            while (intervals.has_capacity()
                   && return_value==quadrature_ok
                   && !satisfied)
            {
              // bisect the subinterval with the largest error estimate.
              double a1 = intervals.largest_error()->lower_bound();
              double b1 = intervals.largest_error()->midpoint();
              double a2 = b1;
              double b2 = intervals.largest_error()->upper_bound();

              image_type area1;
              image_type area2;

              // bisect any working data required by the kernel
              bisect(intervals.largest_error()->kernel_space(),
                     intervals.next()->kernel_space());

              integrator
                .kernel_data(intervals.largest_error()->kernel_space())
                .error_estimator(errest1)
                .recorder(recorder)
                (f,a1,b1,area1);
              integrator
                .kernel_data(intervals.next()->kernel_space())
                .error_estimator(errest2)
                .recorder(recorder)
                (f,a2,b2,area2);
              info.kernel_evaluation(2);

              // improve previous approximations to integral and error and test
              // for accuracy.
              image_type area12;
              sum(area12,area1,area2);
              image_type erro12;
              sum(erro12,errest1.absolute_error(),errest2.absolute_error());

              accumulate_difference(
                errsum,erro12,intervals.largest_error()->error(),0.);
              accumulate_difference(
                area, area12, intervals.largest_error()->value(),0.);

              if (!any(errest1.residual_roundoff()) &&
                  !any(errest2.residual_roundoff()))
              {
                if (roundoff_in_area(intervals.largest_error()->value(),
                                     intervals.largest_error()->error(),
                                     area12, erro12,0.))
                  ++iroff1;

                if (intervals.size()>10
                    && detail::divergent(intervals.largest_error()->error(),erro12))
                  ++iroff2;
              }

              satisfied=error_bound(errbnd,epsabs,epsrel,area,errsum);

              if (!satisfied) {
                // test for roundoff error and eventually set error flag.
                if(iroff1>=6||iroff2>=20)
                  return_value = roundoff_detected;
                // set error flag in the case that the number of subintervals
                // equals limit.
                if(!intervals.has_capacity())
                  return_value = max_intervals_reached;
                // set error flag in the case of bad integrand behaviour
                // at a point of the integration range.
                if((max)((abs)(a1),(abs)(b2))<=
                   (1+10*epmach)*((abs)(a2)+100*uflow))
                  return_value = bad_integrand_behaviour;
              }

              // append the newly-created intervals to the list.
              intervals.bisected(a1, b1, area1, errest1.absolute_error(),
                                 a2, b2, area2, errest2.absolute_error(),
                                 errbnd);
            }

            // compute final result.
            intervals.accumulate(result);
            info.number_of_intervals(intervals.size());
            assign(abserr,errsum);
          }
          else
            info.number_of_intervals(1u);

          return quadrature_error_handler(BOOST_CURRENT_FUNCTION,return_value);
        }

        template <
            typename F,
            typename Domain,
            typename Image,
            typename Q,
            typename IntervalStorageSize,
            typename IntervalStorage,
            typename Recorder,
            typename Info,
            typename SeriesAcceleration
            >
        quadrature_error adaptive(
          const F& f,
          Domain a, Domain b,
          Image& result,
          Image& abserr,
          typename scalar_component<Image>::type epsabs,
          typename scalar_component<Image>::type epsrel,
          const Q& integrator,
          IntervalStorageSize max_storage,
          IntervalStorage& interval_storage,
          Recorder& recorder,
          Info& info,
          SeriesAcceleration& extrapolation
                                  )
        {
          using namespace std;

          typedef typename detail::storage_for_type<Image>::type image_type;
          typedef typename detail::storage_for_type<Domain>::type domain_type;
          typedef typename scalar_component<image_type>::type scalar_image_type;

          // largest relative spacing.
          const scalar_image_type epmach
            = std::numeric_limits<scalar_image_type>::epsilon();
          // smallest positive magnitude
          const scalar_image_type uflow
            = std::numeric_limits<scalar_image_type>::min();

          // check that the interval storage is actually storing values of
          // the correct type
//            BOOST_MPL_ASSERT(
//              ( boost::is_same<
//                typename boost::range_value<IntervalStorage>::type,
//                typename quadrature::kernel_interval<Q,Domain,image_type>::type>));

          detail::interval_list<IntervalStorage> intervals(interval_storage);
          error_estimator<domain_type,image_type> errest1,errest2;

          enum extrapolation_states
          {
            // non-error states
            extrapolation_ok,
            extrapolation_converged,
            // error states
            extrapolation_divergent,
            extrapolation_bad_behaviour,
          };


          // intialise results
          quadrature_error return_value=quadrature_ok;
          info.reset();
          assign(result,0.0);
          assign(abserr,0.0);
          info.reset(); // reset the statistics

          // test on validity of requested accuracy parameters
          if (epsabs<=0.0
              && epsrel<(max)(errest1.minimum_relative_error(),0.5e-28))
            return invalid_input;

          // check we can do at least one interval
          if (!intervals.current_is_valid())
            return invalid_input;

          intervals.current()->set_domain(a,b);

          // first approximation to the integral
          integrator
            .kernel_data(intervals.current()->kernel_space())
            .error_estimator(errest1)
            .recorder(recorder)
            (f, a, b, result);
          info.kernel_evaluation();

          assign(abserr,errest1.absolute_error());

          // test on accuracy.
          image_type errbnd;
          bool satisfied=error_bound(errbnd,epsabs,epsrel,result, abserr);
          intervals.current()->set(result,abserr, errbnd);

          if (check_roundoff(errest1.roundoff(),abserr,errbnd))
            return_value=roundoff_detected;
          if (!intervals.has_capacity())
            return_value=max_intervals_reached;

          if (return_value==quadrature_ok
              && check_residual_roundoff(errest1.residual_roundoff(),
                                         abserr, errbnd)
              && !equal(errest1.absolute_error(),0.0) )
          {
            // initialization
            image_type errmax;
            assign(errmax,abserr);
            image_type area;
            assign(area,result);
            image_type errsum;
            assign(errsum,abserr);

            // sum of the errors over the intervals larger than the
            // smallest interval considered up to now
            image_type erlarg;

            // errbnd used for extrapolation
            image_type ertest;

            int iroff1 = 0;
            int iroff2 = 0;
            int iroff3 = 0;

            // flag that the routine is attempting to perform
            // extrapolation
            bool extrap=false;
            // flag to mark extrapolation as no longer allowed
            bool inhibit_extrapolation=false;
            extrapolation_states extrapolation_status=extrapolation_ok;

            // length of the smallest interval considered up to now,
            // multiplied by 1.5
            double small = (abs)(b-a)*0.375;

            // number of extrapolations without error reduction
            std::size_t ktmin=0;

            image_type correc;
//             int ksgn=-1;
//             if ((abs)(result)>=(1.-errest1.minimum_relative_error())*errest1.integral_abs())
//               ksgn = 1;

            // main loop
            while (intervals.has_capacity()
                   && return_value==quadrature_ok
                   && !satisfied)
            {
              // bisect the subinterval with the largest error estimate.
              double a1 = intervals.selected()->lower_bound();
              double b1 = intervals.selected()->midpoint();
              double a2 = b1;
              double b2 = intervals.selected()->upper_bound();

              // error on the interval currently subdivided (before subdivision)
              image_type erlast;
              assign(erlast,intervals.selected()->error());

              image_type area1;
              image_type area2;

              // bisect any working data required by the kernel
              bisect(intervals.selected()->kernel_space(),
                     intervals.next()->kernel_space());

              integrator
                .kernel_data(intervals.selected()->kernel_space())
                .error_estimator(errest1)
                .recorder(recorder)
                (f,a1,b1,area1);
              integrator
                .kernel_data(intervals.next()->kernel_space())
                .error_estimator(errest2)
                .recorder(recorder)
                (f,a2,b2,area2);

              info.kernel_evaluation(2);

              // improve previous approximations to integral and error and test
              // for accuracy.
              image_type area12;
              sum(area12,area1,area2);
              image_type erro12;
              sum(erro12,errest1.absolute_error(),errest2.absolute_error());

              accumulate_difference(
                errsum,erro12,intervals.selected()->error(),0.);
              accumulate_difference(
                area, area12, intervals.selected()->value(),0.);

              if (!any(errest1.residual_roundoff()) &&
                  !any(errest2.residual_roundoff()))
              {
                if (roundoff_in_area(intervals.selected()->value(),
                                     intervals.selected()->error(),
                                     area12, erro12,0.))
                  ++iroff1;

                if (intervals.size()>10
                    && detail::divergent(intervals.selected()->error(),erro12))
                  ++iroff2;

                if (extrap)
                  ++iroff3;
              }

              satisfied=error_bound(errbnd,epsabs,epsrel,area,errsum);

              if (!satisfied) {
                // test for roundoff error and eventually set error flag.
                if(iroff1>=6||iroff2>=20)
                  return_value = roundoff_detected;
                if(iroff3>=5)
                  extrapolation_status = extrapolation_bad_behaviour;
                // set error flag in the case that the number of subintervals
                // equals limit.
                if (!intervals.has_capacity())
                  return_value = max_intervals_reached;
                // set error flag in the case of bad integrand behaviour
                // at a point of the integration range.
                if((max)(std::abs(a1),std::abs(b2))<=
                   (1+10*epmach)*(std::abs(a2)+100*uflow))
                  return_value = bad_integrand_behaviour;
              }

              // append the newly-created intervals to the list.
              intervals.bisected(a1, b1, area1, errest1.absolute_error(),
                                 a2, b2, area2, errest2.absolute_error(),
                                 errbnd);

              if (satisfied)
                break;

              if (return_value)
                break;

              if (intervals.size()==2)
              {
                assign(erlarg,errsum);
                assign(ertest,errbnd);
                image_type dummy;
                extrapolation(area, dummy, dummy);
                continue;
              }

              if (inhibit_extrapolation)
                continue;

              subtract(erlarg,erlast);
              if ((abs)(b1-a1)>small)
                add(erlarg,erro12);

              if (!extrap)
              {
                // if the largest error interval is not the smallest interval,
                // continue bisection without extrapolation
                if ((abs)(intervals.selected()->span())>small)
                  continue;
                extrap = true;
                intervals.select(1);
              }

              // the smallest interval has the largest error.
              if (extrapolation_status!=extrapolation_bad_behaviour
                  && erlarg>ertest)
              {
                // before bisecting decrease the sum of the errors over the
                // larger intervals (erlarg) and perform extrapolation.
                while (intervals.has_prior())
                {
                  if ((abs)(intervals.selected()->span())<=small)
                    break;
                  intervals.prior();
                }
                continue;
              }


              // perform series extrapolation
              image_type reseps, abseps;
              if (extrapolation(area,reseps,abseps))
              {
                ktmin = ktmin+1;
                if (ktmin>5 && divergent2(abserr,errsum))
                  return_value = quadrature::divergent;

                if (less_than(abseps,abserr))
                {
                  ktmin = 0;
                  assign(abserr,abseps);
                  assign(result,reseps);
                  assign(correc,erlarg);
                  // ***jump out of do-loop
                  if (error_bound(ertest,epsabs,epsrel,reseps,abserr))
                  {
                    extrapolation_status=extrapolation_converged;
                    break;
                  }
                }
              }

              // prepare bisection of the smallest interval.
#if 0 // not sure about these
              if (numrl2==1)
                noext = true;
#endif
              if (return_value == quadrature::divergent)
                break;

              intervals.select(0);
              extrap = false;
              small *= 0.5;
              assign(erlarg,errsum);

            } // while

            if (extrapolation_status>extrapolation_converged)
              add(abserr,correc);

            // test for divergence
//             if ((extrapolation_status>extrapolation_converged || return_value)
//                 && abserr/(abs)(result)<=errsum/(abs)(area))
//             {
//               if (//ksgn!=(-1) || (max)((abs)(result),(abs)(area))||errest1.integral_abs()*0.1e-01)
//                 if (0.01>(result/area)||(result/area)>100||
//                     errsum>(abs)(area))
//                   return_value=quadrature::divergent;
//             }
//             else
            {
              // compute final result.
              if (extrapolation_status!=extrapolation_converged)
              {
                intervals.accumulate(result);
                assign(abserr,errsum);
              }
              info.number_of_intervals(intervals.size());
            }
          } // if
          else
            info.number_of_intervals(1u);

          return quadrature_error_handler(BOOST_CURRENT_FUNCTION,return_value);
        }


        template <
            typename F,
            typename Domain,
            typename Image,
            typename Q,
            typename IntervalStorageSize,
            typename Recorder,
            typename Info,
            typename Accelerator
            >
        quadrature_error adaptive(
          const F& f,
          Domain a, Domain b,
          Image& result,
          Image& abserr,
          typename scalar_component<Image>::type epsabs,
          typename scalar_component<Image>::type epsrel,
          const Q& integrator,
          IntervalStorageSize limit,
          null_intervals&,
          Recorder& recorder,
          Info& info,
          Accelerator& accelerator
                                  )
        {
          typedef typename quadrature::kernel_interval<Q,Domain,Image>::type
            interval_type;
          std::list<interval_type> interval_storage(limit);
          return detail::adaptive(
            f, a, b,
            result, abserr,
            epsabs, epsrel,
            integrator, limit, interval_storage, recorder, info, accelerator);
        }

      }// namespace




        //! Global, adaptive integration.
        /*! Based on QUADPACK QAG and QAGS and the bisection of the interval
          of integration.
        */
      template <
          BOOST_QUADRATURE_ARG_TEMPLATE(Kernel),
          BOOST_QUADRATURE_ARG_TEMPLATE(RelAcc),
          BOOST_QUADRATURE_ARG_TEMPLATE(AbsAcc),
          BOOST_QUADRATURE_ARG_TEMPLATE(MaxInt),
          BOOST_QUADRATURE_ARG_TEMPLATE(Info),
          BOOST_QUADRATURE_ARG_TEMPLATE(Accel),
          BOOST_QUADRATURE_ARG_TEMPLATE(Intervals),
          BOOST_QUADRATURE_ARG_TEMPLATE(Recorder)
          >
      class adaptive_alg
      {
      public:
        adaptive_alg(
          detail::arg_cref<Kernel> kernel=detail::arg_cref<Kernel>(),
          detail::arg_value<RelAcc> relative_accuracy=detail::arg_value<RelAcc>(),
          detail::arg_value<AbsAcc> absolute_accuracy=detail::arg_value<AbsAcc>(),
          detail::arg_value<MaxInt> max_intervals=detail::arg_value<MaxInt>(),
          detail::arg_ref<Info> info=detail::arg_ref<Info>(),
          detail::arg_ref<Accel> accel=detail::arg_ref<Accel>(),
          detail::arg_ref<Intervals> intervals=detail::arg_ref<Intervals>(),
          detail::arg_ref<Recorder> recorder=detail::arg_ref<Recorder>()
                     )
            : m_kernel(kernel),
              m_relative_accuracy(relative_accuracy),
              m_absolute_accuracy(absolute_accuracy),
              m_max_intervals(max_intervals),
              m_info(info),
              m_accelerator(accel),
              m_intervals(intervals),
              m_recorder(recorder)
        {}

        //! Integrate the specified functor
        /*! Integrates the specified functor between lower_limit and
          upper_limit.
          @param integrand   function to be integrated
          @param lower_limit lower bound on range of integration
          @param upper_limit upper bound on range of integration
          @param result     approximation of integral
          @param error      estimation of absolute error in integral approximation
          @returns Error code.  invalid_input if, epsabs<0 and epsrel<max(50*epsilon,0.5e-28) or size(interval_storage)<1
        */
        template <typename F, typename DL, typename DU, typename R>
        quadrature_error operator()(
          F integrand,DL lower_limit, DU upper_limit, R& result, R& error)
        {
          // instantiate any defaults
          return ::boost::numeric::quadrature::detail::adaptive(
            integrand,
            detail::promote(lower_limit, upper_limit), //promote upper and lower
            detail::promote(upper_limit, lower_limit), //limits to same type
            result, error,
            absolute_accuracy_(m_absolute_accuracy),
            relative_accuracy_(m_relative_accuracy),
            kernel_(m_kernel),
            max_intervals_(m_max_intervals),
            intervals_(m_intervals),
            recorder_(m_recorder),
            info_(m_info),
            accelerator_(m_accelerator)
                                                                );
        }


        //! supply kernel
        /*! Supply integration kernel
          @param kernel   the integration kernel to be used
          @result an adaptive integrator object, which includes the kernel
         */
        template <typename Kernel_>
        adaptive_alg<Kernel_,RelAcc,AbsAcc,MaxInt,Info,Accel,Intervals,Recorder>
        kernel(const Kernel_& kernel)
        {
          return adaptive_alg<Kernel_,RelAcc,AbsAcc,MaxInt,Info,Accel,Intervals,
            Recorder>(
              detail::arg_cref<Kernel_>(kernel),
              m_relative_accuracy, m_absolute_accuracy, m_max_intervals,
              m_info, m_accelerator, m_intervals, m_recorder);
        }

        //! supply info
        /*! Supply info
          @param info   the adaptive info to be used
          @result an adaptive integrator object, which includes the info
         */
        template <typename Info_>
        adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt,Info_,Accel,Intervals,Recorder>
        info(Info_& info)
        {
          return adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt,Info_,Accel,Intervals,
            Recorder>(
              m_kernel, m_relative_accuracy, m_absolute_accuracy,
              m_max_intervals,
              detail::arg_ref<Info_>(info),
              m_accelerator, m_intervals,m_recorder);
        }

        //! supply relative accuracy
        /*! Supply relative accuracy
          @param accuracy   the relative accuracy to be used
          @result an adaptive integrator object, which includes the relative accuracy
         */
        template <typename RelAcc_>
        adaptive_alg<Kernel,RelAcc_,AbsAcc,MaxInt,Info,Accel,Intervals,Recorder>
        relative_accuracy(RelAcc_ accuracy)
        {
          return adaptive_alg<Kernel,RelAcc_,AbsAcc,MaxInt,Info,Accel,Intervals,
            Recorder>(
              m_kernel,
              detail::arg_value<RelAcc_>(accuracy), m_absolute_accuracy,
              m_max_intervals,
              m_info,
              m_accelerator, m_intervals,m_recorder);
        }

        //! supply absolute accuracy
        /*!
          @param accuracy   the absolute accuracy to be used
          @result an adaptive integrator object, which includes the absolute accuracy
         */
        template <typename AbsAcc_>
        adaptive_alg<Kernel,RelAcc,AbsAcc_,MaxInt,Info,Accel,Intervals,Recorder>
        absolute_accuracy(AbsAcc_ accuracy)
        {
          return adaptive_alg<Kernel,RelAcc,AbsAcc_,MaxInt,Info,Accel,Intervals,
            Recorder>(
              m_kernel,
              m_relative_accuracy, detail::arg_value<AbsAcc_>(accuracy),
              m_max_intervals,
              m_info,
              m_accelerator, m_intervals,m_recorder);
        }

        //! supply max intervals
        /*! Supply max intervals
          @param num_intervals   the max number of intervals to be used
          @result an adaptive integrator object, which includes the max intervals
         */
        template <typename MaxInt_>
        adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt_,Info,Accel,Intervals,Recorder>
        max_intervals(MaxInt_ num_intervals)
        {
          return adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt_,Info,Accel,Intervals,
            Recorder>(
              m_kernel,
              m_relative_accuracy, m_absolute_accuracy,
              detail::arg_value<MaxInt_>(num_intervals),
              m_info,
              m_accelerator, m_intervals,m_recorder);
        }

        //! supply interval storage
        /*! Supply interval storage
          @param intervals   the interval storage to be used
          @result an adaptive integrator object, which includes the interval storage
         */
        template <typename Intervals_>
        adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt,Info,Accel,Intervals_,Recorder>
        intervals(Intervals_& intervals)
        {
          return adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt,Info,Accel,Intervals_,
            Recorder>(
              m_kernel,
              m_relative_accuracy, m_absolute_accuracy,
              m_max_intervals,
              m_info,
              m_accelerator,
              detail::arg_ref<Intervals_>(intervals),
              m_recorder);
        }

        //! supply accelerator
        /*! Supply accelerator
          @param accelerator   the accelerator to be used
          @result an adaptive integrator object, which includes the accelerator
         */
        template <typename Accel_>
        adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt,Info,Accel_,Intervals,Recorder>
        accelerator(Accel_& accelerator)
        {
          return adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt,Info,Accel_,Intervals,
            Recorder>(
              m_kernel,
              m_relative_accuracy, m_absolute_accuracy,
              m_max_intervals,
              m_info,
              detail::arg_ref<Accel_>(accelerator),
              m_intervals,
              m_recorder);
        }

        //! supply recorder
        /*!
          @param recorder   the recorder to be used
          @result an adaptive integrator object, which includes the recorder
         */
        template <typename Recorder_>
        adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt,Info,Accel,Intervals,Recorder_>
        recorder(Recorder_& recorder)
        {
          return adaptive_alg<Kernel,RelAcc,AbsAcc,MaxInt,Info,Accel,Intervals,
            Recorder_>(
              m_kernel,
              m_relative_accuracy, m_absolute_accuracy,
              m_max_intervals,
              m_info,
              m_accelerator,
              m_intervals,
              detail::arg_ref<Recorder_>(recorder));
        }

      private:
#ifndef BOOST_QUADRATURE_DOXYGEN
        BOOST_QUADRATURE_CREF_ARG(kernel, Kernel, kronrod_gauss<21>, kronrod_gauss<21>())
        BOOST_QUADRATURE_VALUE_ARG(relative_accuracy, RelAcc,double,1e-5);
        BOOST_QUADRATURE_VALUE_ARG(absolute_accuracy, AbsAcc,double,0.);
        BOOST_QUADRATURE_VALUE_ARG(max_intervals, MaxInt,std::size_t,100);
        BOOST_QUADRATURE_REF_ARG(info, Info, detail::null_adaptive_info,
                   detail::null_adaptive_info::instance())
        BOOST_QUADRATURE_REF_ARG(accelerator, Accel,
                     detail::null_adaptive_accelerator,
                     detail::null_adaptive_accelerator::instance())
        BOOST_QUADRATURE_REF_ARG(intervals, Intervals, detail::null_intervals,
                    detail::null_intervals::instance())
        BOOST_QUADRATURE_REF_ARG(recorder, Recorder,
                    detail::null_recorder,
                    detail::null_recorder::instance())
#endif
      };

      //! The base adaptive integrator, before any options are applied
      typedef adaptive_alg<> adaptive;

    }// namespace
  }// namespace
}// namespace

#endif
