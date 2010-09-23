// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UnknownType_h
#define CF_GUI_Client_UnknownType_h

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

  class ClientCore_API UnknownTypeException : public CF::Common::Exception
  {
    public:

    /// Constructor
    UnknownTypeException(const CF::Common::CodeLocation& where,
    const std::string& what);

    /// Copy constructor
    UnknownTypeException(const UnknownTypeException& e) throw ();


  }; // class UnknownType

  /////////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UnknownType_h
