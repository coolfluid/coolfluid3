// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CRoot_hpp
#define CF_Common_CRoot_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CRoot : public Component {

  public: // typedefs

    typedef boost::shared_ptr<CRoot> Ptr;
    typedef boost::shared_ptr<CRoot const> ConstPtr;

  public: // functions

    /// Get the class name
    static CRoot::Ptr create ( const CName& name );

    /// Virtual destructor
    virtual ~CRoot();

    /// Get the class name
    static std::string type_name () { return "CRoot"; }

    /// Configuration Options
    static void defineConfigProperties ( Common::PropertyList& options ) {}

    // functions specific to the CRoot component

    /// Access the component described by the path
    /// The path should be absolute
    /// @param path to the component
    /// @return pointer to Component
    Component::Ptr access_component ( const CPath& path );

    /// Access the component described by the path
    /// The path should be absolute
    /// @param path to the component
    /// @return pointer to Component cast to the sepcified TYPE
    template < typename TYPE >
        typename TYPE::Ptr access_component ( const CPath& path );

    /// define the component path
    /// @param path to the component
    /// @param comp the component for which to define the path
    void change_component_path ( const CPath& path, Component::Ptr comp );

    /// remove a component path
    /// @param path to the component
    void remove_component_path ( const CPath& path );

    /// check that a component path actually points to a component
    /// @param path to the component
    bool exists_component_path ( const CPath& path );

  private: // helper functions

    typedef std::map< std::string , Component::Ptr > CompStorage_t;

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

    /// Private constructor forces creation via the create() funtion
    /// @param name of the component
    CRoot ( const CName& name );

  private: // data

    /// map the paths to each component
    CompStorage_t  m_toc;

  }; // CRoot

////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
  inline typename TYPE::Ptr CRoot::access_component ( const CPath& path )
  {
    return boost::dynamic_pointer_cast<TYPE>( this->access_component (path) );
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CRoot_hpp
