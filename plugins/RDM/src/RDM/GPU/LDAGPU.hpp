// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_LDAGPU_hpp
#define CF_RDM_LDAGPU_hpp

#include "RDM/CellTerm.hpp"
#include "RDM/GPU/LibGPU.hpp"


namespace cf3 {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_GPU_API LDAGPU : public RDM::CellTerm {

  template < typename PHYS > struct ElementLoop;

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  LDAGPU ( const std::string& name );

  /// Virtual destructor
  virtual ~LDAGPU();

  /// Get the class name
  static std::string type_name () { return "LDAGPU"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_Mesh_LDAGPU_hpp
