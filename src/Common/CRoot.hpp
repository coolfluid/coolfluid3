// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

  class NotificationQueue;

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CRoot : public Component {

  public: // typedefs

    typedef boost::shared_ptr<CRoot> Ptr;
    typedef boost::shared_ptr<CRoot const> ConstPtr;

  public: // functions

    /// Get the class name
    static CRoot::Ptr create ( const std::string& name );

    /// Virtual destructor
    virtual ~CRoot();

    /// Get the class name
    static std::string type_name () { return "CRoot"; }

    // functions specific to the CRoot component

    /// Access the component described by the path
    /// The path should be absolute
    /// @param path to the component
    /// @return pointer to Component, or null if no component was found
    Component::Ptr retrieve_component ( const URI& path );

    /// Access the component described by the path
    /// The path should be absolute
    /// @throws InvalidURI if no component can be found at the path
    /// @post never returns nullptr
    /// @param path to the component
    /// @return pointer to Component
    Component::Ptr retrieve_component_checked ( const URI& path );

    /// Access the component described by the path
    /// The path should be absolute
    /// @throws InvalidURI if no component can be found at the path
    /// @param path to the component
    /// @returns nullptr if cannot cast to TYPE::Ptr
    /// @return pointer to Component cast to the sepcified TYPE
    template < typename TYPE >
        typename TYPE::Ptr retrieve_component ( const URI& path );

    /// Access the component described by the path
    /// The path should be absolute
    /// @throws InvalidURI if no component can be found at the path
    /// @throws CastingFailed if a component is found but cannot be cast to type
    /// @param path to the component
    /// @return pointer to Component cast to the sepcified TYPE
    template < typename TYPE >
        typename TYPE::Ptr retrieve_component_checked ( const URI& path );

    /// define the component path
    /// @param path to the component
    /// @param comp the component for which to define the path
    void change_component_path ( const URI& path, Component::Ptr comp );

    /// remove a component path
    /// @param path to the component
    void remove_component_path ( const URI& path );

    /// check that a component path actually points to a component
    /// @param path to the component
    bool exists_component_path ( const URI& path ) const;

    /// dump to string table of contents of the component list
    /// @return string with list of components in the root
    std::string list_toc () const;

    /// @brief Raises an event.
    /// @param event_name Event name
    /// @param raiser_path Path of the component that raised the event. The path
    /// must exist under this root.
    void raise_new_event ( const std::string & event_name,
                           const URI & raiser_path );

    /// @brief Adds a listener to the events
    /// @param queue The queue object. Cannot be null.
    void add_notification_queue ( NotificationQueue * queue );

  private: // helper functions

    typedef std::map< std::string , Component::Ptr > CompStorage_t;
  protected:
    /// Protected constructor forces creation via the create() funtion while still allowing wrapping by AllocatedComponent
    /// @param name of the component
    CRoot ( const std::string& name );

  private: // data

    /// map the paths to each component
    CompStorage_t  m_toc;

    std::vector<NotificationQueue*> m_notif_queues;

  }; // CRoot

////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
  inline typename TYPE::Ptr CRoot::retrieve_component ( const URI& path )
  {
    return boost::dynamic_pointer_cast<TYPE>( this->retrieve_component (path) );
  }

////////////////////////////////////////////////////////////////////////////////

  template < typename TYPE >
  inline typename TYPE::Ptr CRoot::retrieve_component_checked ( const URI& path )
  {
    typename TYPE::Ptr comp = boost::dynamic_pointer_cast<TYPE>( this->retrieve_component (path) );
    if( is_null(comp) )
      throw CastingFailed( FromHere(), "Retrieved component cannot be cast to type " + TYPE::type_name() );
    return comp;
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CRoot_hpp
