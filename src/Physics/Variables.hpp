// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_Variables_hpp
#define CF_Physics_Variables_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Physics/PhysModel.hpp"

namespace CF {

namespace Physics {

  class Variables;

////////////////////////////////////////////////////////////////////////////////

/// Interface to a set of variables
/// @author Tiago Quintino
class Physics_API Variables : public Common::Component {

public: //typedefs

  typedef boost::shared_ptr<Variables> Ptr;
  typedef boost::shared_ptr<Variables const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  Variables ( const std::string& name );

  /// virtual destructor
  virtual ~Variables();

  /// Get the class name
  static std::string type_name () { return "Variables"; }

  /// @name INTERFACE
  //@{

  virtual void compute_properties( PhysModel::Properties& physp ) = 0;
  virtual void flux_jacobian( PhysModel::Properties& physp ) = 0;

  //@} END INTERFACE

}; // Variables

////////////////////////////////////////////////////////////////////////////////

/// Template class that provides a dyanmic wrapper around a static implemented
/// variables class
/// @author Tiago Quintino
template < typename PHYS >
class VariablesT : public Variables {
public:

  /// constructor
  VariablesT ( const std::string& name ) : Variables( name ) {}

  /// virtual destructor
  virtual ~VariablesT() {}

  virtual void compute_properties( PhysModel::Properties& physp )
  {
    typename PHYS::PROPS& cphysp = static_cast<typename PHYS::PROPS&>( physp );
    PHYS::compute_properties( cphysp );
  }

  virtual void flux_jacobian( PhysModel::Properties& physp )
  {
    typename PHYS::PROPS& cphysp = static_cast<typename PHYS::PROPS&>( physp );
    PHYS::flux_jacobian( cphysp );
  }

}; // VariablesT

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Physics_Variables_hpp
