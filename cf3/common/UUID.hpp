// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_UUID_hpp
#define cf3_common_UUID_hpp

#include <boost/uuid/uuid.hpp>

#include "common/CF.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/// Universal Unique identifier, based on a single per-applications instance boost::uuid
/// This class avoids excessive calls to the random number generator
class Common_API UUID
{
public:
  /// Default constructor
  UUID();

  /// String representation of the UUID
  std::string string() const;

  /// The root boost::uuid that the UUIDs are based on
  const boost::uuid& root() const;

  /// The local offset for the UUID
  Uint offset() const;

private:
  const boost::uuid m_uuid;
  const Uint m_offset;
};

} // common
} // cf3

#endif // cf3_common_UUID_hpp
