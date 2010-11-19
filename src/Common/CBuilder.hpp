// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_CBuilder_hpp
#define CF_Common_CBuilder_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "Common/CFactories.hpp"
#include "Common/CRoot.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component that builds other components of a given abstract type
/// @author Tiago Quintino
template < typename BASE >
class CBuilder : public Component
{
public:

  typedef boost::shared_ptr< CBuilder<BASE> > Ptr;
  typedef boost::shared_ptr< CBuilder<BASE> const> ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CBuilder(const std::string& name) : Component(name)
  {
    BUILD_COMPONENT;
  }

  /// @brief Virtual destructor.
  virtual ~CBuilder() {}

  /// @returns the class name
  static std::string type_name() { return  "CBuilder<" + BASE::type_name() + ">"; }

  /// Configuration properties
  static void define_config_properties ( CF::Common::PropertyList& props ) {}

  /// Returns the name of the type of what abstract type it builds.
  /// Should match the name of the CFactory holding the builder.
  /// @return name of type
  virtual std::string builder_abstract_type_name() const { return  BASE::type_name(); }

  /// @return the name of the type of what concrete type it builds
  virtual std::string builder_concrete_type_name() const = 0;

  /// @return the name of the type of what concrete type it builds
  virtual typename BASE::Ptr build ( const std::string& name ) const = 0;

private: // methods

  /// regists all the signals declared in this class
  static void regist_signals ( CBuilder<BASE>* self ) { }

}; // CBuilder

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component that builds other components of a given abstract type
/// This is the actual builder for one concrete type.
/// @author Tiago Quintino
template < typename BASE, typename CONCRETE >
class CBuilderT : public CBuilder<BASE>
{
public:

  typedef boost::shared_ptr< CBuilderT<BASE, CONCRETE> > Ptr;
  typedef boost::shared_ptr< CBuilderT<BASE, CONCRETE> const> ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CBuilderT(const std::string& name) : CBuilder<BASE>(name)
  {
    BUILD_COMPONENT;
  }

  /// @brief Virtual destructor.
  virtual ~CBuilderT() {};

  /// @returns the class name
  static std::string type_name() { return "CBuilderT<" + CONCRETE::type_name() + ">"; }

  /// Configuration properties
  static void define_config_properties ( CF::Common::PropertyList& props ) {}

  /// @return the name of the type of this factory
  virtual std::string builder_concrete_type_name() const { return CONCRETE::type_name(); }

  virtual typename BASE::Ptr build ( const std::string& name ) const
  {
    return typename BASE::Ptr ( new CONCRETE(name), Deleter<BASE>() );
  }

  /// @name SIGNALS
  //@{

  /// creates a component from this component
  void build_component ( XmlNode& xml )
  {
    XmlParams params (xml);
    typename BASE::Ptr comp = this->build ( params.get_option<std::string>("Component name") );
    CPath parent_path ( params.get_option<URI>("Parent component") );
    Component::Ptr parent = this->look_component( parent_path );
    parent->add_component( comp );
  }

  //@} END SIGNALS

private: // methods

  /// regists all the signals declared in this class
  static void regist_signals ( CBuilderT<BASE,CONCRETE>* self )
  {
      self->regist_signal ( "build_component" , "builds a component", "Build component" )->connect ( boost::bind ( &CBuilderT<BASE,CONCRETE>::build_component, self, _1 ) );
//      self->signal("build_component").m_signature.insert<std::string>("Component name", "Name for created component" )
//                                                 .insert<URI>("Parent component", "Path to component where place the newly built component");
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
    // regist the concrete type in TypeInfo
    CF::TypeInfo::instance().regist<CONCRETE>( CONCRETE::type_name() );
    CF::TypeInfo::instance().regist< CBuilderT<BASE,CONCRETE> >(  CBuilderT<BASE,CONCRETE>::type_name() );

    // put builder in correct factory
    CFactories::Ptr factories = Core::instance().root()->get_child_type< CFactories >("Factories");
    CFactory::Ptr the_factory = factories->get_factory< BASE >();
    the_factory->create_component_type< CBuilderT<BASE,CONCRETE> >( name );

    // give some info
    CFinfo << "factory of \'" << BASE::type_name() << "\' registering builder of \'" << CONCRETE::type_name() << "\' with name \'" << name << "\'" << CFendl;
  }

}; // ComponentBuilder

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CBuilder_hpp

