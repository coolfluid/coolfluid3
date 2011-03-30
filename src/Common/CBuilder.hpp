// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_CBuilder_hpp
#define CF_Common_CBuilder_hpp

/////////////////////////////////////////////////////////////////////////////////

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include "Common/CFactories.hpp"
#include "Common/CRoot.hpp"
#include "Common/CLink.hpp"
#include "Common/Core.hpp"
#include "Common/CLibrary.hpp"
#include "Common/CLibraries.hpp"
#include "Common/XML/Protocol.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component that builds other components
/// @author Tiago Quintino
class Common_API CBuilder : public Component
{
public:

  typedef boost::shared_ptr< CBuilder > Ptr;
  typedef boost::shared_ptr< CBuilder const> ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CBuilder(const std::string& name);

  /// @brief Virtual destructor.
  virtual ~CBuilder();

  /// @returns the class name
  static std::string type_name() { return  "CBuilder"; }

  /// Returns the name of the type of what abstract type it builds.
  /// Should match the name of the CFactory holding the builder.
  /// @return name of type
  virtual std::string builder_abstract_type_name() const = 0;

  /// @return the name of the type of what concrete type it builds
  virtual std::string builder_concrete_type_name() const = 0;

  /// @return the name of the type of what concrete type it builds
  virtual Component::Ptr build ( const std::string& name ) const = 0;

  /// @name SIGNALS
  //@{

  /// creates a component from this component
  void signal_build_component ( SignalArgs& args );

  void signature_signal_build_component ( SignalArgs& args );

  //@} END SIGNALS

}; // CBuilder

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component that builds other components of a given abstract type
/// This is the actual builder for one concrete type.
/// @author Tiago Quintino
template < typename BASE, typename CONCRETE >
class CBuilderT : public CBuilder
{
public:

  typedef boost::shared_ptr< CBuilderT<BASE, CONCRETE> > Ptr;
  typedef boost::shared_ptr< CBuilderT<BASE, CONCRETE> const> ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CBuilderT(const std::string& name) : CBuilder(name)
  {
    // verify that BASE derives or is same type of Component
    BOOST_STATIC_ASSERT( (boost::is_base_of<Common::Component,BASE>::value) );
    // verify that CONCRETE derives from BASE
    BOOST_STATIC_ASSERT( (boost::is_base_of<BASE,CONCRETE>::value) );
  }

  /// @brief Virtual destructor.
  virtual ~CBuilderT() {}

  /// @returns the class name
  static std::string type_name() { return "CBuilderT<" + CONCRETE::type_name() + ">"; }

  /// builds the component cast to the correct base
  typename BASE::Ptr build_component_typed ( const std::string& name ) const
  {
    return typename BASE::Ptr ( new CONCRETE(name), Deleter<BASE>() );
  }

  /// Returns the name of the type of what abstract type it builds.
  /// Should match the name of the CFactory holding the builder.
  /// @return name of type
  virtual std::string builder_abstract_type_name() const { return  BASE::type_name(); }

  /// @return the name of the type of this factory
  virtual std::string builder_concrete_type_name() const { return CONCRETE::type_name(); }

  virtual Component::Ptr build ( const std::string& name ) const
  {
    return this->build_component_typed(name);
  }

}; // CBuilderT

/////////////////////////////////////////////////////////////////////////////////

/// @brief Helper class to create the CBuilder and place it in the factory
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class LIB >
struct ComponentBuilder
{
  /// @brief creates the CBuilder and places it into the correct factory
  ComponentBuilder( const std::string& name = std::string( LIB::library_namespace() + "." + CONCRETE::type_name()) )
  {
    // verify that LIB derives from CLibrary
    BOOST_STATIC_ASSERT( (boost::is_base_of<Common::CLibrary,LIB>::value) );
    // verify that BASE derives or is same type of Component
    BOOST_STATIC_ASSERT( (boost::is_base_of<Common::Component,BASE>::value) );
    // verify that CONCRETE derives from BASE
    BOOST_STATIC_ASSERT( (boost::is_base_of<BASE,CONCRETE>::value) );

    // give some info
    //CFinfo << "lib [" << LIB::type_name() << "] : factory of \'" << BASE::type_name() << "\' registering builder of \'" << CONCRETE::type_name() << "\' with name \'" << name << "\'" << CFendl;

    // regist the concrete type in TypeInfo
    CF::Common::TypeInfo::instance().regist<CONCRETE>( CONCRETE::type_name() );
    CF::Common::TypeInfo::instance().regist< CBuilderT<BASE,CONCRETE> >(  CBuilderT<BASE,CONCRETE>::type_name() );

    // get the factories
    Common::CFactories::Ptr factories = Common::Core::instance().factories();

    // put builder in correct factory
    Common::CFactory::Ptr   factory = factories->get_factory< BASE >();
    cf_assert ( factory != nullptr );

    CBuilder::Ptr builder = factory->create_component< Common::CBuilderT<BASE,CONCRETE> >( name );
    cf_assert ( builder != nullptr );

    // put a CLink to the builder in the respective CLibrary
    CLibrary::Ptr lib = Core::instance().libraries()->get_library<LIB>();
    cf_assert ( lib != nullptr );

    CLink::Ptr liblink = lib->create_component<CLink>( name );
    cf_assert ( liblink != nullptr );

    liblink->link_to( builder );
  }

}; // ComponentBuilder

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CBuilder_hpp

