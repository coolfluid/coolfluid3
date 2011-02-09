// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CModel_hpp
#define CF_Solver_CModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
namespace Mesh {
  class CDomain;
}
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// CModel is the top most component on a simulation structure
/// CModel now stores:
/// - Physical model
/// - Iterative solver
/// - Discretization
/// @author Martin Vymazal
class Solver_API CModel : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CModel> Ptr;
  typedef boost::shared_ptr<CModel const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CModel ( const std::string& name );

  /// Virtual destructor
  virtual ~CModel();

  /// Get the class name
  static std::string type_name () { return "CModel"; }

  /// Simulates this model
  virtual void simulate() = 0;

  /// @name SIGNALS
  //@{

  /// Signal to start solving
  void signal_simulate ( Common::XmlNode& node );

  //@} END SIGNALS
  
  Mesh::CDomain& domain() { return *m_domain; }

protected:
  
  boost::shared_ptr<Mesh::CDomain> m_domain;
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CModel_hpp
