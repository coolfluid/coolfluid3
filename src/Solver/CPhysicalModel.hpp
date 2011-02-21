// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CPhysicalModel_hpp
#define CF_Solver_CPhysicalModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Component providing information about the physics, i.e. number of equations, variables, ...
/// @author Bart Janssens
class Solver_API CPhysicalModel : public Common::Component {

public: //typedefs

  typedef boost::shared_ptr<CPhysicalModel> Ptr;
  typedef boost::shared_ptr<CPhysicalModel const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CPhysicalModel ( const std::string& name );

  /// Virtual destructor
  virtual ~CPhysicalModel();

  /// Get the class name
  static std::string type_name () { return "CPhysicalModel"; }

  //////////////////////////////////
  // CPhysicalModel specific
  /////////////////////////////////
  
  /// @return dimensionality of the problem, which is
  ///         the number of spatial coordinates used in the PDEs
  Uint dimensions() const { return m_dim; }
  
  /// @return the number of degrees of freedom (DOFs)
  Uint nb_dof() const { return m_nbdofs; }

private: // data

  /// type of the physcial model
  std::string m_type;

  /// dimensionality of physics
  Uint m_dim;

  /// number of degrees of freedom
  Uint m_nbdofs;

}; // CPhysicalModel

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CPhysicalModel_hpp
