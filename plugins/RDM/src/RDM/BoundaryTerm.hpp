// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_BoundaryTerm_hpp
#define cf3_RDM_BoundaryTerm_hpp

#include "solver/Action.hpp"

#include "RDM/LibRDM.hpp"


namespace cf3 {

namespace mesh { class Field; }

namespace RDM {

  class ElementLoop;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API BoundaryTerm : public cf3::solver::Action {

public: // typedefs

  typedef boost::shared_ptr< BoundaryTerm > Ptr;
  typedef boost::shared_ptr< BoundaryTerm const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  BoundaryTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~BoundaryTerm();

  /// Get the class name
  static std::string type_name () { return "BoundaryTerm"; }

  /// Identifies if this boundary condition is wek or strong
  /// A strong BC forces directly the solution values, whereas a weak is applied
  /// by changing the contribution of the equations, therefore indirectly.
  /// @returns if this term is a weak term
  virtual bool is_weak() const = 0;

  /// @return the element loop for the element type identified by the string
  /// @post will create the element loop if does not exist
  ElementLoop& access_element_loop( const std::string& type_name );

  /// @name ACCESSORS
  //@{

  mesh::Field& solution()    { return *m_solution.lock(); }

  mesh::Field& residual()    { return *m_residual.lock(); }

  mesh::Field& wave_speed()  { return *m_wave_speed.lock(); }

  //@} END ACCESSORS

protected: // function

  void link_fields();

protected: // data

  boost::weak_ptr<mesh::Field> m_solution;     ///< access to the solution field

  boost::weak_ptr<mesh::Field> m_residual;     ///< access to the residual field

  boost::weak_ptr<mesh::Field> m_wave_speed;   ///< access to the wave_speed field

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_BoundaryTerm_hpp
