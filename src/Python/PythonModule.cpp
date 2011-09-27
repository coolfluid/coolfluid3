// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>

#include "Python/Component.hpp"
#include "Python/Core.hpp"
#include "Python/MatrixTypes.hpp"
#include "Python/PythonModule.hpp"
#include "Python/URI.hpp"

namespace CF {
namespace Python {

using namespace boost::python;

BOOST_PYTHON_MODULE(libcoolfluid_python)
{
  def_component();
  def_core();
  def_matrix_types();
  def_uri();
  scope().attr("__doc__") = "Provides access to the Coolfluid API from python";
}

} // Python
} // CF
