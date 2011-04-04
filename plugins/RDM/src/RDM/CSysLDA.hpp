// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CSysLDA_hpp
#define CF_RDM_CSysLDA_hpp

#include "RDM/DomainTerm.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API CSysLDA : public RDM::DomainTerm {

  template < typename PHYS > struct ElementLoop;
  template < typename PHYS > struct ElementLoopSys;

public: // typedefs

  typedef boost::shared_ptr< CSysLDA > Ptr;
  typedef boost::shared_ptr< CSysLDA const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSysLDA ( const std::string& name );

  /// Virtual destructor
  virtual ~CSysLDA();

  /// Get the class name
  static std::string type_name () { return "CSysLDA"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSysLDA_hpp
