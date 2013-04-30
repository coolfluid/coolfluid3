// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_TermComputer_hpp
#define cf3_solver_TermComputer_hpp

#include "common/Action.hpp"
#include "math/MatrixTypes.hpp"
#include "solver/LibSolver.hpp"

// Forward declares
namespace cf3 
{ 
  namespace mesh 
  { 
    class Entities; 
    class Field; 
  } 
}

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

/////////////////////////////////////////////////////////////////////////////////////

/// @brief Computes a term of a system of equations by looping over elements
/// @author Willem Deconinck
class solver_API TermComputer : public common::Action
{
public:
  
  /// @brief Constructor
  TermComputer ( const std::string& name );

  /// Virtual destructor
  virtual ~TermComputer() {}

  /// @Get the class name
  static std::string type_name () { return "TermComputer"; }

  // Compute the term in configured fields
  virtual void execute();

  /// @brief Compute the term in given fields
  virtual void compute_term(mesh::Field& term, mesh::Field& wave_speed);

  /// @brief Initialize the term computer for cells component
  virtual bool loop_cells(const Handle<mesh::Entities const>& cells) = 0;

  /// @brief Compute the term for given element in given vectors
  virtual void compute_term(const Uint elem_idx, std::vector<RealVector>& term, std::vector<Real>& wave_speed) = 0;

 private:

  Handle<mesh::Field> m_term_field;
  Handle<mesh::Field> m_term_ws;
  
  std::vector<RealVector> m_tmp_term;
  std::vector<Real>       m_tmp_ws;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_TermComputer_hpp
