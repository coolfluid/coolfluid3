// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CSolver_hpp
#define CF_Solver_CSolver_hpp

#include <boost/scoped_ptr.hpp>

#include "Common/Component.hpp"
#include "Common/CActionDirector.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {

namespace Mesh { class CDomain; class CMesh; }

namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// Base class for solvers. By default, actions added through the CActionDirector interface are
/// executed in the configured order. Override the execute function to change behavior.
/// Adds an option to set a domain
/// @author Tiago Quintino
/// @author Willem Deconinck
/// @author Bart Janssens
class Solver_API CSolver : public Common::CActionDirector {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CSolver> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~CSolver();

  /// Get the class name
  static std::string type_name () { return "CSolver"; }

  /// Called when a mesh is loaded into the domain that is associated with this solver
  virtual void mesh_loaded(Mesh::CMesh& mesh);

protected:

  /// Checked access to the domain (throws if domain is not properly configured)
  Mesh::CDomain& domain();

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

#endif // CF_Solver_CSolver_hpp
