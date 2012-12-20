// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_common_Builder_hpp
#define cf3_common_Builder_hpp

/////////////////////////////////////////////////////////////////////////////////

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include "common/Core.hpp"
#include "common/Factories.hpp"
#include "common/Link.hpp"
#include "common/Library.hpp"
#include "common/Libraries.hpp"
#include "common/Log.hpp"
#include "common/Option.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionFactory.hpp"
#include "common/XML/Protocol.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component that builds other components
/// @author Tiago Quintino
class Common_API Builder : public Component
{
public:
  /// @brief Contructor
  /// @param name of component
  Builder(const std::string& name);

  /// @brief Virtual destructor.
  virtual ~Builder();

  /// @returns the class name
  static std::string type_name() { return  "Builder"; }

  /// Returns the name of the type of what abstract type it builds.
  /// Should match the name of the Factory holding the builder.
  /// @return name of type
  virtual std::string builder_abstract_type_name() const = 0;

  /// @return the name of the type of what concrete type it builds
  virtual std::string builder_concrete_type_name() const = 0;

  /// @return the created component, which is of the passed type
  virtual boost::shared_ptr<Component> build ( const std::string& name ) const = 0;

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

}; // Builder

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component that builds other components of a given abstract type
/// This is the actual builder for one concrete type.
/// @author Tiago Quintino
template < typename BASE, typename CONCRETE >
class BuilderT : public Builder {

public:

  /// @brief Contructor
  /// @param name of component
  BuilderT(const std::string& name) : Builder(name)
  {
    // verify that BASE derives or is same type of Component
    BOOST_STATIC_ASSERT( (boost::is_base_of<common::Component,BASE>::value) );
    // verify that CONCRETE derives from BASE
    BOOST_STATIC_ASSERT( (boost::is_base_of<BASE,CONCRETE>::value) );

    // verify inheritance
    BOOST_STATIC_ASSERT( (boost::is_base_of<Builder,BuilderT<BASE,CONCRETE> >::value) );

    // Register option builders
    RegisterOptionBuilder opt_builder(common::class_name< Handle<CONCRETE> >(), new OptionComponentBuilder(*this));
    RegisterOptionBuilder arr_builder(common::class_name< std::vector< Handle<CONCRETE> > >(), new OptionArrayBuilder(*this));
  }

  /// @brief Virtual destructor.
  virtual ~BuilderT() {}

  /// @returns the class name
  static std::string type_name() { return "BuilderT<" + BASE::type_name() + "," + CONCRETE::type_name() + ">"; }

  /// builds the component cast to the correct base
  boost::shared_ptr<BASE> create_component_typed ( const std::string& name ) const
  {
    return typename boost::shared_ptr<BASE> ( allocate_component<CONCRETE>(name) );
  }

  /// Returns the name of the type of what abstract type it builds.
  /// Should match the name of the Factory holding the builder.
  /// @return name of type
  virtual std::string builder_abstract_type_name() const { return  BASE::type_name(); }

  /// @return the name of the type of this factory
  virtual std::string builder_concrete_type_name() const { return CONCRETE::type_name(); }

  virtual boost::shared_ptr<Component> build ( const std::string& name ) const
  {
    return this->create_component_typed(name);
  }

private:
  /// Builder for OptionComponents
  class OptionComponentBuilder : public OptionBuilder
  {
  public:
    OptionComponentBuilder(const Builder& builder) : m_builder(builder)
    {
    }

    virtual boost::shared_ptr< Option > create_option(const std::string& name, const boost::any& default_value)
    {
      return boost::shared_ptr<Option>(new OptionComponent<CONCRETE>(name, Handle<CONCRETE>(m_builder.access_component(URI(boost::any_cast<std::string>(default_value))))));
    }

  private:
    const Builder& m_builder;
  };

