// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ComputeCellJacobianPerturb_hpp
#define cf3_sdm_ComputeCellJacobianPerturb_hpp

#include "common/Action.hpp"
#include "math/MatrixTypes.hpp"
#include "sdm/LibSDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3  {

namespace mesh { class Field; class Space; class Cells; }
namespace common {
  template <typename T> class DynTable; }

namespace sdm  {

class DomainDiscretization;

/////////////////////////////////////////////////////////////////////////////////////

class sdm_API ComputeCellJacobianPerturb : public common::Component
{
public: // functions

  /// Get the class name
  static std::string type_name () { return "ComputeCellJacobianPerturb"; }

  /// Contructor
  /// @param name of the component
  ComputeCellJacobianPerturb ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeCellJacobianPerturb() {};

  virtual bool loop_cells(const Handle<mesh::Cells const>& cells);

  virtual void compute_jacobian(const Uint elem, RealMatrix& cell_jacob);

private: // data

  // macro eigen

  Handle<DomainDiscretization> m_domain_discretization;

  Handle<mesh::Field> m_solution;

  Handle<mesh::Field> m_residual;

  std::vector<Real>   m_ref_sol;

  Real m_eps;

  Handle<mesh::Space const> m_space;

  Uint m_nb_sol_pts;
  Uint m_nb_vars;

  RealMatrix m_Q0;
  RealMatrix m_R0;

};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_ComputeCellJacobianPerturb_hpp
