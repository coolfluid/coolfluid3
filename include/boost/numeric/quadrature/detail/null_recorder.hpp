#ifndef BOOST_NUMERIC_QUADRATURE_NULLRECORDER_HPP
#define BOOST_NUMERIC_QUADRATURE_NULLRECORDER_HPP

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! A null recorder
  @file   null_recorder.hpp
  @brief  A Recorder that does nothing
*/

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {

      namespace detail
      {
        //! null recorder
        /** A recorder that does nothing.  This is used to provide a
            default argument when a recorder is required.
        */
        struct null_recorder
        {
          template <typename Domain, typename Image>
          void operator()(const Domain&, const Image&) const
          {}
          static null_recorder& instance()
          {
            static null_recorder inst;
            return inst;
          }

        };
      }// namespace
    }// namespace
  }// namespace
}// namespace

#endif
