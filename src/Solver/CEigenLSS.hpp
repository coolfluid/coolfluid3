// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CEigenLSS_hpp
#define CF_Solver_CEigenLSS_hpp

////////////////////////////////////////////////////////////////////////////////

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>

#include "Common/Component.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"

#include "LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// CEigenLSS component class
/// This class stores a linear system for use by proto expressions
/// @author Bart Janssens
class Solver_API CEigenLSS : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CEigenLSS> Ptr;
  typedef boost::shared_ptr<CEigenLSS const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CEigenLSS ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "CEigenLSS"; }    
  
  void set_config_file(const std::string& path);
  
  /// Set the number of equations
  void resize ( Uint nb_dofs );
  
  /// Number of equations
  Uint size() const;
  
  /// Access to the elements
  Real& at(const Uint row, const Uint col);
  
  /// Zero the system (RHS and system matrix)
  void set_zero();
  
  /// Set a dirichlet BC value, zeroing the corresponding row and column and adjusting the RHS
  void set_dirichlet_bc(const Uint row, const Real value, const Real coeff = 1.);
  
  /// Reference to the RHS vector
  RealVector& rhs();
  
  /// Const access to the solution
  const RealVector& solution();
  
  /// Solve the system and store the result in the solution vector
  void solve();
  
  void print_matrix();
  
private:
  /// System matrix
  typedef Eigen::DynamicSparseMatrix<Real, Eigen::RowMajor> MatrixT;
  MatrixT m_system_matrix;
  
  /// Right hand side
  RealVector m_rhs;
  
  /// Solution
  RealVector m_solution;
};

/// Helper function to increment the solution field(s) with the given solution vector from a LSS, i.e. treat the solution vector as the differece between the new  and old field values
void increment_solution(const RealVector& solution, const std::vector<std::string>& field_names, const std::vector<std::string>& var_names, const std::vector<Uint>& var_sizes, Mesh::CMesh& solution_mesh);

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF


////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CEigenLSS_hpp
