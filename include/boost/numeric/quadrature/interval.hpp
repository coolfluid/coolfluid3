#ifndef BOOST_NUMERIC_QUADRATURE_INTERVAL_HPP
#define BOOST_NUMERIC_QUADRATURE_INTERVAL_HPP

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   interval.hpp
  @brief  Structures for use by quadrature over an interval
*/

#include <boost/numeric/quadrature/quadrature_config.hpp>
#include <boost/numeric/quadrature/interval_fwd.hpp>
#include <boost/numeric/quadrature/detail/local_traits.hpp>
#include <boost/numeric/quadrature/detail/null_interval_kernel_space.hpp>
#include <boost/math/tools/precision.hpp>
#include <boost/call_traits.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>
#include <boost/range.hpp>
#include <boost/utility/enable_if.hpp>
#include <numeric>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        inline void bisect(null_interval_kernel_space&,null_interval_kernel_space&)
        {}

        template <typename Image>
        void scale_error(
          Image& scaled_error,
          const Image& error,
          const Image& /* bound */,
          typename boost::enable_if<is_arithmetic_scalar<Image> >::type* v=0)
        {
          scaled_error=error;
        }

        template <typename ScalarImage, typename Image>
        inline void scale_error(
          ScalarImage& scaled_error,
          const Image& error,
          const Image& bound,
          typename boost::disable_if<is_arithmetic_scalar<Image> >::type* v=0)
        {
          using namespace std;
          BOOST_QUADRATURE_ASSERT(boost::size(bound)==boost::size(error));
          typename boost::range_const_iterator<Image>::type
            bnd=boost::begin(bound);
          typename boost::range_const_iterator<Image>::type
            i=boost::begin(error),
            i_end=boost::end(error);
          scaled_error=boost::math::tools::min_value<ScalarImage>();
          for (; i!=i_end; ++i, ++bnd)
          {
            ScalarImage v=*i/(*bnd);
            scaled_error=(max)(scaled_error,v);
          }
        }

      }

      //! Keeps track of a sub-interval of the full integration range.
      template <typename Domain, typename Image, typename KernelSpace>
      class interval
      {
      public:
        BOOST_STATIC_ASSERT(
          (boost::is_same<Domain,
           typename detail::storage_for_type<Domain>::type>::value ));
        BOOST_STATIC_ASSERT(
          (boost::is_same<Image,
           typename detail::storage_for_type<Image>::type>::value ));

        typedef Image image;
        typedef Domain domain;
        typedef typename detail::scalar_component<Image>::type scalar_image;

        void set_domain(typename call_traits<Domain>::param_type aa,
                        typename call_traits<Domain>::param_type bb)
        {
          a=aa;
          b=bb;
#ifdef BOOST_QUADRATURE_DEBUG
          detail::assign(r,0.);
          detail::assign(e,0.);
#endif
        }

        //! set the values for the integration estimate over this interval
        /*! for non scalar integrands, the absolute error is scaled
          by the error bound.
          @param integral the estimate of the integral
          @param abserr the estimate of the absolute error
          @param bound the bound that is trying to be achieved
        */
        template <typename Result, typename Abserr, typename Bound>
        void set(const Result& integral,
                 const Abserr& abserr,
                 const Bound& bound)
        {
          detail::assign(r,integral);
          detail::assign(e,abserr);
          detail::scale_error(m_scaled_error,e,bound);
        }

        void lower_bound(typename call_traits<Domain>::param_type aa)
        {
          a=aa;
        }

        void upper_bound(typename call_traits<Domain>::param_type bb)
        {
          b=bb;
        }

        typename call_traits<Domain>::const_reference lower_bound() const
        {
          return a;
        }

        typename call_traits<Domain>::const_reference upper_bound() const
        {
          return b;
        }

        Domain midpoint() const
        {
          return 0.5*(a+b);
        }

        Domain span() const
        {
          return b-a;
        }

        typename call_traits<Image>::const_reference value() const
        {
          return r;
        }

        typename call_traits<Image>::const_reference error() const
        {
          return e;
        }

        KernelSpace& kernel_space()
        {
          return m_kernel_space;
        }

        typename boost::call_traits<scalar_image>::const_reference
        scaled_error() const
        {
          return m_scaled_error;
        }

      private:
        Domain a;
        Domain b;
        Image r;
        Image e;
        scalar_image m_scaled_error;
        KernelSpace m_kernel_space;
      };

      namespace detail
      {

        /** Maintain a list of intervals */
        template <typename IntervalRange>
        class interval_list
        {
        public:
          interval_list(IntervalRange& intervals)
              : m_intervals(intervals),
                m_current(boost::begin(intervals)),
                m_largest_error(m_current),
                m_selected(m_current),
                m_offset(0)
          {}

          typedef typename boost::range_iterator<IntervalRange>::type iterator;
          typedef typename boost::range_const_iterator<IntervalRange>::type const_iterator;
          typedef typename boost::range_value<IntervalRange>::type value_type;
          typedef typename boost::range_size<IntervalRange>::type size_type;
          typedef typename boost::range_difference<IntervalRange>::type difference_type;

          typedef typename value_type::image image;
          typedef typename value_type::domain domain;

          size_type size() const
          {
            return std::distance(boost::begin(m_intervals),m_current)+1;
          }

          size_type capacity() const
          {
            return std::distance(boost::begin(m_intervals),
                                 boost::end(m_intervals));
          }


          bool current_is_valid() const
          {
            return m_current!=boost::end(m_intervals);
          }

          bool has_capacity() const
          {
            return boost::next(m_current)!=boost::end(m_intervals);
          }

          iterator current()
          {
            return m_current;
          }

          iterator next()
          {
            return boost::next(m_current);
          }

          iterator largest_error()
          {
            return m_largest_error;
          }

          void select(difference_type i)
          {
            m_offset=i;
            m_selected=boost::next(m_largest_error,i);
          }

          bool has_prior() const
          {
            return m_selected!=m_current;
          }

          void prior()
          {
            BOOST_ASSERT(has_prior());
            ++m_offset;
            std::advance(m_selected,1);
          }

          //! Return the interval with the 'th largest error
          /*!
            @return interval wiht i'th largest error
          */
          iterator selected()
          {
            return m_selected;
          }

          //! Provide bisected data for the largest error interval
          /** The information has to be put in the list of intervals at the
              correct locations */
          void bisected(
            domain a1, domain b1,
            typename boost::call_traits<image>::param_type area1,
            typename boost::call_traits<image>::param_type error1,
            domain a2, domain b2,
            typename boost::call_traits<image>::param_type area2,
            typename boost::call_traits<image>::param_type error2,
            typename boost::call_traits<image>::param_type bound
                        )
          {
            ++m_current;
            BOOST_QUADRATURE_ASSERT(m_selected!=boost::end(m_intervals));
            BOOST_QUADRATURE_ASSERT(m_current!=boost::end(m_intervals));
            BOOST_QUADRATURE_ASSERT(m_selected->lower_bound()==a1
                         ||m_selected->lower_bound()==a2);
            BOOST_QUADRATURE_ASSERT(m_selected->upper_bound()==b1
                         ||m_selected->upper_bound()==b2);
            BOOST_QUADRATURE_ASSERT(b1==a2);

//             if (error2<=error1)
//             {
              BOOST_QUADRATURE_ASSERT(m_selected->lower_bound()==a1);
              m_selected->upper_bound(b1);
              m_selected->set(area1,error1, bound);
              m_current->set_domain(a2,b2);
              m_current->set(area2,error2, bound);
//             }
//             else
//             {
//               m_selected->lower_bound(a2);
//               BOOST_ASSERT(m_selected->upper_bound()==b2);
//               m_selected->set(area2,error2);
//               m_current->set_domain(a1,b1);
//               m_current->set(area1,error1);
//             }

            // maintain the descending ordering
            // in the list of error estimates and select the subinterval
            // with the largest error estimate (to be bisected next).
#if defined(BOOST_QUADRATURE_INTERVAL_USE_SORT)
            difference_type d=std::distance(boost::begin(m_intervals),m_current);
            iterator i=m_current;
            ++i;
            std::sort(m_largest_error, i, interval_error_greater());
            m_largest_error=m_intervals.begin();
            m_current=boost::next(boost::begin(m_intervals),d);

#else
            // fix the location of m_current and m_selected
            bool sort_selected_lower=true;
            iterator rest=boost::next(m_selected);
            iterator i=rest;
            iterator end=m_current;// this can be optimised

            if (m_selected!=m_largest_error && // there is a prior
                m_selected->scaled_error()>boost::prior(m_selected)->scaled_error())
            {
              // sort selected into higher error list
              iterator i=std::lower_bound(
                m_largest_error, m_selected,
                *m_selected, interval_error_greater());
              std::rotate(i,m_selected,rest);
              sort_selected_lower=false;
            }
            if (sort_selected_lower)
            {
              // sort selected into lower error list
              if (m_selected->scaled_error() < rest->scaled_error())
              {
                i=std::lower_bound(rest, end, *m_selected,
                                   interval_error_greater());
                std::rotate(m_selected,boost::next(m_selected),i);
                if (i==end)
                  i=boost::next(m_selected);
              }
            }
            // sort current
            if (m_current->scaled_error()>i->scaled_error())
              i=std::lower_bound(
                m_largest_error, i, *m_current, interval_error_greater());
            else
              i=std::lower_bound(i, end, *m_current, interval_error_greater());
            std::rotate(i, m_current, boost::next(m_current));
#endif
            m_selected=boost::next(m_largest_error, m_offset);
          }

          template <typename Result>
          void accumulate(Result& result) const
          {
            detail::assign(result,0.0);
            accumulate_result(
              result,
              boost::iterator_range<iterator>(
                boost::begin(m_intervals),boost::next(m_current)));
          }

        private:

          template <typename Value, typename InputRange>
          void accumulate_result(Value& output, const InputRange& input) const
          {
            typename boost::range_const_iterator<InputRange>::type
              in=boost::begin(input),
              in_end=boost::end(input);
            for (; in!=in_end; ++in)
              detail::accumulate(output,in->value(),0.);
          }

          struct result_accumulator
          {
            image operator()(image value, const value_type& interval) const
            {
              return interval.value()+value;
            }
          };

          struct interval_error_greater
          {
            bool operator()(const value_type& a, const value_type& b) const
            {
              return a.scaled_error()>b.scaled_error();
            }
          };

          IntervalRange& m_intervals;
          iterator m_current;
          iterator m_largest_error;
          iterator m_selected;
          difference_type m_offset;
        };

      } // namespace detail


    }// namespace
  }// namespace
}// namespace

#endif
