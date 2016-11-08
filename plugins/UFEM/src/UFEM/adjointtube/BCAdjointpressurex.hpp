// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BCAdjointpressurex_hpp
#define cf3_UFEM_BCAdjointpressurex_hpp

#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"


#include "LibUFEMAdjointTube.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {
namespace adjointtube {
/// Boundary condition to hold the value of a field at a value given by another (or the same) field
class UFEM_API BCAdjointpressurex : public solver::actions::Proto::ProtoAction
{
public:

  /// Contructor
  /// @param name of the component
  BCAdjointpressurex ( const std::string& name );
  
  virtual ~BCAdjointpressurex();

  /// Get the class name
  static std::string type_name () { return "BCAdjointpressurex"; }
  

private:
  cf3::solver::actions::Proto::DirichletBC m_dirichlet;
  Uint m_turbulence = 0;
  Real m_c_epsilon_1 = 1.44;
  Real m_c_mu = 0.09;

};
} // adjoint
} // UFEM
} // cf3


#endif // cf3_UFEM_BCAdjointpressurex_hpp
