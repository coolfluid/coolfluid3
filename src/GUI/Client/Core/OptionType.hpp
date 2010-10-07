// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_OptionType_hpp
#define CF_GUI_Client_Core_OptionType_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/EnumT.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

  class ClientCore_API OptionType
  {
  public:

    /// Enumeration of the worker statuses recognized in CF
    enum Type  {
      /// @brief Invalid type.
      INVALID = -1,

      /// @brief Boolean type.
      TYPE_BOOL = 0,

      /// @brief Signed integer type.
      TYPE_INT = 1,

      /// @brief Integer type.
      TYPE_UNSIGNED_INT = 2,

      /// @brief Real type.
      TYPE_DOUBLE = 3,

      /// @brief String type.
      TYPE_STRING = 4,

      /// @brief Files type.
      TYPE_FILES = 5,

      /// @brief Libraries type.
      TYPE_LIBRARIES = 6,

      /// @brief Array type
      TYPE_ARRAY = 7,

      TYPE_PATH = 8
    };

    typedef Common::EnumT< OptionType > ConverterBase;

    struct ClientCore_API Convert : public ConverterBase
    {
      /// storage of the enum forward map
      static ConverterBase::FwdMap_t all_fwd;
      /// storage of the enum reverse map
      static ConverterBase::BwdMap_t all_rev;
    };

  }; // class OptionType

////////////////////////////////////////////////////////////////////////////////

  std::ostream& operator<< ( std::ostream& os, const OptionType::Type& in );
  std::istream& operator>> ( std::istream& is, OptionType::Type& in );

////////////////////////////////////////////////////////////////////////////////

} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Network_ComponentType_hpp
