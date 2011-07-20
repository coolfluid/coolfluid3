// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_BoundaryTerm_hpp
#define CF_RDM_BoundaryTerm_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

  class ElementLoop;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API BoundaryTerm : public CF::Solver::Action {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< BoundaryTerm > Ptr;
  typedef boost::shared_ptr< BoundaryTerm const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  BoundaryTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~BoundaryTerm();

  /// Get the class name
  static std::string type_name () { return "BoundaryTerm"; }

  /// Identifies if this boundary condition is wek or strong
  /// A strong BC forces directly the solution values, whereas a weak is applied
  /// by changing the contribution of the equations, therefore indirectly.
  /// @returns if this term is a weak term
  virtual bool is_weak() const = 0;

  /// @return the element loop for the element type identified by the string
  /// @post will create the element loop if does not exist
  ElementLoop& access_element_loop( const std::string& type_name );

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_BoundaryTerm_hpp
