// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <fstream>

#include <boost/utility.hpp>

#include "Math/LibMath.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/Component.hpp"
#include "Common/OptionT.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Math/LSS/System.hpp"
#include "Math/LSS/Matrix.hpp"
#include "Math/LSS/Vector.hpp"
#include "Math/LSS/BlockAccumulator.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file System.cpp implementation of LSS::System
  @author Tamas Banyai
**/

////////////////////////////////////////////////////////////////////////////////////////////

#include "Math/LSS/EmptyLSS/EmptyLSSVector.hpp"
#include "Math/LSS/EmptyLSS/EmptyLSSMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_TRILINOS
  #include "Math/LSS/Trilinos/TrilinosMatrix.hpp"
  #include "Math/LSS/Trilinos/TrilinosVector.hpp"
#endif // CF_HAVE_TRILINOS

////////////////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Math;

////////////////////////////////////////////////////////////////////////////////////////////

LSS::System::System(const std::string& name) :
  Component(name)
{
  options().add_option< CF::Common::OptionT<std::string> >( "solver" , "Trilinos" );
  m_mat.reset();
  m_sol.reset();
  m_rhs.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::create(CF::Common::Comm::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices)
{

  if (is_created()) destroy();
  std::string solvertype=options().option("solver").value_str();

  if (solvertype=="EmptyLSS"){
      m_mat=(LSS::Matrix::Ptr) new LSS::EmptyLSSMatrix("Matrix");
      m_rhs=(LSS::Vector::Ptr) new LSS::EmptyLSSVector("RHS");
      m_sol=(LSS::Vector::Ptr) new LSS::EmptyLSSVector("Solution");
  }

  if (solvertype=="Trilinos"){
    #ifdef CF_HAVE_TRILINOS
      m_mat=(LSS::Matrix::Ptr) new LSS::TrilinosMatrix("Matrix");
      m_rhs=(LSS::Vector::Ptr) new LSS::TrilinosVector("RHS");
      m_sol=(LSS::Vector::Ptr) new LSS::TrilinosVector("Solution");
    #else
      throw Common::SetupError(FromHere(),"Trilinos is selected for linear solver, but COOLFluiD was not compiled with it.");
    #endif
  }

  m_rhs->create(cp,neq);
  m_sol->create(cp,neq);
  m_mat->create(cp,neq,node_connectivity,starting_indices,m_sol,m_rhs);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::swap(LSS::Matrix::Ptr matrix, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs)
{
  if (m_mat->compatible(solution,rhs))
  {
  if ((matrix->is_created()!=solution->is_created())||(matrix->is_created()!=rhs->is_created()))
    throw Common::SetupError(FromHere(),"Inconsistent states.");
  if ((matrix->solvertype()!=solution->solvertype())||(matrix->solvertype()!=rhs->solvertype()))
    throw Common::NotSupported(FromHere(),"Inconsistent linear solver types.");
  if ((matrix->neq()!=solution->neq())||(matrix->neq()!=rhs->neq()))
    throw Common::BadValue(FromHere(),"Inconsistent number of equations.");
  if ((matrix->blockcol_size()!=solution->blockrow_size())||(matrix->blockcol_size()!=rhs->blockrow_size()))
    throw Common::BadValue(FromHere(),"Inconsistent number of block rows.");
  if (m_mat!=matrix) m_mat=matrix;
  if (m_rhs!=rhs) m_rhs=rhs;
  if (m_sol!=solution) m_sol=solution;
  options().option("solver").change_value(matrix->solvertype());
  } else {
    throw Common::NotSupported(FromHere(),"System of '" + matrix->name() + "' x '" + solution->name() + "' = '" + rhs->name() + "' is incompatible." );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::destroy()
{
  m_mat.reset();
  m_sol.reset();
  m_rhs.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::solve()
{
  cf_assert(is_created());
  m_mat->solve(m_sol,m_rhs);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::set_values(const LSS::BlockAccumulator& values)
{
  cf_assert(is_created());
  m_mat->set_values(values);
  m_sol->set_sol_values(values);
  m_rhs->set_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::add_values(const LSS::BlockAccumulator& values)
{
  cf_assert(is_created());
  m_mat->add_values(values);
  m_sol->add_sol_values(values);
  m_rhs->add_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::get_values(LSS::BlockAccumulator& values)
{
  cf_assert(is_created());
  m_mat->get_values(values);
  m_sol->get_sol_values(values);
  m_rhs->get_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::dirichlet(const Uint iblockrow, const Uint ieq, const Real value, const bool preserve_symmetry)
{
  cf_assert(is_created());
  std::vector<Real> v;
  if (preserve_symmetry)
  {
    m_mat->get_column_and_replace_to_zero(iblockrow,ieq,v);
    if (preserve_symmetry)
      for (int i=0; i<(const int)v.size(); i++)
        m_rhs->add_value(i,-v[i]*value);
  }
  m_mat->set_row(iblockrow,ieq,1.,0.);
  m_sol->set_value(iblockrow,ieq,value);
  m_rhs->set_value(iblockrow,ieq,value);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::periodicity (const Uint iblockrow_to, const Uint iblockrow_from)
{
  cf_assert(is_created());
  m_mat->tie_blockrow_pairs(iblockrow_to,iblockrow_from);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::set_diagonal(const std::vector<Real>& diag)
{
  cf_assert(is_created());
  m_mat->set_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::add_diagonal(const std::vector<Real>& diag)
{
  cf_assert(is_created());
  m_mat->add_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::get_diagonal(std::vector<Real>& diag)
{
  cf_assert(is_created());
  m_mat->get_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::reset(Real reset_to)
{
  cf_assert(is_created());
  m_mat->reset(reset_to);
  m_sol->reset(reset_to);
  m_rhs->reset(reset_to);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::print(Common::LogStream& stream)
{
  if (is_created())
  {
    m_mat->print(stream);
    m_sol->print(stream);
    m_rhs->print(stream);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::print(std::ostream& stream)
{
  if (is_created())
  {
    m_mat->print(stream);
    m_sol->print(stream);
    m_rhs->print(stream);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::print(const std::string& filename)
{
  if (is_created())
  {
    std::ofstream ofs(filename.c_str());
    m_mat->print(ofs);
    m_sol->print(ofs);
    m_rhs->print(ofs);
    ofs.close();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

const bool LSS::System::is_created()
{
  int numcreated=0;
  if (m_mat!=nullptr) if (m_mat->is_created()) numcreated+=1;
  if (m_sol!=nullptr) if (m_sol->is_created()) numcreated+=2;
  if (m_rhs!=nullptr) if (m_rhs->is_created()) numcreated+=4;
  switch (numcreated) {
    case 0 : return false;
    case 7 : return true;
    default: throw Common::SetupError(FromHere(),"LSS System is in inconsistent state.");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////


