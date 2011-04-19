// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CSysSUPG_hpp
#define CF_RDM_CSysSUPG_hpp

#include "RDM/Core/DomainTerm.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API CSysSUPG : public RDM::DomainTerm {

public: // typedefs

  /// the actual scheme implementation is a nested class
  /// varyng with shape function (SF), quadrature rule (QD) and Physics (PHYS)
  template < typename SF, typename QD, typename PHYS > class Scheme;

  typedef boost::shared_ptr< CSysSUPG > Ptr;
  typedef boost::shared_ptr< CSysSUPG const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSysSUPG ( const std::string& name );

  /// Virtual destructor
  virtual ~CSysSUPG();

  /// Get the class name
  static std::string type_name () { return "CSysSUPG"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSysSUPG_hpp
