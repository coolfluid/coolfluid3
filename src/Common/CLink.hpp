// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CLink_hpp
#define CF_Common_CLink_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component for creating links between components
  /// @author Tiago Quintino
  class Common_API CLink : public Component {

  public: //typedefs

    typedef boost::shared_ptr<CLink> Ptr;
    typedef boost::shared_ptr<CLink const> ConstPtr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CLink ( const CName& name );

    /// Virtual destructor
    virtual ~CLink();

    /// Get the class name
    static std::string type_name () { return "CLink"; }

    /// Configuration Options
    static void define_config_properties ( Common::PropertyList& options ) {}

    /// get the component through the links to the actual components
    virtual Component::Ptr get ();
    virtual Component::ConstPtr  get() const;

    // functions specific to the CLink component

    /// link to component
    void link_to ( Component::Ptr lnkto );

    void change_link( XmlNode & node );

  private: // helper functions

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  private: // data

    /// this is a link to the component
    /// using weak_ptr means it might become invalid so we should test for expire()
    boost::weak_ptr<Component> m_link_component;

  }; // CLink

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CLink_hpp
