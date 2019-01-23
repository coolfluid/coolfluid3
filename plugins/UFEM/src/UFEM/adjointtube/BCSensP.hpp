// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BCSensP_hpp
#define cf3_UFEM_BCSensP_hpp

#include "solver/actions/Proto/BlockAccumulator.hpp"
#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

#include "LibUFEMAdjointTube.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {
namespace adjointtube {
/// Boundary condition to hold the value of a field at a value given by another (or the same) field
class UFEM_API BCSensP: public solver::actions::Proto::ProtoAction
{
public:

  /// Contructor
  /// @param name of the component
  BCSensP ( const std::string& name );

  virtual ~BCSensP();

  /// Get the class name
  static std::string type_name () { return "BCSensP"; }


private:
  cf3::solver::actions::Proto::SystemRHS m_rhs;
  //cf3::solver::actions::Proto::DirichletBC m_dirichlet;

};
} // adjoint
} // UFEM
} // cf3


#endif // cf3_UFEM_BCAdjointpressurex_hpp
