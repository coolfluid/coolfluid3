// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_UUCount_hpp
#define cf3_common_UUCount_hpp

#include <boost/uuid/uuid.hpp>

#include "common/CF.hpp"
#include "common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/// Combination of a UUID and a count, together forming the equivalent of a 192-bit identifier
class Common_API UUCount
{
public:
  /// Default constructor. Repetitive calls to this constructer always generate UUCounts with the same UUID, unique for the process,
  /// but increment the counter each time
  UUCount();
  
  /// Construct from a string, formatted as UUID:Count. Useful for deserialization.
  UUCount(const std::string& str);

  /// String representation of the UUCount
  /// Format is UUID:Count
  std::string string() const;

  /// The UUID part
  const boost::uuids::uuid& uuid() const;

  /// The count part
  Uint count() const;
  
  /// True if the UUID is nil and the count is 0
  bool is_nil() const;
  
  bool operator==(UUCount const& rhs) const;
  bool operator!=(UUCount const& rhs) const;
  bool operator<(UUCount const& rhs) const;
  bool operator>(UUCount const& rhs) const;
  bool operator<=(UUCount const& rhs) const;
  bool operator>=(UUCount const& rhs) const;

private:
  boost::uuids::uuid m_uuid;
  Uint m_count;
};

} // common
} // cf3

#endif // cf3_common_UUCount_hpp
