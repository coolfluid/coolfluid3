// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/LibCommon.hpp"
#include "Common/Component.hpp"
#include "Common/OptionT.hpp"
#include "Common/CBuilder.hpp"

#ifndef DUMMYCOMPONENTS_HPP
#define DUMMYCOMPONENTS_HPP

namespace CF {
namespace Common {

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
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CAbstract() {}

  /// Get the class name
  static std::string type_name () { return "CAbstract"; }

  // --------- Configuration ---------

  static void define_config_properties ( Common::PropertyList& options ) {}

  // --------- Specific functions to this component ---------

  virtual std::string type() { return type_name(); }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( CAbstract* self ) {}

};

class CConcrete1 : public CAbstract
{

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CConcrete1> Ptr;
  typedef boost::shared_ptr<CConcrete1 const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CConcrete1 ( const std::string& name ) : CAbstract(name)
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CConcrete1() {}

  /// Get the class name
  static std::string type_name () { return "CConcrete1"; }

  // --------- Configuration ---------

  static void define_config_properties ( Common::PropertyList& options )
  {
    URI def_path("cpath://");
    options.add_option< OptionT<URI> > ( "MyRelativeFriend", "a path to another component"   , def_path  );
    options.add_option< OptionT<URI> > ( "MyAbsoluteFriend", "a path to another component"   , def_path  );
  }

  // --------- Specific functions to this component ---------

  virtual std::string type() { return type_name(); }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( CConcrete1* self ) {}

};

class CConcrete2 : public CAbstract
{

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CConcrete2> Ptr;
  typedef boost::shared_ptr<CConcrete2 const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CConcrete2 ( const std::string& name ) : CAbstract(name)
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~CConcrete2() {}

  /// Get the class name
  static std::string type_name () { return "CConcrete2"; }

  // --------- Configuration ---------

  static void define_config_properties ( Common::PropertyList& options ) {}

  // --------- Specific functions to this component ---------

  virtual std::string type() { return type_name(); }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( CConcrete2* self ) {}

};


CF::Common::ComponentBuilder < CConcrete1, CAbstract, LibCommon >
aConcrete1Component_Builder ( "Concrete1" );

CF::Common::ComponentBuilder < CConcrete2, CAbstract, LibCommon >
aConcrete2Component_Builder ( "Concrete2" );

} // Common
} // CF

#endif // DUMMYCOMPONENTS_HPP
