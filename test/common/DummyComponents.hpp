// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/LibCommon.hpp"
#include "common/Component.hpp"
#include "common/OptionList.hpp"
#include "common/OptionURI.hpp"
#include "common/Builder.hpp"

#ifndef DUMMYCOMPONENTS_HPP
#define DUMMYCOMPONENTS_HPP

namespace cf3 {
namespace common {

class CAbstract : public Component
{

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CAbstract> Ptr;
  typedef boost::shared_ptr<CAbstract const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CAbstract ( const std::string& name ) : Component(name)
  {

  }

  /// Virtual destructor
  virtual ~CAbstract() {}

  /// Get the class name
  static std::string type_name () { return "CAbstract"; }

  virtual std::string type() { return type_name(); }

};

class CConcrete1 : public CAbstract
{

public: // functions

  /// Contructor
  /// @param name of the component
  CConcrete1 ( const std::string& name ) : CAbstract(name)
  {
    // options
    URI def_path("cpath:/");
    options().add( "MyRelativeFriend", def_path  )
        .description("a path to another component");
    options().add( "MyAbsoluteFriend", def_path  )
        .description("a path to another component");
  }

  /// Virtual destructor
  virtual ~CConcrete1() {}

  /// Get the class name
  static std::string type_name () { return "CConcrete1"; }

  virtual std::string type() { return type_name(); }

};

class CConcrete2 : public CAbstract
{

public: // functions

  /// Contructor
  /// @param name of the component
  CConcrete2 ( const std::string& name ) : CAbstract(name)
  {

  }

  /// Virtual destructor
  virtual ~CConcrete2() {}

  /// Get the class name
  static std::string type_name () { return "CConcrete2"; }

  virtual std::string type() { return type_name(); }

};


cf3::common::ComponentBuilder < CConcrete1, CAbstract, LibCommon > aConcrete1Component_Builder;

cf3::common::ComponentBuilder < CConcrete2, CAbstract, LibCommon > aConcrete2Component_Builder;

} // common
} // cf3

#endif // DUMMYCOMPONENTS_HPP
