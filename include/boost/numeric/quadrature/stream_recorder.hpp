#ifndef BOOST_NUMERIC_QUADRATURE_STREAM_RECORDER_H
#define BOOST_NUMERIC_QUADRATURE_STREAM_RECORDER_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! A recorder
  @file   stream_recorder.hpp
  @brief  A recorder that records domain, image pairs to a stream
*/

#include <boost/range.hpp>

#include <utility>
#include <iostream>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {

      //! A recorder that writes to a stream
      /** A recorder that writes to a stream
       */
      struct stream_recorder
      {
        stream_recorder(std::ostream& stream)
            : m_stream(stream)
        {}

        template <typename Domain, typename Image>
        void operator()(const Domain& x, const Image& y)
        {
          m_stream << x << " -> " << y << std::endl;
        }

      private:
        std::ostream& m_stream;
      };

    }// namespace
  }// namespace
}// namespace

#endif
