// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <set>

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

RealMatrix& CEigenLSS::matrix()
{
  return m_system_matrix;
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
  m_solution = matrix().colPivHouseholderQr().solve(rhs());
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
      CField2& field = *solution_mesh.get_child<CField2>(field_name);
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
