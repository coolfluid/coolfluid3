#ifndef BOOST_NUMERIC_QUADRATURE_CONTAINER_PAIR_RECORDER_H
#define BOOST_NUMERIC_QUADRATURE_CONTAINER_PAIR_RECORDER_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! A recorder
  @file   container_pair_recorder.hpp
  @brief  A Recorder that records domain, image pairs to a container
*/

#include <boost/range.hpp>
#include <boost/numeric/quadrature/detail/local_traits.hpp>

#include <utility>
#include <vector>

namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {

  //! A recorder that stores values in a container
  /** A recorder that stores values in a container that supports push_back.
      Values are stored as a std::pair<Domain,Image>
   */
  template <typename Domain, typename Image,
      typename Container=std::vector<
          std::pair<Domain,typename detail::storage_for_type<Image>::type> >
      >
  struct container_pair_recorder
  {
    typedef typename boost::range_const_iterator<Container>::type const_iterator;
    typedef typename Container::size_type size_type;
    typedef typename detail::storage_for_type<Image>::type image;

    void operator()(const Domain& x, const image& y)
    {
      m_container.push_back(std::make_pair(x,y));
    }

    const_iterator begin() const
    {
      return boost::begin(m_container);
    }

    const_iterator end() const
    {
      return boost::end(m_container);
    }

  private:
    Container m_container;
  };


    }// namespace
  }// namespace
}// namespace

#endif
