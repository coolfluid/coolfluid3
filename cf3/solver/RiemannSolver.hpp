// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_RiemannSolver_hpp
#define cf3_solver_RiemannSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

template < typename TERM >
class solver_API RiemannSolver : public common::Component
{
public:

  RiemannSolver(const std::string& name) : common::Component(name)
  {
    regist_typeinfo(this);
  }

  static std::string type_name () { return "RiemannSolver<"+TERM::type_name()+">"; }

  virtual ~RiemannSolver() {}

  virtual void compute_riemann_flux( const typename TERM::DATA& left, const typename TERM::DATA& right, const typename TERM::ColVector_NDIM& normal,
                                     typename TERM::RowVector_NEQS& flux, Real& wave_speed ) = 0;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_RiemannSolver_hpp
