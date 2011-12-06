// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_CellTerm_hpp
#define cf3_RDM_CellTerm_hpp

#include "solver/Action.hpp"

#include "RDM/LibRDM.hpp"

namespace cf3 {

namespace mesh { class Field; }

namespace RDM {

  class ElementLoop;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API CellTerm : public cf3::solver::Action {

public: // typedefs

  /// provider
  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CellTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~CellTerm();

  /// Get the class name
  static std::string type_name () { return "CellTerm"; }

  ElementLoop& access_element_loop( const std::string& type_name );

  /// @name ACCESSORS
  //@{

  Handle<mesh::Field> solution()    { return m_solution; }

  Handle<mesh::Field> residual()    { return m_residual; }

  Handle<mesh::Field> wave_speed()  { return m_wave_speed; }

  //@} END ACCESSORS

protected: // function

  void link_fields();

protected: // data

  Handle<mesh::Field> m_solution;     ///< access to the solution field

  Handle<mesh::Field> m_residual;     ///< access to the residual field

  Handle<mesh::Field> m_wave_speed;   ///< access to the wave_speed field

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_CellTerm_hpp
