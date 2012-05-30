// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionURI_hpp
#define cf3_common_OptionURI_hpp

///////////////////////////////////////////////////////////////////////////////

#include "common/URI.hpp"

#include "common/OptionT.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

  /////////////////////////////////////////////////////////////////////////////

class Common_API OptionURI : public OptionT<URI>
{

public:

  typedef URI value_type;

  OptionURI(const std::string & name, const URI & def);

  virtual ~OptionURI();

  /// Add the supplied protocol type to the list of supported protocols
  /// No effect if the protocol was already registered with this option.
  OptionURI& supported_protocol(URI::Scheme::Type protocol);

  const std::vector<URI::Scheme::Type>& supported_protocols() const { return m_protocols; }

  virtual std::string restricted_list_str() const;

private:
  std::vector<URI::Scheme::Type> m_protocols;
  virtual void change_value_impl(const boost::any& value);
}; // class OptionURI

  /////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

///////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionURI_hpp
