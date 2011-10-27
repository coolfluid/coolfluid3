// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>
#include <boost/weak_ptr.hpp>

#include "common/URI.hpp"
#include "python/URIWrapper.hpp"

namespace cf3 {
namespace python {

using namespace boost::python;

void def_uri()
{
  scope in_uri = class_<common::URI>("URI", "Coolfluid URI class wrapper")
    .def(init<std::string>())
    .def(init<std::string, common::URI::Scheme::Type>())
    .def("__str__", &common::URI::string);

  enum_<common::URI::Scheme::Type>("Scheme")
    .value(common::URI::Scheme::Convert::instance().to_str(common::URI::Scheme::HTTP).c_str(), common::URI::Scheme::HTTP)
    .value(common::URI::Scheme::Convert::instance().to_str(common::URI::Scheme::HTTPS).c_str(), common::URI::Scheme::HTTPS)
    .value(common::URI::Scheme::Convert::instance().to_str(common::URI::Scheme::CPATH).c_str(), common::URI::Scheme::CPATH)
    .value(common::URI::Scheme::Convert::instance().to_str(common::URI::Scheme::FILE).c_str(), common::URI::Scheme::FILE);
}

} // python
} // cf3
