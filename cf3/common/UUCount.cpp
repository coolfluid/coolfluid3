// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"
#include "common/UUCount.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

namespace detail
{
  inline const boost::uuids::uuid& static_uuid()
  {
    static boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return uuid;
  }

  inline Uint static_count()
  {
    static Uint offset = 0;
    ++offset;
    cf3_always_assert(offset); // Bail out if there is a UUCount overlap
    return offset;
  }
}

UUCount::UUCount() : m_uuid(detail::static_uuid()), m_count(detail::static_count())
{
}

UUCount::UUCount(const std::string& str)
{
  if(str.empty())
  {
    m_uuid = boost::uuids::nil_uuid();
    m_count = 0;
  }
  else
  {
    std::vector<std::string> split_parts;
    boost::algorithm::split(split_parts, str, boost::algorithm::is_any_of(":"));
    if(split_parts.size() != 2)
      throw ParsingFailed(FromHere(), "Did not find exactly one ':' separating UUID and count when parsing UUID string " + str);
    
    try
    {
      m_uuid = boost::uuids::string_generator()(split_parts.front());
    }
    catch(std::runtime_error&)
    {
      throw ParsingFailed(FromHere(), "Invalid UUID string: " + split_parts.front());
    }
    
    try
    {
      m_count = boost::lexical_cast<Uint>(split_parts.back());
    }
    catch(boost::bad_lexical_cast& e)
    {
      throw ParsingFailed(FromHere(), "Error casting this string to Uint: " + split_parts.back());
    }
  }
}


std::string UUCount::string() const
{
  return boost::uuids::to_string(m_uuid) + ":" + boost::lexical_cast<std::string>(m_count);
}


const boost::uuids::uuid& UUCount::uuid() const
{
  return m_uuid;
}


Uint UUCount::count() const
{
  return m_count;
}

bool UUCount::is_nil() const
{
  return m_count == 0 && m_uuid.is_nil();
}


bool UUCount::operator!=(UUCount const& rhs) const
{
  return this->m_count != rhs.m_count || this->m_uuid != rhs.m_uuid;
}

bool UUCount::operator==(UUCount const& rhs) const
{
  return !(*this != rhs);
}

bool UUCount::operator<(UUCount const& rhs) const
{
  if(this->m_uuid == rhs.m_uuid)
    return this->m_count < rhs.m_count;
  
  return this->m_uuid < rhs.m_uuid;
}

bool UUCount::operator>(UUCount const& rhs) const
{
  return rhs < *this;
}

bool UUCount::operator<=(UUCount const& rhs) const
{
  return !(rhs < *this);
}

bool UUCount::operator>=(UUCount const& rhs) const
{
  return !(*this < rhs);
}

} // common
} // cf3
