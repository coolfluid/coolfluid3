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
    BuildComponent<none>().build(this);
  }

  /// Virtual destructor
  virtual ~CAbstract() {}

  /// Get the class name
  static std::string type_name () { return "CAbstract"; }

  virtual std::string type() { return type_name(); }

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
    BuildComponent<nosignals>().build(this);
  }

  /// Virtual destructor
  virtual ~CConcrete1() {}

  /// Get the class name
  static std::string type_name () { return "CConcrete1"; }

  virtual void define_config_properties ()
  {
    URI def_path("cpath://");
    m_properties.add_option< OptionT<URI> > ( "MyRelativeFriend", "a path to another component"   , def_path  );
    m_properties.add_option< OptionT<URI> > ( "MyAbsoluteFriend", "a path to another component"   , def_path  );
  }

  virtual std::string type() { return type_name(); }

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
    BuildComponent<none>().build(this);
  }

  /// Virtual destructor
  virtual ~CConcrete2() {}

  /// Get the class name
  static std::string type_name () { return "CConcrete2"; }

  virtual std::string type() { return type_name(); }

};


CF::Common::ComponentBuilder < CConcrete1, CAbstract, LibCommon > aConcrete1Component_Builder;

CF::Common::ComponentBuilder < CConcrete2, CAbstract, LibCommon > aConcrete2Component_Builder;

} // Common
} // CF

#endif // DUMMYCOMPONENTS_HPP
