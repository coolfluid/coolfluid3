// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_Term_hpp
#define cf3_SFDM_Term_hpp

#include "solver/Action.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {

namespace mesh   { class Field; class SpaceFields; class Cells; }
namespace SFDM {

/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API Term : public cf3::solver::Action {

public: // typedefs

  /// provider
  
  

public: // functions

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name );

  /// Virtual destructor
  virtual ~Term();

  /// Get the class name
  static std::string type_name () { return "Term"; }

  /// @name ACCESSORS
  //@{

  mesh::SpaceFields& field_group() { return *m_field_group; }

  mesh::Field& solution()    { return *m_solution; }

  mesh::Field& residual()    { return *m_residual; }

  mesh::Field& wave_speed()  { return *m_wave_speed; }

  mesh::Field& jacob_det()   { return *m_jacob_det; }

  //@} END ACCESSORS

protected: // function

  void link_fields();

protected: // data

  Handle<mesh::SpaceFields> m_field_group;

  Handle<mesh::Field> m_solution;     ///< access to the solution field

  Handle<mesh::Field> m_residual;     ///< access to the residual field

  Handle<mesh::Field> m_wave_speed;   ///< access to the wave_speed field

  Handle<mesh::Field> m_jacob_det;    ///< access to the jacobian_determinant field

};

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

#endif // cf3_SFDM_Term_hpp
