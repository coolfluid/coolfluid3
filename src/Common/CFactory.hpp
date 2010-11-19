// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_CFactory_hpp
#define CF_Common_CFactory_hpp

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
  class Common_API CFactory : public Component
  {
  public:

    typedef boost::shared_ptr<CFactory> Ptr;
    typedef boost::shared_ptr<CFactory const> ConstPtr;

    /// @brief Contructor
    /// @param lib_name Library name
    CFactory(const std::string & lib_name);

    /// @brief Virtual destructor.
    virtual ~CFactory();

    /// Get the class name
    static std::string type_name() { return "CFactory"; }

    /// Configuration properties
    static void define_config_properties ( CF::Common::PropertyList& props );

  private: // methods

    /// regists all the signals declared in this class
    static void regist_signals ( CFactory* self ) { }

  private: // data

  }; // CFactory

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CFactory_hpp

