// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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
class Common_API Link : public Component {

public: //typedefs

  typedef boost::shared_ptr<Link> Ptr;
  typedef boost::shared_ptr<Link const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Link ( const std::string& name );

  /// Virtual destructor
  virtual ~Link();

  /// Get the class name
  static std::string type_name () { return "Link"; }

  /// get the component through the links to the actual components
  virtual Component::Ptr follow ();
  virtual Component::ConstPtr  follow() const;

  // functions specific to the Link component

  /// link to component
  bool is_linked () const;

  Link& link_to ( Component::Ptr lnkto );
  Link& link_to ( Component& lnkto );
  Link& link_to ( const Component& lnkto );

  void change_link( SignalArgs & args );

private: // data

  /// this is a link to the component
  /// using weak_ptr means it might become invalid so we should test for expire()
  boost::weak_ptr<Component> m_link_component;

}; // Link

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Link_hpp
