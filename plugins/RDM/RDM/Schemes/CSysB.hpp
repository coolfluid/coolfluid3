// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CSysB_hpp
#define CF_RDM_CSysB_hpp

#include "RDM/Core/DomainTerm.hpp"

#include "RDM/Schemes/LibSchemes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_SCHEMES_API CSysB : public RDM::DomainTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Term;

  typedef boost::shared_ptr< CSysB > Ptr;
  typedef boost::shared_ptr< CSysB const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSysB ( const std::string& name );

  /// Virtual destructor
  virtual ~CSysB();

  /// Get the class name
  static std::string type_name () { return "CSysB"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSysB_hpp
