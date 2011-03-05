// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_DomainTerm_hpp
#define CF_RDM_DomainTerm_hpp

#include "RDM/Action.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API DomainTerm : public RDM::Action {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< DomainTerm > Ptr;
  typedef boost::shared_ptr< DomainTerm const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  DomainTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~DomainTerm();

  /// Get the class name
  static std::string type_name () { return "DomainTerm"; }

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_DomainTerm_hpp
