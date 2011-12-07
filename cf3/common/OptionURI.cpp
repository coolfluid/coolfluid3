// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include<sstream>

#include <boost/foreach.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/StringConversion.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/Protocol.hpp"

#include "common/XML/FileOperations.hpp"

#include "common/OptionURI.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////


OptionURI::OptionURI(const std::string & name, const URI & def) :
  OptionT(name,  def)
{
}

///////////////////////////////////////////////////////////////////////////////

OptionURI::~OptionURI()
{
}

///////////////////////////////////////////////////////////////////////////////

OptionURI& OptionURI::supported_protocol(URI::Scheme::Type protocol)
{
  if(std::find(m_protocols.begin(), m_protocols.end(), protocol) == m_protocols.end())
    m_protocols.push_back(protocol);

  return *this;
}

//////////////////////////////////////////////////////////////////////////////

void OptionURI::change_value(const boost::any& value)
{
  try
  {
    const URI val = boost::any_cast<URI>(value);

    if(std::count(m_protocols.begin(), m_protocols.end(), val.scheme() == 0))
    {
      std::stringstream error_str;
      error_str << "Protocol " + URI::Scheme::Convert::instance().to_str(val.scheme()) + " is not supported. Accepted values are:";
      BOOST_FOREACH(const URI::Scheme::Type scheme, m_protocols)
      {
        error_str << " " << URI::Scheme::Convert::instance().to_str(scheme);
      }
      throw BadValue(FromHere(), error_str.str());
    }

    Option::change_value(value);
  }
  catch(boost::bad_any_cast& e)
  {
    throw CastingFailed(FromHere(), "Expected a URI, got a " + demangle(value.type().name()));
  }
}


//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

