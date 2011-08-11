// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>
#include <boost/weak_ptr.hpp>

#include "Common/URI.hpp"
#include "Python/URI.hpp"

namespace CF {
namespace Python {

using namespace boost::python;

void def_uri()
{
  scope in_uri = class_<Common::URI>("URI", "Coolfluid URI class wrapper")
    .def(init<std::string>())
    .def(init<std::string, Common::URI::Scheme::Type>())
    .def("__str__", &Common::URI::string);

  enum_<Common::URI::Scheme::Type>("Scheme")
    .value(Common::URI::Scheme::Convert::instance().to_str(Common::URI::Scheme::HTTP).c_str(), Common::URI::Scheme::HTTP)
    .value(Common::URI::Scheme::Convert::instance().to_str(Common::URI::Scheme::HTTPS).c_str(), Common::URI::Scheme::HTTPS)
    .value(Common::URI::Scheme::Convert::instance().to_str(Common::URI::Scheme::CPATH).c_str(), Common::URI::Scheme::CPATH)
    .value(Common::URI::Scheme::Convert::instance().to_str(Common::URI::Scheme::FILE).c_str(), Common::URI::Scheme::FILE);
}

} // Python
} // CF
