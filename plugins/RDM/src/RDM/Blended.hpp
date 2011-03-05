// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Blended_hpp
#define CF_RDM_Blended_hpp

#include "RDM/LibRDM.hpp"
#include "RDM/Action.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API Blended : public RDM::Action
{

public: // typedefs

  typedef boost::shared_ptr< Blended > Ptr;
  typedef boost::shared_ptr< Blended const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Blended ( const std::string& name );

  /// Virtual destructor
  virtual ~Blended();

  /// Get the class name
  static std::string type_name () { return "Blended"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Blended_hpp
