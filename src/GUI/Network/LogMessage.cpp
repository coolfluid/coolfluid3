// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp> // for map_list_of

#include "GUI/Network/LogMessage.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::GUI::Network;

////////////////////////////////////////////////////////////////////////////////

LogMessage::Convert::FwdMap_t LogMessage::Convert::all_fwd = boost::assign::map_list_of
  ( LogMessage::INVALID,   "INVALID" )
  ( LogMessage::INFO,      "Info")
  ( LogMessage::ERROR,     "Error")
  ( LogMessage::EXCEPTION, "Exception")
  ( LogMessage::WARNING,   "Warning");

LogMessage::Convert::BwdMap_t LogMessage::Convert::all_rev = boost::assign::map_list_of
  ("INVALID",     LogMessage::INVALID)
  ("Info",    LogMessage::INFO)
  ("Error", LogMessage::ERROR)
  ("Exception",     LogMessage::EXCEPTION)
  ("Warning",     LogMessage::WARNING)
;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const LogMessage::Type& in )
{
  os << LogMessage::Convert::to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, LogMessage::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = LogMessage::Convert::to_enum(tmp);
  return is;
}


////////////////////////////////////////////////////////////////////////////////
