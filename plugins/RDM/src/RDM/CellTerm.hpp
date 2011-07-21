// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CellTerm_hpp
#define CF_RDM_CellTerm_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {

namespace Mesh { class CField; }

namespace RDM {

  class ElementLoop;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API CellTerm : public CF::Solver::Action {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< CellTerm > Ptr;
  typedef boost::shared_ptr< CellTerm const > ConstPtr;

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

  Mesh::CField& solution()    { return *m_solution.lock(); }

  Mesh::CField& residual()    { return *m_residual.lock(); }

  Mesh::CField& wave_speed()  { return *m_wave_speed.lock(); }

  //@} END ACCESSORS

protected: // function

  void link_fields();

protected: // data

  boost::weak_ptr<Mesh::CField> m_solution;     ///< access to the solution field

  boost::weak_ptr<Mesh::CField> m_residual;     ///< access to the residual field

  boost::weak_ptr<Mesh::CField> m_wave_speed;   ///< access to the wave_speed field

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_CellTerm_hpp
