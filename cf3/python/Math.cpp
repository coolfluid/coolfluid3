// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/math/special_functions/next.hpp>

#include "common/CF.hpp"
#include "python/BoostPython.hpp"

namespace cf3 {
namespace python {

struct Math
{
  static inline Real float_distance(const Real left, const Real right)
  {
    return boost::math::float_distance(left, right);
  }
};

void def_math()
{
  boost::python::class_<Math>("math", "Mathemathical functions", boost::python::no_init)
    .def("float_distance", Math::float_distance, "float_distance function from boost")
    .staticmethod("float_distance");
}


} // python
} // cf3
