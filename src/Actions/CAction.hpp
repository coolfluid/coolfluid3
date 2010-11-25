// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CAction_hpp
#define CF_Mesh_CAction_hpp

#include "Common/Component.hpp"
#include "Common/ConcreteProvider.hpp"
#include "Actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CAction : public Common::Component
{
public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CAction , Common::NB_ARGS_1 > PROVIDER;

  /// pointers
  typedef boost::shared_ptr<CAction> Ptr;
  typedef boost::shared_ptr<CAction const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CAction ( const std::string& name );

  /// Virtual destructor
  virtual ~CAction() {};

  /// Get the class name
  static std::string type_name () { return "CAction"; }

  /// Configuration Options
  static void define_config_properties ( Common::PropertyList& options ) {}

  /// execute the action
  virtual void execute () = 0;

  /// Templated version for high efficiency
  template < typename T >
  void executeT ()
  {
    execute();
  }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CAction_hpp
