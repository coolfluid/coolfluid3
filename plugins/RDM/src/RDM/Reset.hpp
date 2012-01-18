// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_Reset_hpp
#define cf3_RDM_Reset_hpp

#include "solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { class Field; }
namespace RDM {


class RDM_API Reset : public cf3::solver::Action {

public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  Reset ( const std::string& name );

  /// Virtual destructor
  virtual ~Reset() {}

  /// Get the class name
  static std::string type_name () { return "Reset"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_fields();
  void config_field_tags();

private: // data

  std::vector< Handle<mesh::Field> > m_fields;

};

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3

#endif // cf3_RDM_Reset_hpp
