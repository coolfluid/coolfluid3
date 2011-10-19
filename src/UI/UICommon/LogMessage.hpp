// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_UICommon_LogMessage_hpp
#define cf3_GUI_UICommon_LogMessage_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/EnumT.hpp"
#include "UI/UICommon/LibUICommon.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace UICommon {

////////////////////////////////////////////////////////////////////////////////

class UICommon_API LogMessage
{
  public:

  /// Enumeration of the worker statuses recognized in CF
  enum Type  { INVALID    =-1,
               INFO       = 0,
               ERROR      = 1,
               EXCEPTION  = 2,
               WARNING    = 3};

  typedef common::EnumT< LogMessage > ConverterBase;

  struct Common_API Convert : public ConverterBase
  {
    /// constructor where all the converting maps are built
    Convert();
    /// get the unique instance of the converter class
    static Convert& instance();
  };


}; // class LogMessage

////////////////////////////////////////////////////////////////////////////////

UICommon_API std::ostream& operator<< ( std::ostream& os, const LogMessage::Type& in );
UICommon_API std::istream& operator>> ( std::istream& is, LogMessage::Type& in );

////////////////////////////////////////////////////////////////////////////////

} // Network
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_UICommon_LogMessage_hpp
