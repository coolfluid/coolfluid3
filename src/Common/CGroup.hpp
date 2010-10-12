// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CGroup_hpp
#define CF_Common_CGroup_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Common/OptionT.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CGroup : public Component {

  public: // typedefs

    /// provider
    typedef Common::ConcreteProvider < CGroup,1 > PROVIDER;
    /// pointer to this type
    typedef boost::shared_ptr<CGroup> Ptr;
    typedef boost::shared_ptr<CGroup const> ConstPtr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CGroup ( const CName& name );

    /// Virtual destructor
    virtual ~CGroup();

    /// Get the class name
    static std::string type_name () { return "CGroup"; }

    /// Configuration Options
    static void defineConfigProperties ( Common::PropertyList& options );
  private: // helper functions

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CGroup_hpp
