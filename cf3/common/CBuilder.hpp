// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_common_CBuilder_hpp
#define cf3_common_CBuilder_hpp

/////////////////////////////////////////////////////////////////////////////////

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include "common/CFactories.hpp"
#include "common/CRoot.hpp"
#include "common/CLink.hpp"
#include "common/Core.hpp"
#include "common/CLibrary.hpp"
#include "common/CLibraries.hpp"
#include "common/XML/Protocol.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component that builds other components
/// @author Tiago Quintino
class Common_API CBuilder : public Component {

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
  void signal_create_component ( SignalArgs& args );

  void signature_signal_create_component ( SignalArgs& args );

  //@} END SIGNALS

  /// Extract the builder's reduced name from the given builder name
  /// The reduced name is the name with the namespace removed, in other words
  /// all characters after the last dot
  static std::string extract_reduced_name (const std::string& builder_name);
  /// Extract the builder's namespace from the given builder name
  /// The namespace is the name up until the last dot.
  static std::string extract_namespace (const std::string& builder_name);

  /// Extract the library name from the given builder name
  static std::string extract_library_name (const std::string& builder_name);

}; // CBuilder

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component that builds other components of a given abstract type
/// This is the actual builder for one concrete type.
/// @author Tiago Quintino
template < typename BASE, typename CONCRETE >
class CBuilderT : public CBuilder {

public:

  typedef boost::shared_ptr< CBuilderT<BASE,CONCRETE> > Ptr;
  typedef boost::shared_ptr< CBuilderT<BASE,CONCRETE> const > ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CBuilderT(const std::string& name) : CBuilder(name)
  {
    // verify that BASE derives or is same type of Component
    BOOST_STATIC_ASSERT( (boost::is_base_of<common::Component,BASE>::value) );
    // verify that CONCRETE derives from BASE
    BOOST_STATIC_ASSERT( (boost::is_base_of<BASE,CONCRETE>::value) );

    // verify inheritance
    BOOST_STATIC_ASSERT( (boost::is_base_of<CBuilder,CBuilderT<BASE,CONCRETE> >::value) );

  }

  /// @brief Virtual destructor.
  virtual ~CBuilderT() {}

  /// @returns the class name
  static std::string type_name() { return "CBuilderT<" + BASE::type_name() + "," + CONCRETE::type_name() + ">"; }

  /// builds the component cast to the correct base
  typename BASE::Ptr create_component_typed ( const std::string& name ) const
  {
    return typename BASE::Ptr ( allocate_component<CONCRETE>(name) );
  }

  /// Returns the name of the type of what abstract type it builds.
  /// Should match the name of the CFactory holding the builder.
  /// @return name of type
  virtual std::string builder_abstract_type_name() const { return  BASE::type_name(); }

  /// @return the name of the type of this factory
  virtual std::string builder_concrete_type_name() const { return CONCRETE::type_name(); }

  virtual Component::Ptr build ( const std::string& name ) const
  {
    return this->create_component_typed(name);
  }

}; // CBuilderT

/////////////////////////////////////////////////////////////////////////////////

/// @brief Helper class to create the CBuilder and place it in the factory
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class LIB >
struct ComponentBuilder
{
  /// @brief creates the CBuilder and places it into the correct factory
  ComponentBuilder(const std::string& name =
                   std::string( LIB::library_namespace() + "." + CONCRETE::type_name()) )
  {
    // verify that LIB derives from CLibrary
    BOOST_STATIC_ASSERT( (boost::is_base_of<common::CLibrary,LIB>::value) );
    // verify that BASE derives or is same type of Component
    BOOST_STATIC_ASSERT( (boost::is_base_of<common::Component,BASE>::value) );
    // verify that CONCRETE derives from BASE
    BOOST_STATIC_ASSERT( (boost::is_base_of<BASE,CONCRETE>::value) );
    // verify that CBuilderT derives from CBuilder
    BOOST_STATIC_ASSERT( (boost::is_base_of<CBuilder,CBuilderT<BASE,CONCRETE> >::value) );

    // give some info
    //CFinfo << "lib [" << LIB::library_namespace() << "] : factory of \'" << BASE::type_name() << "\' registering builder of \'" << CONCRETE::type_name() << "\' with name \'" << name << "\'" << CFendl;

    // regist the concrete type in TypeInfo
    RegistTypeInfo<CONCRETE,LIB> regist(name);
    cf3::common::TypeInfo::instance().
        regist< CBuilderT<BASE,CONCRETE> >(  CBuilderT<BASE,CONCRETE>::type_name() );

    // put builder in correct factory
    common::CFactory::Ptr factory =
        common::Core::instance().factories().get_factory< BASE >();

    cf3_assert ( is_not_null(factory) );

    boost::shared_ptr< common::CBuilderT<BASE,CONCRETE> > builder =
        factory->create_component_ptr< CBuilderT<BASE,CONCRETE> >( name );

    // check that CBuilderT can cast to CBuilder
    /// @note sanity check after weird bug on MacOSX
    ///       when including CBuilder.cpp instead of CBuilder.hpp
    cf3_assert ( builder->template as_ptr<CBuilder>() );

    // put a CLink to the builder in the respective CLibrary
    CLibrary::Ptr lib = Core::instance().libraries().library<LIB>();
    cf3_assert ( is_not_null(lib) );

    CLink::Ptr builder_link = lib->create_component_ptr<CLink>( name );
    cf3_assert ( is_not_null(builder_link) );

    builder_link->link_to(builder);

    cf3_assert ( is_not_null(builder_link->follow()) );
    cf3_assert ( is_not_null(builder_link->follow()->as_ptr<CBuilderT<BASE,CONCRETE> >()) );
    cf3_assert ( is_not_null(builder_link->follow()->as_ptr<CBuilder>()) );

  }

}; // ComponentBuilder

/////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CBuilder_hpp

