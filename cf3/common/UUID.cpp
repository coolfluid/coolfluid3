// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "common/Assertions.hpp"
#include "common/UUID.hpp"

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

  inline Uint static_offset()
  {
    static Uint offset = 0;
    ++offset;
    cf3_always_assert(offset); // Bail out if there is a UUID overlap
    return offset;
  }
}

UUID::UUID() : m_uuid(detail::static_offset()), m_offset(detail::static_offset())
{
}

std::string UUID::string() const
{
  return boost::lexical_cast<std::string>(m_uuid) + ":" + boost::lexical_cast<std::string>(m_offset);
}


const boost::uuid& UUID::root() const
{
  return m_uuid;
}


Uint UUID::offset() const
{
  return m_offset;
}



} // common
} // cf3
