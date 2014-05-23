// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/assign/list_of.hpp> // for map_list_of

#include "ui/uicommon/LogMessage.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace uiCommon {

////////////////////////////////////////////////////////////////////////////////

LogMessage::Convert& LogMessage::Convert::instance()
{
  static LogMessage::Convert instance;
  return instance;
}

LogMessage::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
  ( LogMessage::INVALID,   "INVALID" )
  ( LogMessage::INFO,      "Info")
  ( LogMessage::ERROR,     "Error")
  ( LogMessage::EXCEPTION, "Exception")
  ( LogMessage::WARNING,   "Warning");

  all_rev = boost::assign::map_list_of
  ("INVALID",     LogMessage::INVALID)
  ("Info",    LogMessage::INFO)
  ("Error", LogMessage::ERROR)
  ("Exception",     LogMessage::EXCEPTION)
  ("Warning",     LogMessage::WARNING);
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const LogMessage::Type& in )
{
  os << LogMessage::Convert::instance().to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, LogMessage::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = LogMessage::Convert::instance().to_enum(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////

} // uiCommon
} // ui
} // cf3
