// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Link_hpp
#define cf3_common_Link_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// Component for creating links between components
/// @author Tiago Quintino
class Common_API Link : public Component
{
public: // functions
  /// Contructor
  /// @param name of the component
  Link ( const std::string& name );

  /// Virtual destructor
  virtual ~Link();

  /// Get the class name
  static std::string type_name () { return "Link"; }

  /// get the component through the links to the actual components
  Handle<Component> follow();
  Handle<Component const>  follow() const;

  // functions specific to the Link component

  /// link to component
  bool is_linked () const;

  Link& link_to ( Component& lnkto );

  void change_link( SignalArgs & args );

private: // data

  /// this is a link to the component
  Handle<Component> m_link_component;

}; // Link

/// Follow links or return the component itself if it's not a link
Handle<Component> follow_link(const Handle<Component>& link_or_comp);
Handle<Component const> follow_link(const Handle<Component const>& link_or_comp);
Handle<Component> follow_link(Component& link_or_comp);
Handle<Component const> follow_link(const Component& link_or_comp);

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Link_hpp
