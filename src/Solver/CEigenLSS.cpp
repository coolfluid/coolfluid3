// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <set>

#include "coolfluid_config.h"

#ifdef CF_HAVE_SUPERLU
  #include <Eigen/SuperLUSupport>
#else
  #define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
  #include <Eigen/SparseExtra>
#endif

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"

#include "Mesh/CField2.hpp"

#include "CEigenLSS.hpp"

namespace CF {
namespace Solver {

using namespace CF::Mesh;
  
CF::Common::ComponentBuilder < CEigenLSS, Common::Component, LibSolver > aCeigenLSS_Builder;

CEigenLSS::CEigenLSS ( const std::string& name ) : Component ( name )
{
}

void CEigenLSS::resize ( Uint nb_dofs )
{
  if(nb_dofs == m_system_matrix.rows())
    return;
  
  m_system_matrix.resize(nb_dofs, nb_dofs);
  m_rhs.resize(nb_dofs);
  m_solution.resize(nb_dofs);
  
  m_system_matrix.setZero();
  m_rhs.setZero();
}

Uint CEigenLSS::size() const
{
  return m_system_matrix.cols();
}

Real& CEigenLSS::at(const CF::Uint row, const CF::Uint col)
{
#ifdef CF_HAVE_SUPERLU
  return m_system_matrix.coeffRef(row, col);
#else
  // Hack to make sure the system is symmetric in case SUperLU is not found.
  // We must only store the lower triangular part and have a diagonal with only non-zeros in this case
  static Real dummyval = 0.;
  if(row >= col)
    return m_system_matrix.coeffRef(row, col);
  else
    return dummyval;
#endif
}


void CEigenLSS::set_zero()
{
  m_system_matrix.setZero();
  m_rhs.setZero();
}

void CEigenLSS::set_dirichlet_bc(const CF::Uint row, const CF::Real value, const CF::Real coeff)
{
  for(int k=0; k < m_system_matrix.outerSize(); ++k)
  {
    for(MatrixT::InnerIterator it(m_system_matrix, k); it; ++it)
    {
      if(it.row() == row && it.col() != row)
      {
        it.valueRef() = 0.;
      }
      else if(it.row() == row && it.col() == row)
      {
        it.valueRef() = coeff;
      }
      else if(it.row() != row && it.col() == row)
      {
        m_rhs[it.row()] -= it.value() * value;
        it.valueRef() = 0.;
      }
    }
  }
  
  m_rhs[row] = coeff * value;
}


RealVector& CEigenLSS::rhs()
{
  return m_rhs;
}

const RealVector& CEigenLSS::solution()
{
  return m_solution;
}


void CEigenLSS::solve()
{
  Eigen::SparseMatrix<Real> A(m_system_matrix);
#ifdef CF_HAVE_SUPERLU
  Eigen::SparseLU<Eigen::SparseMatrix<Real>,Eigen::SuperLU> lu_of_A(A);
  if(!lu_of_A.solve(rhs(), &m_solution))
    throw Common::FailedToConverge(FromHere(), "Solution failed.");
#else
  // WARNING: This only works for symmetric matrices
  Eigen::SparseLLT< Eigen::SparseMatrix<Real> > llt(A);
  m_solution = llt.solve(m_rhs);
#endif
}

void CEigenLSS::print_matrix()
{
  std::cout << m_system_matrix << std::endl;
}


void increment_solution(const RealVector& solution, const std::vector<std::string>& field_names, const std::vector<std::string>& var_names, const std::vector<Uint>& var_sizes, CMesh& solution_mesh)
{ 
  const Uint nb_vars = var_names.size();
  
  std::vector<Uint> var_offsets;
  var_offsets.push_back(0);
  for(Uint i = 0; i != nb_vars; ++i)
  {
    var_offsets.push_back(var_offsets.back() + var_sizes[i]);
  }
  
  // Copy the data to the fields, where each field value is incremented with the value from the solution vector 
  std::set<std::string> unique_field_names;
  BOOST_FOREACH(const std::string& field_name, field_names)
  {
    if(unique_field_names.insert(field_name).second)
    {
      CField2& field = *solution_mesh.get_child_ptr(field_name)->as_ptr<CField2>();
      CTable<Real>& field_table = field.data();
      const Uint field_size = field_table.size();
      for(Uint row_idx = 0; row_idx != field_size; ++row_idx)
      {
        CTable<Real>::Row row = field_table[row_idx];
        for(Uint i = 0; i != nb_vars; ++i)
        {
          if(field_names[i] != field_name)
            continue;
          
          const Uint solution_begin = var_offsets.back() * row_idx + var_offsets[i];
          const Uint solution_end = solution_begin + var_sizes[i];
          Uint field_idx = field.var_index(var_names[i]);
          
          cf_assert(field.var_type(var_names[i]) == var_sizes[i]);
          
          for(Uint sol_idx = solution_begin; sol_idx != solution_end; ++sol_idx)
          {
            row[field_idx++] += solution[sol_idx];
          }
        }
      }
    }
  }
}


} // Solver
} // CF
