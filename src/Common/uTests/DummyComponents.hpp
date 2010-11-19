// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/LibCommon.hpp"
#include "Common/Component.hpp"
#include "Common/OptionT.hpp"
#include "Common/ObjectProvider.hpp"

#ifndef DUMMYCOMPONENTS_HPP
#define DUMMYCOMPONENTS_HPP

namespace CF {
namespace Common {

class CAbstract : public Component
{

public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CAbstract,1 > PROVIDER;
  /// pointer to this type
  typedef boost::shared_ptr<CAbstract> Ptr;
  typedef boost::shared_ptr<CAbstract const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CAbstract ( const CName& name ) : Component(name)
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
  CConcrete1 ( const CName& name ) : CAbstract(name)
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
  CConcrete2 ( const CName& name ) : CAbstract(name)
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


CF::Common::ObjectProvider < CConcrete1, CAbstract, LibCommon, NB_ARGS_1 >
aConcrete1ComponentProvider ( "Concrete1" );

CF::Common::ObjectProvider < CConcrete2, CAbstract, LibCommon, NB_ARGS_1 >
aConcrete2ComponentProvider ( "Concrete2" );

} // Common
} // CF

#endif // DUMMYCOMPONENTS_HPP
