#ifndef BOOST_NUMERIC_QUADRATURE_LOCAL_TRAITS_H
#define BOOST_NUMERIC_QUADRATURE_LOCAL_TRAITS_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   local_traits.hpp
  @brief  traits for use internally in quadrature routines
*/

#include <boost/numeric/quadrature/arithmetic_vector_traits.hpp>
#include <boost/numeric/quadrature/is_arithmetic_scalar.hpp>

#include <boost/array.hpp>
#include <boost/tr1/array.hpp>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/ice.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/identity.hpp>

#include <boost/math/special_functions/fpclassify.hpp>

#include <limits>
#include <cmath>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      template <typename Value, int N>
      struct arithmetic_vector_value<Value(&)[N]>
      {
        typedef Value type;
      };

      template <typename Value, std::size_t N>
      struct arithmetic_vector_value<std::tr1::array<Value,N> >
      {
        typedef Value type;
      };

      template <typename Value, int N>
      struct arithmetic_vector_size<Value(&)[N]>
      {
        BOOST_STATIC_CONSTANT(int, value=N);
      };

      template <typename Value, int N>
      struct arithmetic_vector_size<Value[N]>
      {
        BOOST_STATIC_CONSTANT(int, value=N);
      };

      template <typename Value, std::size_t N>
      struct arithmetic_vector_size< std::tr1::array<Value,N> >
      {
        BOOST_STATIC_CONSTANT(int, value=N);
      };

      namespace detail
      {
        template <typename Value, typename Required>
        struct storage_for_vector_type
        {
          typedef std::tr1::array<Required,
              arithmetic_vector_size<Value>::value > type;
        };

        template <typename Value>
        struct storage_for_vector_type<Value,Value>
        {
          typedef std::tr1::array<
              typename arithmetic_vector_value<Value>::type,
              arithmetic_vector_size<Value>::value > type;
        };

        template <typename Required>
        struct storage_for_scalar_type
        {
          typedef Required type;
        };

        // determine the type that will be used for workspace
        template <typename Value, typename Required=Value>
        struct storage_for_type
          : ::boost::mpl::if_<
            is_arithmetic_scalar<Value>,
            storage_for_scalar_type<Required> ,
            storage_for_vector_type<Value,Required> >::type
        {};

        template <typename Value>
        struct recursive_vector_value
          : mpl::if_<
            is_arithmetic_scalar<typename arithmetic_vector_value<Value>::type>,
            arithmetic_vector_value<Value>,
            arithmetic_vector_value<typename arithmetic_vector_value<Value>::type > >::type
        {};

      template <typename Value>
      struct scalar_component
        : ::boost::mpl::if_< is_arithmetic_scalar<Value>,
          ::boost::mpl::identity<Value> ,
          recursive_vector_value<Value> >::type
      {};

        inline bool any(bool value)
        {
          return value;
        }

        template <typename BoolRange>
        bool any(const BoolRange& flags)
        {
          typename boost::range_const_iterator<BoolRange>::type
            out=boost::begin(flags),
            out_end=boost::end(flags);
          for (; out!=out_end; ++out)
            if (*out)
              return true;
          return false;
        }

        template <typename Value>
        void subtract(
          Value& output, Value value,
          typename enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          output-=value;
        }

        template <typename OutputRange, typename Value>
        void subtract(
          OutputRange& output, Value value,
          typename enable_if<is_arithmetic_scalar<Value> >::type* v=0
                      )
        {
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output),
            out_end=boost::end(output);
          for (; out!=out_end; ++out)
            *out-=value;
        }

        template <typename OutputRange, typename InputRange>
        void subtract(
          OutputRange& output, const InputRange& value,
          typename disable_if<is_arithmetic_scalar<InputRange> >::type* v=0
                      )
        {
          typename boost::range_mutable_iterator<OutputRange>::type
            out=boost::begin(output),
            out_end=boost::end(output);

          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(value);
          for (; out!=out_end; ++out, ++in)
            *out-=*in;
        }


        template <typename Value>
        void add(
          Value& output, Value value,
          typename enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          output+=value;
        }

        template <typename OutputRange, typename Value>
        void add(
          OutputRange& output, Value value,
          typename enable_if<is_arithmetic_scalar<Value> >::type* v=0
                      )
        {
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output),
            out_end=boost::end(output);
          for (; out!=out_end; ++out)
            *out+=value;
        }

        template <typename OutputRange, typename InputRange>
        void add(
          OutputRange& output, const InputRange& value,
          typename disable_if<is_arithmetic_scalar<InputRange> >::type* v=0
                      )
        {
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output),
            out_end=boost::end(output);

          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(value);
          for (; out!=out_end; ++out, ++in)
            *out+=*in;
        }




      template <typename Value, typename Weight>
      void multiply(
        Value& output, Weight weight, Value input,
        typename boost::enable_if<is_arithmetic_scalar<Value> >::type* v=0)
      {
        output=weight*input;
      }

      template <typename OutputRange, typename Value, typename InputRange>
      void multiply(
        OutputRange& output, Value weight,
        const InputRange& input,
        typename boost::disable_if<is_arithmetic_scalar<InputRange> >::type* v=0)
      {
        typename boost::range_iterator<OutputRange>::type
          out=boost::begin(output);
        typename boost::range_const_iterator<OutputRange>::type
          in=boost::begin(input),
          in_end=boost::end(input);
        for (; in!=in_end; ++out, ++in)
          *out=weight*(*in);
      }

        template <typename Value, typename Weight>
        void multiply(Value& output, Weight weight,
          typename enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          output*=weight;
        }

        template <typename OutputRange, typename Value>
        void multiply(
          OutputRange& output, Value weight,
          typename disable_if<is_arithmetic_scalar<OutputRange> >::type* v=0)
        {
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output),
            out_end=boost::end(output);
          for (; out!=out_end; ++out)
            *out*=weight;
        }


        template <typename Value, typename Weight>
        void multiply_abs_difference(
          Value& output, Weight weight, Value input1, Value input2,
          typename boost::enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          using namespace std;
          output=weight*(abs)(input1-input2);
        }

        template <typename OutputRange, typename Weight,
            typename InputRange1, typename InputRange2>
        void multiply_abs_difference(
          OutputRange& output, Weight weight,
          const InputRange1& input, const InputRange2& input2,
          typename boost::disable_if<is_arithmetic_scalar<InputRange1> >::type* v=0)
        {
          using namespace std;
          typename boost::range_mutable_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(input2);
          for (; in!=in_end; ++out, ++in, ++in2)
            *out=weight*(abs)(*in-*in2);
        }


        template <typename Value, typename Weight>
        void acc_mult_sum_abs_diff(
          Value& output, Weight weight,
          Value input1, Value input2, Value input3,
          typename boost::enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          using namespace std;
          output+=weight*((abs)(input1-input3)+(abs)(input2-input3));
        }

        template <typename OutputRange, typename InputRange1,
            typename InputRange2, typename InputRange3>
        void acc_mult_sum_abs_diff(
          OutputRange& output, double weight,
          const InputRange1& input1, const InputRange2& input2,
          const InputRange3& input3,
          typename boost::disable_if<is_arithmetic_scalar<InputRange1> >::type* v=0 )
        {
          using namespace std;
          typename boost::range_mutable_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(input1),
            in_end=boost::end(input1);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(input2);
          typename boost::range_const_iterator<InputRange3>::type
            in3=boost::begin(input3);

          for (; in!=in_end; ++out, ++in, ++in2, ++in3)
            *out+=weight*((abs)(*in-*in3)+(abs)(*in2-*in3));
        }



        template <typename Value>
        void accumulate_difference(Value& output, Value input1, Value input2,
                                   Value dummy)
        {
          using namespace std;
          output+=input1-input2;
        }

        template <
            typename OutputRange,
            typename InputRange1, typename InputRange2,
            typename Value>
        void accumulate_difference(
          OutputRange& output,
          const InputRange1& input1, const InputRange2& input2,
          Value dummy)
        {
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(input1),
            in_end=boost::end(input1);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(input2);
          for (; in!=in_end; ++out, ++in, ++in2)
            *out+=*in-*in2;
        }




        template <typename Value, typename Weight>
        void abs_mult_diff(
          Value& output, Weight weight,
          Value input1, Value input2,
          typename boost::enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          using namespace std;
          output=abs(weight*(input1-input2));
        }

        template <typename OutputRange,
            typename InputRange1, typename InputRange2>
        void abs_mult_diff(
          OutputRange& output, double weight,
          const InputRange1& input1, const InputRange2& input2,
          typename boost::disable_if<
          is_arithmetic_scalar<InputRange1> >::type* v=0)
        {
          using namespace std;
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<InputRange1>::type
            in=boost::begin(input1),
            in_end=boost::end(input1);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(input2);

          for (; in!=in_end; ++out, ++in, ++in2)
            *out=(abs)(weight*(*in-*in2));
        }


        template <typename Value>
        void sum(
          Value& output, Value input1, Value input2,
          typename boost::enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          output=input1+input2;
        }

        template <typename OutputRange,
            typename InputRange1, typename InputRange2>
        void sum(OutputRange& output,
                 const InputRange1& input1, const InputRange2& input2,
                 typename boost::disable_if<
                 is_arithmetic_scalar<InputRange1> >::type* v=0 )
        {
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<OutputRange>::type
            in=boost::begin(input1),
            in2=boost::begin(input2),
            in_end=boost::end(input1);
          for (; in!=in_end; ++out, ++in, ++in2)
            *out=(*in)+(*in2);
        }


        template <typename Value>
        void accumulate_weighted(Value& output, Value weight, Value input)
        {
          output+=weight*input;
        }

        template <typename OutputRange, typename Value, typename InputRange>
        void accumulate_weighted(OutputRange& output,
                                 Value weight, const InputRange& input)
        {
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          for (; in!=in_end; ++out, ++in)
            *out+=weight*(*in);
        }

        template <typename Value>
        void acc_weighted_sum_abs(Value& output, Value weight,
                                  Value input1, Value input2)
        {
          using namespace std;
          output+=weight*((abs)(input1)+(abs)(input2));
        }

        template <typename OutputRange, typename Value, typename InputRange>
        void acc_weighted_sum_abs(
          OutputRange& output, Value weight,
          const InputRange& input1, const InputRange& input2)
        {
          using namespace std;
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(input1),
            in2=boost::begin(input2),
            in_end=boost::end(input1);
          for (; in!=in_end; ++out, ++in, ++in2)
            *out+=weight*((abs)(*in)+(abs)(*in2));
        }



        template <typename Value>
        void accumulate(Value& output, Value input, Value dummy)
        {
          output+=input;
        }

        template <typename OutputRange, typename InputRange, typename Value>
        void accumulate(OutputRange& output, const InputRange& input,
                        Value dummy)
        {
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          for (; in!=in_end; ++out, ++in)
            *out+=*in;
        }



