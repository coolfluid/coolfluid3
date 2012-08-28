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

/// @brief Compute residual jacobian (dR/dQ) using perturbation method
///
/// This jacobian has dimensions (nb_sol_pts*nb_eqs)x(nb_sol_pts*nb_eqs).
/// It is computed by perturbing every solution variable of every point seperately,
/// computing the residual for every of these perturbation, and seeing the
/// influence to the other variables of every point.
///
/// Example: Imagine cell with 2 solution points (p1, p2) and 2 variables (v1, v2)
/** @verbatim
         [ dR(p1,v1)/dQ(p1,v1)    dR(p1,v1)/dQ(p1,v2)    dR(p1,v1)/dQ(p2,v1)    dR(p1,v1)/dQ(p2,v2) ]
 dR/dQ = [ dR(p1,v2)/dQ(p1,v1)    dR(p1,v2)/dQ(p1,v2)    dR(p1,v2)/dQ(p2,v1)    dR(p1,v2)/dQ(p2,v2) ]
         [ dR(p2,v1)/dQ(p1,v1)    dR(p2,v1)/dQ(p1,v2)    dR(p2,v1)/dQ(p2,v1)    dR(p2,v1)/dQ(p2,v2) ]
         [ dR(p2,v2)/dQ(p1,v1)    dR(p2,v2)/dQ(p1,v2)    dR(p2,v2)/dQ(p2,v1)    dR(p2,v2)/dQ(p2,v2) ]
@endverbatim */
/// In computational expense, this means that the residual, computed in
/// one element, has to be evaluated (nb_sol_pts*nb_eqs) times, once for every column.
///
/// @author Willem Deconinck, Matteo Parsani
class sdm_API ComputeCellJacobianPerturb : public common::Component
{
public: // functions

  /// @def Allow Eigen-type members
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// @brief Get the class name
  static std::string type_name () { return "ComputeCellJacobianPerturb"; }

  /// @brief Contructor
  /// @param name of the component
  ComputeCellJacobianPerturb ( const std::string& name );

  /// @brief Destructor
  virtual ~ComputeCellJacobianPerturb() {};

  /// @brief Prepare to loop over given cells
  /// @return true if cells can be used to loop over
  virtual bool loop_cells(const Handle<mesh::Cells const>& cells);

  /// @brief Compute cell jacobian for given cell
  /// @param [in]  elem         element index in cells that are being looped over
  /// @param [out] cell_jacob   computed jacobian matrix with dimensions (nb_sol_pts*nb_eqs)x(nb_sol_pts*nb_eqs).
  ///
  /// Every element (i,j) is computed as
  /// @f[ \frac{R_i(Q + \delta Q_j) - R_i(Q)}{\delta Q_j} @f]
  /// with @f$ \delta Q_j @f$ defined as
  /// @f[ \delta Q_j = \sqrt{\varepsilon_{\mathrm{mach}}} \  \mathrm{sign}(Q_j) \ \max( |Q_j| , |Q_{\mathrm{ref},j}| ) @f]
  /// This makes sure that @f$ \delta Q_j @f$ is small enough, and non-zero.
  virtual void compute_jacobian(const Uint elem, RealMatrix& cell_jacob);

private: // data

  /// @brief Domain-discretization action, to help compute the residual
  Handle<DomainDiscretization> m_domain_discretization;

  /// @brief Space defined by the cells and solution
  Handle<mesh::Space const> m_space;

  /// @brief Solution being used to compute the residual
  Handle<mesh::Field> m_solution;

  /// @brief The residual
  Handle<mesh::Field> m_residual;

  /// @brief Reference solution (all elements should be different from zero)
  std::vector<Real>   m_ref_sol;

  /// @brief Small number, defined as sqrt(machine_precision)
  Real m_eps;

  /// @brief Current cell's number of solution points
  Uint m_nb_sol_pts;

  /// @brief Number of variables or equations to solve for
  Uint m_nb_vars;

  /// @brief Storage for unperturbed solution
  RealMatrix m_Q0;

  /// @brief Storage for unperturbed residual
  RealMatrix m_R0;
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_ComputeCellJacobianPerturb_hpp
