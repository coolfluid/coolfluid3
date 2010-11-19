// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_ObjectProvider_hpp
#define CF_Common_ObjectProvider_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/Log.hpp"

#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a concrete templated provider to create
/// whatever kind of polymorphic objects. By default it takes two
/// template parameters and it builds a non configurable object
/// whose create() method accepts no arguments.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class MODULE, int NBARGS = 0>
class ObjectProvider : public BASE::PROVIDER {
public:

  /// Constructor
  explicit ObjectProvider(const std::string& name) : BASE::PROVIDER(name)
  {
    CFLogDebugVerbose (  "Creating provider \'" << name << "\' of type \'" << BASE::type_name() << "\'\n" );
    CF::TypeInfo::instance().regist<CONCRETE>( CONCRETE::type_name() );
    Common::LibraryRegister<MODULE>::instance().self_registry().regist(this);
  }

  /// Destructor
  /// @todo unregistration is currently not working
  ~ObjectProvider()
  {
    // Common::LibraryRegister<MODULE>::instance().self_registry().unregist(this);
  }

  /// Polymorphic function to create objects of dynamical type BASE
  /// @return boost::shared_ptr olding the created object
  boost::shared_ptr<BASE> create()
  {
    return boost::shared_ptr<BASE>(new CONCRETE(), Deleter<BASE>() );
  }

}; // ObjectProvider

////////////////////////////////////////////////////////////////////////////////

/// This class represents a concrete templated provider to create
/// whatever kind of polymorphic objects. It takes four
/// template parameters and it builds a non configurable object
/// whose create() method accepts one arguments.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class MODULE>
class ObjectProvider<CONCRETE,BASE,MODULE, 1> : public BASE::PROVIDER {
public:

  /// Constructor
  explicit ObjectProvider(const std::string& name) : BASE::PROVIDER(name)
  {
    CFLogDebugVerbose (  "Creating provider \'" << name << "\' of type \'" << BASE::type_name() << "\'\n" );
    CF::TypeInfo::instance().regist<CONCRETE>( CONCRETE::type_name() );
    MODULE::instance().self_registry().regist(this);
  }

  /// Destructor
  /// @todo unregistration is currently not working
  ~ObjectProvider()
  {
    // Common::LibraryRegister<MODULE>::instance().self_registry().unregist(this);
  }

  /// Polymorphic function to create objects of dynamical type BASE
  /// @return boost::shared_ptr olding the created object
  boost::shared_ptr<BASE> create(typename BASE::ARG1 arg)
  {
    return boost::shared_ptr<BASE>(new CONCRETE(arg), Deleter<BASE>());
  }

}; // ObjectProvider

////////////////////////////////////////////////////////////////////////////////

/// This class represents a concrete templated provider to create
/// whatever kind of polymorphic objects. It takes four
/// template parameters and it builds a non configurable object
/// whose create() method accepts one arguments.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class MODULE>
class ObjectProvider<CONCRETE,BASE,MODULE, 2> : public BASE::PROVIDER {
public:

  /// Constructor
  explicit ObjectProvider(const std::string& name) :
    BASE::PROVIDER(name)
  {
    CFLogDebugVerbose (  "Creating provider \'" << name << "\' of type \'" << BASE::type_name() << "\'\n" );
    CF::TypeInfo::instance().regist<CONCRETE>( CONCRETE::type_name() );
    MODULE::instance().self_registry().regist(this);
  }

  /// Destructor
  /// @todo unregistration is currently not working
  ~ObjectProvider()
  {
    // Common::LibraryRegister<MODULE>::instance().self_registry().unregist(this);
  }

  /// Polymorphic function to create objects of dynamical type BASE
  /// @param arg1 first parameter
  /// @param arg2 first parameter
  /// @return boost::shared_ptr olding the created object
  boost::shared_ptr<BASE> create(typename BASE::ARG1 arg1,
                                 typename BASE::ARG2 arg2)
  {
    return boost::shared_ptr<BASE>(new CONCRETE(arg1, arg2), Deleter<BASE>());
  }

  /// Polymorphic function to create objects of dynamical type BASE
  /// @return boost::shared_ptr olding the created object
  boost::shared_ptr<BASE> create()
  {
  }

}; // ObjectProvider

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ObjectProvider_hpp
