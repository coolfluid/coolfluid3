// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include <boost/utility.hpp>

#include "Math/LibMath.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/Component.hpp"
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

// don't forget to add entry in the create function and in the cmakelists.txt to hhok things in

/// @todo to be implemented an empty solver, which does nothing
//#include "Math/LSS/Trilinos/EmptySolver.hpp"
//#include "Math/LSS/Trilinos/EmptySolver.hpp"

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
  Component(name),
  m_is_created(false)
{
  options().add_option< OptionT<std::string> >( "solver" , "Trilinos" );
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::create(Comm::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices)
{
  if (m_is_created) destroy();
  std::string solvertype=options().option("solver").value();
  std::string typestr;

  typestr="Trilinos";
  if (solvertype==typestr){
    #ifdef CF_HAVE_TRILINOS
      m_mat=new TrilinosMatrix("Matrix");
      m_rhs=new TrilinosVector("RHS");
      m_sol=new TrilinosVector("Solution");
      m_mat.create(cp,neq,node_connectivity,starting_indices,m_sol,m_rhs);
    #else
      throw Common::SetupError(FromHere(),"Trilinos is selected for linear solver, but COOLFluiD was not compiled with it.");
    #endif
  }
  m_is_created=true;
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::swap(LSS::Matrix::Ptr matrix, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs)
{
  if (!((matrix->is_created()==solution->is_created())&&(matrix->is_created()==rhs->is_created())))
    throw Common::BadValue(FromHere(),"Inconsistent states.");
  if (!((matrix->solvertype()==solution->solvertype())&&(matrix->solvertype()==rhs->solvertype())))
    throw Common::NotSupported(FromHere(),"Inconsistent linear solver types.");
  if (m_mat!=matrix) delete m_mat;
  if (m_rhs!=matrix) delete m_rhs;
  if (m_sol!=matrix) delete m_sol;
  m_mat=matrix;
  m_rhs=rhs;
  m_sol=solution;
  options().option("solver").put_value(matrix->solvertype());
  m_is_created=true;
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::destroy()
{
  delete m_mat;
  delete m_sol;
  delete m_rhs;
  m_is_created=false;
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::solve()
{
  m_mat->solve(m_sol,m_rhs);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::set_values(const LSS::BlockAccumulator& values)
{
  CF_ASSERT(m_is_created);
  m_mat->set_values(values);
  m_sol->set_sol_values(values);
  m_rhs->set_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::add_values(const LSS::BlockAccumulator& values)
{
  CF_ASSERT(m_is_created);
  m_mat->add_values(values);
  m_sol->add_sol_values(values);
  m_rhs->add_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::get_values(LSS::BlockAccumulator& values)
{
  CF_ASSERT(m_is_created);
  m_mat->get_values(values);
  m_sol->get_sol_values(values);
  m_rhs->get_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::dirichlet(const Uint iblockrow, const Uint ieq, LSS::Vector& values, bool preserve_symmetry=false)
{
  CF_ASSERT(m_is_created);
  /// @todo finish
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::periodicity (const Uint iblockrow_to, const Uint iblockrow_from)
{
  CF_ASSERT(m_is_created);
  /// @todo finish
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::set_diagonal(const Vector& diag)
{
  CF_ASSERT(m_is_created);
  m_mat->set_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::add_diagonal(const LSS::Vector& diag)
{
  CF_ASSERT(m_is_created);
  m_mat->add_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::get_diagonal(LSS::Vector& diag)
{
  CF_ASSERT(m_is_created);
  m_mat->get_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::reset(Real reset_to=0.)
{
  CF_ASSERT(m_is_created);
  m_mat->reset(reset_to);
  m_sol->reset(reset_to);
  m_rhs->reset(reset_to);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::print(std::iostream& stream)
{
  if (m_is_created)
  {
    m_mat->print(stream);
    m_sol->print(stream);
    m_rhs->print(stream);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::print(const std::string& filename)
{
  if (m_is_created)
  {
    std::ofstream ofs(filename.c_str());
    m_mat->print(ofs);
    m_sol->print(ofs);
    m_rhs->print(ofs);
    ofs.close();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

