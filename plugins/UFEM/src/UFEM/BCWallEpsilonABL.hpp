// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BCWallEpsilonABL_hpp
#define cf3_UFEM_BCWallEpsilonABL_hpp


#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/BlockAccumulator.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

#include "LibUFEM.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Set the value of epsilon consistently with the use of wall functions
class UFEM_API BCWallEpsilonABL : public solver::actions::Proto::ProtoAction
{
public:

  /// Contructor
  /// @param name of the component
  BCWallEpsilonABL ( const std::string& name );

  virtual ~BCWallEpsilonABL();

  /// Get the class name
  static std::string type_name () { return "BCWallEpsilonABL"; }

private:
  cf3::solver::actions::Proto::SystemRHS rhs;
  cf3::solver::actions::Proto::SystemMatrix system_matrix;
  cf3::solver::actions::Proto::DirichletBC dirichlet;

  Real m_theta = 0.5;
  Real m_sigma_epsilon = 1.3;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BCWallEpsilonABL_hpp
