// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_Integrate_hpp
#define cf3_Math_Integrate_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/LibMath.hpp"

#define BOOST_QUADRATURE_DEBUG
#include <boost/numeric/quadrature/quadrature_config.hpp>
#include <boost/numeric/quadrature/adaptive.hpp>
#include <boost/numeric/quadrature/epsilon.hpp>
#include <boost/numeric/quadrature/kronrodgauss.hpp>

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {

////////////////////////////////////////////////////////////////////////////////

class Math_API Integrate
{
public:

  template <typename F>
  Real operator()(const F& f, const Real& from, const Real& to)
  {
    Real answer;
    boost::numeric::quadrature::adaptive().
        accelerator(epsilon).
        info(info)
        (f, from, to, answer, error_estimate);
    nb_intervals = info.num_intervals();
    nb_kernel_evaluations = info.num_kernel_evaluations();
    return answer;
  }

private:

  // declare Wynn's epsilon algorithm
  boost::numeric::quadrature::wynn_epsilon_algorithm<double> epsilon;

  // declare the info data
  boost::numeric::quadrature::adaptive_info info;

public:
  Real error_estimate;
  Uint nb_intervals;
  Uint nb_kernel_evaluations;
};

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Math_Integrate_hpp