  /// Builder for OptionArrays of this component
  class OptionArrayBuilder : public OptionBuilder
  {
  public:
    OptionArrayBuilder(const Builder& builder) : m_builder(builder)
    {
    }
    virtual boost::shared_ptr< Option > create_option(const std::string& name, const boost::any& default_value)
    {
      const std::vector<std::string> uri_strings = boost::any_cast< std::vector<std::string> >(default_value);
      typename OptionArray< Handle<CONCRETE> >::value_type def_val; def_val.reserve(uri_strings.size());
      BOOST_FOREACH(const std::string& uri_str, uri_strings)
      {
        def_val.push_back(Handle<CONCRETE>(m_builder.access_component(URI(uri_str))));
      }
      return boost::shared_ptr<Option>(new OptionArray< Handle<CONCRETE> >(name, def_val));
    }
  private:
    const Builder& m_builder;
  };
}; // BuilderT

/////////////////////////////////////////////////////////////////////////////////

/// @brief Helper class to create the Builder and place it in the factory
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class LIB >
struct ComponentBuilder
{
  /// @brief creates the Builder and places it into the correct factory
  ComponentBuilder(const std::string& name =
                   std::string( LIB::library_namespace() + "." + CONCRETE::type_name()) )
  {
    // verify that LIB derives from Library
    BOOST_STATIC_ASSERT( (boost::is_base_of<common::Library,LIB>::value) );
    // verify that BASE derives or is same type of Component
    BOOST_STATIC_ASSERT( (boost::is_base_of<common::Component,BASE>::value) );
    // verify that CONCRETE derives from BASE
    BOOST_STATIC_ASSERT( (boost::is_base_of<BASE,CONCRETE>::value) );
    // verify that BuilderT derives from Builder
    BOOST_STATIC_ASSERT( (boost::is_base_of<Builder,BuilderT<BASE,CONCRETE> >::value) );

    // give some info
    //CFdebug << "lib [" << LIB::library_namespace() << "] : factory of \'" << BASE::type_name() << "\' registering builder of \'" << CONCRETE::type_name() << "\' with name \'" << name << "\'" << CFendl;

    // regist the concrete type in TypeInfo
    RegistTypeInfo<CONCRETE,LIB> regist(name);
    cf3::common::TypeInfo::instance().
        regist< BuilderT<BASE,CONCRETE> >(  BuilderT<BASE,CONCRETE>::type_name() );

    // put builder in correct factory
    Handle<Factory> factory(common::Core::instance().factories().get_factory< BASE >());

    cf3_assert ( is_not_null(factory) );

    Handle< common::BuilderT<BASE,CONCRETE> > builder =
        factory->create_component< BuilderT<BASE,CONCRETE> >( name );

    // check that BuilderT can cast to Builder
    /// @note sanity check after weird bug on MacOSX
    ///       when including Builder.cpp instead of Builder.hpp
    cf3_assert ( is_not_null(Handle<Builder>(builder)) );

    // put a Link to the builder in the respective Library
    Handle<Library> lib(Core::instance().libraries().library<LIB>());
    cf3_assert ( is_not_null(lib) );

    Handle<Link> builder_link = lib->create_component<Link>( name );
    cf3_assert ( is_not_null(builder_link) );

    builder_link->link_to(*builder);

    cf3_assert ( is_not_null(builder_link->follow()) );
    cf3_assert ( is_not_null(Handle< BuilderT<BASE,CONCRETE> >(builder_link->follow())) );
    cf3_assert ( is_not_null(Handle<Builder>(builder_link->follow())) );

  }

}; // ComponentBuilder

/////////////////////////////////////////////////////////////////////////////////

/// Find the builder with the given fully-qualified name. Throws if unsuccessful
Builder& find_builder(const std::string& builder_name);

/////////////////////////////////////////////////////////////////////////////////

/// Find the builder with the given fully-qualified name. Returns a null handle if unsuccessful
Handle<Builder> find_builder_ptr(const std::string& builder_name);

/////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Builder_hpp

