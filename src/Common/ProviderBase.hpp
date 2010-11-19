// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef COOFluiD_Common_ProviderBase_hpp
#define COOFluiD_Common_ProviderBase_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class for provider types
/// @author Dries Kimpe
/// @author Tiago Quintino
class Common_API ProviderBase : public boost::noncopyable {

public: // methods

  /// Constructor
  ProviderBase ();

  /// Virtual destructor
  virtual ~ProviderBase ();

  /// @return the name of this provider
  virtual std::string provider_name () const = 0;

  /// @return the type of this provider
  virtual std::string provider_type () const = 0;

}; // end ProviderBase

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // COOFluiD_Common_ProviderBase_hpp
