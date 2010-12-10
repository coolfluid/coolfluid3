// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_URIProtocol_hpp
#define CF_Common_URIProtocol_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CommonAPI.hpp"
#include "Common/EnumT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common{

////////////////////////////////////////////////////////////////////////////////

class Common_API URIProtocol
{
  public:

  /// Enumeration of the Shapes recognized in CF
  enum Type  { INVALID = -1,
               HTTP    = 0,
               HTTPS   = 1,
               CPATH   = 2,
               FILE    = 3 };

  typedef Common::EnumT< URIProtocol > ConverterBase;

  struct Common_API Convert : public ConverterBase
  {
    /// storage of the enum forward map
    static ConverterBase::FwdMap_t all_fwd;
    /// storage of the enum reverse map
    static ConverterBase::BwdMap_t all_rev;
  };

}; // class URIProtocol

////////////////////////////////////////////////////////////////////////////////

Common_API std::ostream& operator<< ( std::ostream& os, const URIProtocol::Type& in );
Common_API std::istream& operator>> ( std::istream& is, URIProtocol::Type& in );

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_URIProtocol_hpp
