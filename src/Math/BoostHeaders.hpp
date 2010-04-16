#ifndef CF_Math_BOOSTMATH_HPP
#define CF_Math_BOOSTMATH_HPP

#include "Common/CF.hpp"

#if (defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__))
#  define LDBL_MANT_DIG -1 // avoid anoying warning in boost 1.42 on APPLE
#endif

// for finite() isnan() isinf()
#include <boost/math/special_functions/fpclassify.hpp>

#ifdef CF_HAVE_BOOST_ERFC
#include <boost/math/special_functions/erf.hpp>
#endif

#endif // CF_Math_BOOSTMATH_HPP
