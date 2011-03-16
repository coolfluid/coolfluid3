// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SUPG_hpp
#define CF_RDM_SUPG_hpp

#include "RDM/DomainTerm.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API SUPG : public RDM::DomainTerm {

  template < typename PHYS> struct ElementLoop;

public: // typedefs

  typedef boost::shared_ptr< SUPG > Ptr;
  typedef boost::shared_ptr< SUPG const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SUPG ( const std::string& name );

  /// Virtual destructor
  virtual ~SUPG();

  /// Get the class name
  static std::string type_name () { return "SUPG"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_SUPG_hpp
