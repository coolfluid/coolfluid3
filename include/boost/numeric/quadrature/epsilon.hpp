#ifndef BOOST_NUMERIC_QUADRATURE_EPSILON_HPP
#define BOOST_NUMERIC_QUADRATURE_EPSILON_HPP

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   epsilon.hpp
  @brief  Wynn epsilon algorithm
*/

#include <boost/numeric/quadrature/detail/local_traits.hpp>
#include <boost/numeric/quadrature/detail/vector_traits.hpp>
#include <boost/numeric/quadrature/detail/array_traits.hpp>
#include <boost/array.hpp>
#include <boost/call_traits.hpp>
#include <boost/math/tools/precision.hpp>
#include <algorithm>
#include <limits>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        template <typename T>
        void resize(
          T& range, typename boost::range_size<T>::type size,
          typename boost::enable_if<
            is_arithmetic_scalar<typename boost::range_value<T>::type> >::type*v=0)
        {
          range.resize(size);
        }

        template <typename T>
        void resize(
          T& range, typename boost::range_size<T>::type size,
          typename boost::disable_if<
            is_arithmetic_scalar<typename boost::range_value<T>::type> >::type*v=0)
        {
          for (typename boost::range_mutable_iterator<T>::type
                 i=boost::begin(range), i_end=boost::end(range);
               i!=i_end; ++i)
            i->resize(size);
        }

      }

      //! Approximates the limit of a sequence by Wynn's epsilon algorithm
      /*! Approximates the limit of a given sequence, by means of
        the epsilon algorithm of P.Wynn.
        An estimate of the absolute error is also given.
        The condensed epsilon table is computed. Only those elements
        needed for the computation of the next diagonal are
        preserved.
      */
      template <typename Value>
      class wynn_epsilon_algorithm
      {
      public:
        typedef typename detail::scalar_component<Value>::type
          scalar_type;
        typedef typename detail::storage_for_type<Value>::type
          storage_type;

        typedef typename detail::array_storage_for_type<Value,3>::type history;

        typedef typename detail::vector_storage_for_type<Value>::type
          container;


      public:
        wynn_epsilon_algorithm()
            : n(0), m_converged(false)
        {
          detail::resize(m_epstab,52);

#ifndef BOOST_QUADRATURE_NDEBUG
          scalar_type nan=std::numeric_limits<scalar_type>::quiet_NaN();
          detail::assign(m_epstab,nan);
#endif
        }

        //! Provide a new value from the series and obtain a limit
        /*!
          An estimate of the absolute error is also given.
          The condensed epsilon table is computed. Only those elements
          needed for the computation of the next diagonal are
          preserved.
          @param value   the next value from the series
          @param result  estimate of series limit
          @param abserr  estimate of absolute error for result
          @return true if a valid result is provided, false otherwise
        */
        template <typename Value2, typename Result, typename Abserr >
        bool operator()(
          const Value2& value,
          Result& result,
          Abserr& abserr);

        //! flag if the algorithm has recognised convergence
        /*!
          @return   true if converged
        */

        bool converged() const
        {
          return m_converged;
        }

      private:
        template <typename Value2, typename Result, typename Abserr >
        bool implementation(
          const Value2& value, Result& result, Abserr& abserr,
          std::vector<scalar_type>& epstab,
          std::tr1::array<scalar_type,3>& history);

        template <typename Value2, typename Result, typename Abserr,
            typename Epstab, typename History>
        bool implementation(
          const Value2& value, Result& result, Abserr& abserr,
          Epstab& epstab, History&);

        container m_epstab;
        history m_res3la;
        std::size_t n;
        bool m_converged;
      };

      template <typename Value>
      template <typename Value2, typename Result, typename Abserr >
      bool wynn_epsilon_algorithm<Value>::operator()(
        const Value2& value,
        Result& result,
        Abserr& abserr)
      {
        m_converged=true;
        bool rv=implementation(value, result, abserr, m_epstab, m_res3la);
        ++n;
        return rv;
      }

      // used when we have a vector valued function
      template <typename Value>
      template <typename Value2, typename Result, typename Abserr,
          typename Epstab, typename History>
      bool wynn_epsilon_algorithm<Value>::implementation(
        const Value2& value,
        Result& result,
        Abserr& abserr,
        Epstab& epstab,
        History& history)
      {
        typename boost::range_const_iterator<Value2>::type
          v=boost::begin(value), v_end=boost::end(value);
        typename boost::range_iterator<Result>::type
          r=boost::begin(result);
        typename boost::range_iterator<Abserr>::type
          e=boost::begin(abserr);
        typename boost::range_iterator<Epstab>::type
          t=boost::begin(epstab);
        typename boost::range_iterator<History>::type
          h=boost::begin(history);

        bool rv=true;
        for (; v!=v_end; ++v,++r,++e,++t, ++h)
        {
          rv=implementation(*v,*r,*e,*t,*h) && rv;
        }
        return rv;

      }


      template <typename Value>
      template <typename Value2, typename Result, typename Abserr>
      bool wynn_epsilon_algorithm<Value>::implementation(
        const Value2& value,
        Result& result,
        Abserr& abserr,
        std::vector<scalar_type>& epstab,
        std::tr1::array<scalar_type,3>& history)
      {
        // smallest relative spacing.
        const scalar_type epmach = boost::math::tools::epsilon<scalar_type>();
        // largest positive magnitude
        const scalar_type oflow = boost::math::tools::max_value<scalar_type>();

        bool converged=false;

        abserr=oflow;
        result=value;
        epstab[n]=value;

        if (n<3)
          history[n]=value;
        if (n<2)
        {
          m_converged=false;
          return false;
        }
        else
        {
          std::size_t limexp = epstab.size()-2;
          epstab[n+2] = epstab[n];
          epstab[n] = oflow;

          // 1 + number of elements to be computed in the new diagonal
          std::size_t newelm = n/2;
          std::size_t num = n;
          std::size_t k1 = n;

          for (std::size_t i = 0; i!=newelm; ++i)
          {
            std::size_t k2 = k1-1;
            std::size_t k3 = k1-2;
            scalar_type res = epstab[k1+2];
            scalar_type e0 = epstab[k3];
            scalar_type e1 = epstab[k2];
            scalar_type e2 = res;
            scalar_type e1abs = std::abs(e1);
            scalar_type delta2 = e2-e1;
            scalar_type err2 = std::abs(delta2);
            scalar_type tol2 = std::max(std::abs(e2),e1abs)*epmach;
            scalar_type delta3 = e1-e0;
            scalar_type err3 = std::abs(delta3);
            scalar_type tol3 = std::max(e1abs,std::abs(e0))*epmach;

            if ((err2<tol2 && err3<tol3))
            {
              // if e0, e1 and e2 are equal to within machine accuracy,
              // convergence is assumed.
              result = res;
              abserr = err2+err3;
              epstab[k1]=res;
              converged=true;
              break;
            }

            scalar_type e3 = epstab[k1];

            epstab[k1] = e1;
            scalar_type delta1 = e1-e3;

            scalar_type err1 = std::abs(delta1);
            scalar_type tol1 = std::max(e1abs,std::abs(e3))*epmach;

            // if two elements are very close to each other, omit a part
            // of the table by adjusting the value of n
            if (err1<=tol1 || err2<=tol2 || err3<=tol3)
            {
              n = i+i;
              break;
            }

            scalar_type ss = 1/delta1+1/delta2-1/delta3;
            scalar_type epsinf = std::abs(ss*e1);

            // test to detect irregular behaviour in the table, and
            // eventually omit a part of the table adjusting the value
            // of n.
            if (epsinf<=0.1e-03)
            {
              n = i+i;
              break;
            }

            // compute a new element and eventually adjust the value of result.
            res = e1+1./ss;
            epstab[k1] = res;
            k1 -= 2;
            scalar_type error = err2+std::abs(res-e2)+err3;
            if (error<=abserr)
            {
              abserr = error;
              result = res;
            }
          } // for

          m_converged = m_converged && converged;

          // shift the table.
          std::size_t ib = 0;
          if ((num/2)*2!=num) // odd
            ib = 1;
          std::size_t ie = newelm+1;
          for (std::size_t i=0; i!=ie; ++i)
          {
            std::size_t ib2 = ib+2;
            epstab[ib] = epstab[ib2];
            ib = ib2;
          }

          // shift the table.
          if (n==limexp-1)
            n = 2*(limexp/2)-2;

          if (num!=n)
          {
            std::size_t indx = num-n;
            for (std::size_t i = 0; i!=n+3; ++i,++indx)
              epstab[i]=epstab[indx];
          }

          // compute error estimate
          abserr =
            std::abs(result-history[2])
            +std::abs(result-history[1])
            +std::abs(result-history[0]);

          history[0] = history[1];
          history[1] = history[2];
          history[2] = result;
        }
        abserr = std::max(abserr,5*epmach*std::abs(result));
        return true;
      }

    }// namespace
  }// namespace
}// namespace

#endif
