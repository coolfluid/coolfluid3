// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_FaceTerm_hpp
#define CF_RDM_FaceTerm_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {
namespace RDM {

  class ElementLoop;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API FaceTerm : public CF::Solver::Action {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< FaceTerm > Ptr;
  typedef boost::shared_ptr< FaceTerm const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  FaceTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~FaceTerm();

  /// Get the class name
  static std::string type_name () { return "FaceTerm"; }

  ElementLoop& access_element_loop( const std::string& type_name );

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_FaceTerm_hpp
