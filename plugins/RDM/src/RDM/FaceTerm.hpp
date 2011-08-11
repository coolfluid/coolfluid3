// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_FaceTerm_hpp
#define CF_RDM_FaceTerm_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {

namespace Mesh { class Field; }

namespace RDM {

  class ElementLoop;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API FaceTerm : public CF::Solver::Action {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< FaceTerm > Ptr;
  typedef boost::shared_ptr< FaceTerm const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  FaceTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~FaceTerm();

  /// Get the class name
  static std::string type_name () { return "FaceTerm"; }

  ElementLoop& access_element_loop( const std::string& type_name );

  /// @name ACCESSORS
  //@{

  Mesh::Field& solution()    { return *m_solution.lock(); }

  Mesh::Field& residual()    { return *m_residual.lock(); }

  Mesh::Field& wave_speed()  { return *m_wave_speed.lock(); }

  //@} END ACCESSORS

protected: // function

  void link_fields();

protected: // data

  boost::weak_ptr<Mesh::Field> m_solution;     ///< access to the solution field

  boost::weak_ptr<Mesh::Field> m_residual;     ///< access to the residual field

  boost::weak_ptr<Mesh::Field> m_wave_speed;   ///< access to the wave_speed field

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_FaceTerm_hpp
