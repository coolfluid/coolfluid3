// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CFactories_hpp
#define CF_Common_CFactories_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CFactory.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component that defines global environment
  /// @author Quentin Gasper
  class Common_API CFactories : public Component {

  public: //typedefs

    typedef boost::shared_ptr<CFactories> Ptr;
    typedef boost::shared_ptr<CFactories const> ConstPtr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CFactories ( const CName& name );

    /// Virtual destructor
    virtual ~CFactories();

    /// Get the class name
    static std::string type_name () { return "CFactories"; }

    /// Configuration Options
    static void define_config_properties ( Common::PropertyList& options );

    /// gives access to the factory of supplied type,
    /// insuring that in case it does not exist it gets built.
    template < typename CBase >
    typename CFactoryT<CBase>::Ptr get_factory ()
    {
      const std::string tname = CBase::type_name();
      Component::Ptr factory = get_child(tname);
      if ( factory != nullptr )
        return boost::dynamic_pointer_cast< CFactoryT<CBase> >(factory);
      else
      {
        CF::TypeInfo::instance().regist< CFactoryT<CBase> >( CFactoryT<CBase>::type_name() );
        return create_component_type< CFactoryT<CBase> >(tname);
      }
    }

  private: // helper functions

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  private: // data

}; // CFactories

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CFactories_hpp
