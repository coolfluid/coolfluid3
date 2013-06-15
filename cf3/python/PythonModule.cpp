// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/BoostPython.hpp"

#include "python/ComponentWrapper.hpp"
#include "python/CoreWrapper.hpp"
#include "python/TableWrapper.hpp"
#include "python/MatrixWrappers.hpp"
#include "python/PythonModule.hpp"
#include "python/URIWrapper.hpp"

namespace cf3 {
namespace python {

using namespace boost::python;

BOOST_PYTHON_MODULE(libcoolfluid_python)
{
  def_component();
  def_core();
  def_ctable_types();
  def_matrix_types();
  def_uri();
  scope().attr("__doc__") = "Provides access to the Coolfluid API from python";
}

} // python
} // cf3
