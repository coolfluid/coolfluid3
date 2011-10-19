// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_CFactories_hpp
#define cf3_common_CFactories_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CFactory.hpp"

namespace cf3 {
namespace common {

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
    CFactories ( const std::string& name );

    /// Virtual destructor
    virtual ~CFactories();

    /// Get the class name
    static std::string type_name () { return "CFactories"; }

    /// gives access to the factory of supplied type,
    /// insuring that in case it does not exist it gets built.
    template < typename CBase >
    typename CFactoryT<CBase>::Ptr get_factory ()
    {
      const std::string tname = CBase::type_name();
      Component::Ptr factory = get_child_ptr(tname);
      if ( is_not_null(factory) )
        return boost::dynamic_pointer_cast< CFactoryT<CBase> >(factory);
      else
      {
        cf3::common::TypeInfo::instance().regist< CFactoryT<CBase> >( CFactoryT<CBase>::type_name() );
        return create_component_ptr< CFactoryT<CBase> >(tname);
      }
    }

}; // CFactories

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CFactories_hpp
