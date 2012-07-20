// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ComputeCellJacobianPerturb_hpp
#define cf3_sdm_ComputeCellJacobianPerturb_hpp

#include "common/Action.hpp"
#include "sdm/LibSDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3  {

namespace mesh { class Field; }
namespace common {
  template <typename T> class DynTable; }

namespace sdm  {

class DomainDiscretization;

/////////////////////////////////////////////////////////////////////////////////////

class sdm_API ComputeCellJacobianPerturb : public common::Action
{
public: // functions

  /// Get the class name
  static std::string type_name () { return "ComputeCellJacobianPerturb"; }

  /// Contructor
  /// @param name of the component
  ComputeCellJacobianPerturb ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeCellJacobianPerturb() {};

  virtual void execute();

private:

  void create_backup_fields();

private: // data

  Handle<DomainDiscretization> m_domain_discretization;

  Handle<mesh::Field> m_solution;

  Handle<mesh::Field> m_residual;

  Handle<mesh::Field> m_residual_backup;

  Handle<mesh::Field> m_solution_backup;

  Handle< common::DynTable<Real> > m_cell_jacobian;

  std::vector<Real>   m_ref_sol;

  Real m_eps;

};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_ComputeCellJacobianPerturb_hpp
