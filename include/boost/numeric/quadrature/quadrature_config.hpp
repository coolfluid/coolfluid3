#ifndef BOOST_NUMERIC_QUADRATURE_QUADRATURE_CONFIG_H
#define BOOST_NUMERIC_QUADRATURE_QUADRATURE_CONFIG_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*!
  @file   quadrature_config.hpp
  @brief  Config for quadrature routines
*/

#include <boost/config.hpp>

#if !defined(NDEBUG) && !defined(BOOST_QUADRATURE_NDEBUG)
#define BOOST_QUADRATURE_DEBUG
#endif

#ifdef BOOST_QUADRATURE_DEBUG
#define BOOST_QUADRATURE_INVALID( type ) \
  =std::numeric_limits<type>::quietNaN()
#endif

#ifdef BOOST_QUADRATURE_DEBUG
#define BOOST_QUADRATURE_ASSERT( value ) BOOST_ASSERT( value )
#endif

#ifndef BOOST_QUADRATURE_INVALID
#define BOOST_QUADRATURE_INVALID( type )
#endif

#endif