//! absolute value
        template <typename Value>
        void absval(
          Value& out, Value in,
          typename boost::enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          using namespace std;
          out=abs(in);
        }

//! absolute value
        template <typename OutputRange, typename InputRange>
        void absval(
          OutputRange& output, const InputRange& input,
          typename boost::disable_if<
          is_arithmetic_scalar<InputRange> >::type* v=0 )
        {
          using namespace std;
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output);
          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          for (; in!=in_end; ++out, ++in)
            *out=abs(*in);
        }

        template <typename Value, typename Value2>
        void assign(
          Value& out, Value2 in,
          typename boost::enable_if<
          mpl::and_<is_arithmetic_scalar<Value>,is_arithmetic_scalar<Value2> >
          >::type*v=0)
        {
          using namespace std;
          out=in;
        }

        template <typename OutputRange, typename InputRange>
        void assign(
          OutputRange& output, const InputRange& input,
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
            *out=*in;
        }

        template <typename OutputRange, typename Value>
        void assign(
          OutputRange& output, Value input,
          typename boost::disable_if<
          boost::mpl::or_<
          boost::mpl::not_< quadrature::is_arithmetic_scalar<Value> >,
          quadrature::is_arithmetic_scalar<OutputRange>,
          boost::mpl::not_< quadrature::is_arithmetic_scalar_<
          boost::range_value<OutputRange> > > > >::type*v=0)
        {
          using namespace std;
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output),
            out_end=boost::end(output);
          for (; out!=out_end; ++out)
            *out=input;
        }

        template <typename OutputRange, typename Value>
        void assign(
          OutputRange& output, Value input,
          typename boost::disable_if<
          boost::mpl::or_<boost::mpl::not_< quadrature::is_arithmetic_scalar<Value> >,
          quadrature::is_arithmetic_scalar<OutputRange>,
          quadrature::is_arithmetic_scalar_<boost::range_value<OutputRange> >
          > >::type*v=0)
        {
          using namespace std;
          typename boost::range_iterator<OutputRange>::type
            out=boost::begin(output),
            out_end=boost::end(output);

          typedef typename boost::range_iterator<
          typename boost::range_value<OutputRange>::type >::type iter;

          for (; out!=out_end; ++out)
            for (iter i=out->begin(), i_end=out->end(); i!=i_end; ++i)
              *i=input;
        }



        template <typename Value>
        bool equal(Value in, Value value)
        {
          return in==value;
        }

        //! compare each element of a vector to a constant
        template <typename InputRange, typename Value>
        bool equal(
          const InputRange& input, Value value,
          typename enable_if<
          mpl::and_<is_arithmetic_scalar<Value>,
          mpl::not_<is_arithmetic_scalar<InputRange> > >
              >::type*v=0)
        {
          using namespace std;
          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          for (; in!=in_end; ++in)
            if (*in!=value)
              return false;
          return true;
        }

        //! compare each element of a vector to a constant
        template <typename InputRange, typename InputRange2>
        bool equal(
          const InputRange& input, const InputRange2& value,
          typename disable_if<
          mpl::or_<is_arithmetic_scalar<InputRange>,
          is_arithmetic_scalar<InputRange2> > >::type*v=0)
        {
          using namespace std;
          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(value);
          for (; in!=in_end; ++in, ++in2)
            if (*in!=*in2)
              return false;
          return true;
        }


        template <typename Value>
        bool less_than(
          Value in, Value value,
          typename enable_if< is_arithmetic_scalar<Value> >::type*v=0)
        {
          return in<value;
        }

        //! compare each element of a vector to a constant
        template <typename InputRange, typename Value>
        bool less_than(const InputRange& input, Value value,
          typename enable_if<
                       mpl::and_<is_arithmetic_scalar<Value>,
                       mpl::not_<is_arithmetic_scalar<InputRange> > >
                       >::type*v=0)
        {
          using namespace std;
          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          for (; in!=in_end; ++in)
            if (*in>value)
              return false;
          return true;
        }

        //! compare each element of a vector to a constant
        template <typename InputRange, typename InputRange2>
        bool less_than(
          const InputRange& input, const InputRange2& value,
          typename disable_if<
          mpl::or_<is_arithmetic_scalar<InputRange>,
          is_arithmetic_scalar<InputRange2> > >::type*v=0)
        {
          using namespace std;
          typename boost::range_const_iterator<InputRange>::type
            in=boost::begin(input),
            in_end=boost::end(input);
          typename boost::range_const_iterator<InputRange2>::type
            in2=boost::begin(value);
          for (; in!=in_end; ++in, ++in2)
            if (*in>*in2)
              return false;
          return true;
        }


        template <typename Value>
        bool isnan(Value value,
                   typename enable_if<is_arithmetic_scalar<Value> >::type* v=0)
        {
          return boost::math::isnan(value);
        }

        template <typename Range>
        bool isnan(
          Range value,
          typename disable_if<
          mpl::or_<is_arithmetic_scalar<Range>,
          mpl::not_<is_arithmetic_scalar_<boost::range_value<Range> > > > >::type* v=0)
        {
          for (typename boost::range_const_iterator<Range>::type
                 i=boost::begin(value), i_end=boost::end(value);
               i!=i_end; ++i)
            if (boost::math::isnan(*i))
              return true;
          return false;
        }

        template <typename Range>
        bool isnan(
          Range value,
          typename disable_if<
          mpl::or_<
          is_arithmetic_scalar<Range>,
          is_arithmetic_scalar_<boost::range_value<Range> > > >::type* v=0)
        {
          for (typename boost::range_const_iterator<Range>::type
                 i=boost::begin(value), i_end=boost::end(value);
               i!=i_end; ++i)
            for (typename boost::range_const_iterator<typename boost::range_value<Range>::type>::type
                   j=boost::begin(*i), j_end=boost::end(*i);
                 j!=j_end; ++j)
              if (boost::math::isnan(*j))
                return true;
          return false;
        }



        template <typename Integrand, typename Domain, typename Image,
          typename Recorder>
        void eval(
          const Integrand& f, const Domain x, Image& y,
          Recorder& recorder,
          typename boost::enable_if<is_arithmetic_scalar<Image> >::type* dummy=0
                  )
        {
          y=f(x);
          recorder(x,y);
          BOOST_QUADRATURE_ASSERT(!detail::isnan(y));
        }

        template <typename Integrand, typename Domain, typename Image,
          typename Recorder>
        void eval(
          const Integrand& f, const Domain x, Image& y,
          Recorder& recorder,
          typename boost::disable_if<is_arithmetic_scalar<Image> >::type* dummy=0
                  )
        {
          f(x,y);
          recorder(x,y);
          BOOST_QUADRATURE_ASSERT(!detail::isnan(y));
        }

      }// namespace

    }// namespace
  }// namespace
}// namespace

#endif
