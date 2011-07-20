// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CellTerm_hpp
#define CF_RDM_CellTerm_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {
namespace RDM {

  class ElementLoop;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API CellTerm : public CF::Solver::Action {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< CellTerm > Ptr;
  typedef boost::shared_ptr< CellTerm const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CellTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~CellTerm();

  /// Get the class name
  static std::string type_name () { return "CellTerm"; }

  ElementLoop& access_element_loop( const std::string& type_name );

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_CellTerm_hpp
