// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Factories_hpp
#define cf3_common_Factories_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Factory.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

  /// Component that defines global environment
  /// @author Quentin Gasper
  class Common_API Factories : public Component {

  public: //typedefs

    typedef boost::shared_ptr<Factories> Ptr;
    typedef boost::shared_ptr<Factories const> ConstPtr;

  public: // functions

    /// Contructor
    /// @param name of the component
    Factories ( const std::string& name );

    /// Virtual destructor
    virtual ~Factories();

    /// Get the class name
    static std::string type_name () { return "Factories"; }

    /// gives access to the factory of supplied type,
    /// insuring that in case it does not exist it gets built.
    template < typename CBase >
    typename FactoryT<CBase>::Ptr get_factory ()
    {
      const std::string tname = CBase::type_name();
      Component::Ptr factory = get_child_ptr(tname);
      if ( is_not_null(factory) )
        return boost::dynamic_pointer_cast< FactoryT<CBase> >(factory);
      else
      {
        cf3::common::TypeInfo::instance().regist< FactoryT<CBase> >( FactoryT<CBase>::type_name() );
        return create_component_ptr< FactoryT<CBase> >(tname);
      }
    }

}; // Factories

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Factories_hpp
