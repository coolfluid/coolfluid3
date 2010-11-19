// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CLibrary_hpp
#define CF_Common_CLibrary_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  class LibraryRegisterBase;

/////////////////////////////////////////////////////////////////////////////////

  /// @brief Component class for a library
  /// @author Quentin Gasper
  class Common_API CLibrary : public Component
  {
  public:

    typedef boost::shared_ptr<CLibrary> Ptr;
    typedef boost::shared_ptr<CLibrary const> ConstPtr;

    /// @brief Contructor
    /// @param lib_name Library name
    CLibrary(const std::string & lib_name);

    /// @brief Virtual destructor.
    virtual ~CLibrary();

    void set_library(LibraryRegisterBase * lib);

    /// Get the class name
    static std::string type_name() { return "CLibrary"; }

    /// Configuration properties
    static void defineConfigProperties ( CF::Common::PropertyList& props );

  private: // methods

    /// regists all the signals declared in this class
    static void regist_signals ( CLibrary* self ) { }

  private: // data

      LibraryRegisterBase * m_library;

  }; // CLibrary

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////


#endif // CF_Common_CLibrary_hpp
