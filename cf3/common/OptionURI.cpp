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

#include "common/OptionFactory.hpp"
#include "common/OptionURI.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////


OptionURI::OptionURI(const std::string & name, const URI & def) :
  OptionT<URI>(name,  def)
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

void OptionURI::change_value_impl(const boost::any& value)
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

    m_value = value;
  }
  catch(boost::bad_any_cast& e)
  {
    throw CastingFailed(FromHere(), "Expected a URI, got a " + demangle(value.type().name()));
  }
}

//////////////////////////////////////////////////////////////////////////////

std::string OptionURI::restricted_list_str() const
{
  std::vector<URI> restr_list_vec;
  BOOST_FOREACH(const boost::any& restr_item, restricted_list())
  {
    restr_list_vec.push_back(boost::any_cast<URI>(restr_item));
  }
  return option_vector_to_str(restr_list_vec, separator());
}


//////////////////////////////////////////////////////////////////////////////

class OptionURIBuilder : public OptionBuilder
{
public:
  virtual boost::shared_ptr< Option > create_option(const std::string& name, const boost::any& default_value)
  {
    const URI val = from_str<URI>(boost::any_cast<std::string>(default_value));
    return boost::shared_ptr<Option>(new OptionURI(name, val));
  }
};

RegisterOptionBuilder option_uri_builder(common::class_name<URI>(), new OptionURIBuilder());

//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

