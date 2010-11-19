// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Network_LogMessage_hpp
#define CF_GUI_Network_LogMessage_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/EnumT.hpp"
#include "GUI/Network/LibNetwork.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////

class Network_API LogMessage
{
  public:

  /// Enumeration of the worker statuses recognized in CF
  enum Type  { INVALID    =-1,
               INFO       = 0,
               ERROR      = 1,
               EXCEPTION  = 2,
               WARNING    = 3};

  typedef Common::EnumT< LogMessage > ConverterBase;

  struct Network_API Convert : public ConverterBase
  {
    /// storage of the enum forward map
    static ConverterBase::FwdMap_t all_fwd;
    /// storage of the enum reverse map
    static ConverterBase::BwdMap_t all_rev;
  };

}; // class LogMessage

////////////////////////////////////////////////////////////////////////////////

Network_API std::ostream& operator<< ( std::ostream& os, const LogMessage::Type& in );
Network_API std::istream& operator>> ( std::istream& is, LogMessage::Type& in );

////////////////////////////////////////////////////////////////////////////////

} // Network
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Network_LogMessage_hpp
