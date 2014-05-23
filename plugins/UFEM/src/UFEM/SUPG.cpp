// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp> // for map_list_of

#include "SUPG.hpp"

namespace cf3 {

namespace UFEM {

SUPGTypes::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
  (SUPGTypes::TEZDUYAR, "tezduyar")
  (SUPGTypes::METRIC, "metric")
  (SUPGTypes::CF2, "cf2");
  
  all_rev = boost::assign::map_list_of
  ("tezduyar", SUPGTypes::TEZDUYAR)
  ("metric", SUPGTypes::METRIC)
  ("cf2", SUPGTypes::CF2);
}

SUPGTypes::Convert& SUPGTypes::Convert::instance()
{
  static SUPGTypes::Convert instance;
  return instance;
}

} // UFEM
} // cf3

