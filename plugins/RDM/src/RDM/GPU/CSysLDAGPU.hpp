// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CSysLDAGPU_hpp
#define CF_RDM_CSysLDAGPU_hpp

#include "RDM/CellTerm.hpp"

#include "RDM/Schemes/LibSchemes.hpp"
#include "RDM/GPU/LibGPU.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API CSysLDAGPU : public RDM::CellTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CSysLDAGPU ( const std::string& name );

  /// Virtual destructor
  virtual ~CSysLDAGPU();

  /// Get the class name
  static std::string type_name () { return "CSysLDAGPU"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSysLDAGPU_hpp
