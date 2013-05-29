// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_BOOSTMATH_HPP
#define cf3_Math_BOOSTMATH_HPP

#include "common/CF.hpp"

#if (defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__))
#  ifndef LDBL_MANT_DIG
#    define LDBL_MANT_DIG -1 // avoid anoying warning in boost 1.42 on APPLE
#  endif
#endif

// for is_finite() is_nan() is_inf()
#include <boost/math/special_functions/fpclassify.hpp>

// for erfc
#include <boost/math/special_functions/erf.hpp>

#endif // cf3_Math_BOOSTMATH_HPP
