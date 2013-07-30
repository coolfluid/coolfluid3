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
#include "physics/MatrixTypes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

template < typename DATA, Uint NB_DIM, Uint NB_EQS >
class solver_API RiemannSolver : public common::Component
{
public:
  typedef DATA Data;
  static const Uint NDIM = NB_DIM;
  static const Uint NEQS = NB_EQS;

  typedef typename physics::MatrixTypes<NDIM,NEQS>::ColVector_NDIM    ColVector_NDIM;
  typedef typename physics::MatrixTypes<NDIM,NEQS>::RowVector_NEQS    RowVector_NEQS;

  RiemannSolver(const std::string& name) : common::Component(name)
  {
    regist_typeinfo(this);
  }

  static std::string type_name () { return "RiemannSolver"; }

  virtual ~RiemannSolver() {}

  virtual void compute_riemann_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                                     RowVector_NEQS& flux, Real& wave_speed ) = 0;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_RiemannSolver_hpp
