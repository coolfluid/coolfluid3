// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>
#include <boost/weak_ptr.hpp>

#include "common/BasicExceptions.hpp"

#include "math/MatrixTypes.hpp"
#include "python/MatrixWrappers.hpp"

namespace cf3 {
namespace python {

using namespace boost::python;

Real get_item(const RealVector& self, const Uint i)
{
  if(i >= self.size())
    throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for vector of size " + boost::lexical_cast<std::string>(self.size()));
  return self[i];
}

void set_item(RealVector& self, const Uint i, const Real value)
{
  if(i >= self.size())
    throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for vector of size " + boost::lexical_cast<std::string>(self.size()));
  self[i] = value;
}

Uint get_size(const RealVector& self)
{
  return self.size();
}

void def_matrix_types()
{
  scope real_vector = class_<RealVector>("RealVector", "Coolfluid RealVector class wrapper")
    .def(init<Uint>())
    .def("__getitem__", get_item)
    .def("__setitem__", set_item)
    .def("__len__", get_size);
}

} // python
} // cf3
