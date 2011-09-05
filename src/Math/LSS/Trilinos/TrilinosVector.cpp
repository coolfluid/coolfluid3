// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include "Common/Assertions.hpp"
#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"
#include "Math/LSS/Trilinos/TrilinosVector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosVector.cpp Implementation of LSS::vector interface for Trilinos package.
  @author Tamas Banyai

  The chosen tool is epetra vector which has been implemented.
**/

////////////////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Math;
using namespace CF::Math::LSS;

////////////////////////////////////////////////////////////////////////////////////////////

TrilinosVector::TrilinosVector(const std::string& name) :
  LSS::Vector(name),
  m_neq(0),
  m_blockrow_size(0),
  m_is_created(false),
  m_vec(0),
  m_converted_indices(0),
  m_comm(Common::Comm::PE::instance().communicator())
{
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::create(Common::Comm::CommPattern& cp, Uint neq)
{
  // if built
  if (m_is_created) destroy();

  // get global ids vector
  int *gid=(int*)cp.gid()->pack();

  // prepare intermediate data
  int nmyglobalelements=0;

  for (int i=0; i<(const int)cp.isUpdatable().size(); i++)
    if (cp.isUpdatable()[i])
      ++nmyglobalelements;

  // process local to matrix local numbering mapper
  int iupd=0;
  int ighost=nmyglobalelements;
  m_p2m.resize(0);
  m_p2m.reserve(cp.isUpdatable().size());
  for (int i=0; i<(const int)cp.isUpdatable().size(); i++)
  {
    if (cp.isUpdatable()[i]) { m_p2m.push_back(iupd++); }
    else { m_p2m.push_back(ighost++); }
  }

  // map (its actually blockmap insteady of rowmap, to involve ghosts)
  Epetra_BlockMap map(-1,cp.isUpdatable().size(),gid,neq,0,m_comm);

  // create matrix
  m_vec=Teuchos::rcp(new Epetra_Vector(map));
/*must be a bug in Trilinos, Epetra_FEVbrMatrix constructor is in Copy mode but it hangs up anyway
  more funny, when it gets out of scope and gets dealloc'd, everything survives according to memcheck
  map.~Epetra_BlockMap();
*/

  delete gid;
  m_neq=neq;
  m_blockrow_size=cp.isUpdatable().size();
  m_is_created=true;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::destroy()
{
  if (m_is_created) m_vec.reset();
  m_p2m.resize(0);
  m_p2m.reserve(0);
  m_neq=0;
  m_blockrow_size=0;
  m_is_created=false;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set_value(const Uint irow, const Real value)
{
  cf_assert(m_is_created);
  const int iblockrow=m_p2m[irow/m_neq];
  const int ieq=irow%m_neq;
  cf_assert(iblockrow>=m_blockrow_size);
  (*m_vec)[iblockrow*m_neq+ieq]=value;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::add_value(const Uint irow, const Real value)
{
  cf_assert(m_is_created);
  const int iblockrow=m_p2m[irow/m_neq];
  const int ieq=irow%m_neq;
  cf_assert(iblockrow>=m_blockrow_size);
  (*m_vec)[iblockrow*m_neq+ieq]+=value;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get_value(const Uint irow, Real& value)
{
  cf_assert(m_is_created);
  const int iblockrow=m_p2m[irow/m_neq];
  const int ieq=irow%m_neq;
  cf_assert(iblockrow>=m_blockrow_size);
  value=(*m_vec)[iblockrow*m_neq+ieq];
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set_value(const Uint iblockrow, const Uint ieq, const Real value)
{
  cf_assert(m_is_created);
  cf_assert(iblockrow>=m_blockrow_size);
  (*m_vec)[m_p2m[iblockrow]*m_neq+ieq]=value;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::add_value(const Uint iblockrow, const Uint ieq, const Real value)
{
  cf_assert(m_is_created);
  cf_assert(iblockrow>=m_blockrow_size);
  (*m_vec)[m_p2m[iblockrow]*m_neq+ieq]+=value;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get_value(const Uint iblockrow, const Uint ieq, Real& value)
{
  cf_assert(m_is_created);
  cf_assert(iblockrow>=m_blockrow_size);
  value=(*m_vec)[m_p2m[iblockrow]*m_neq+ieq];
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set_rhs_values(const BlockAccumulator& values)
{
/*
  cf_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<m_neq*numblocks) m_converted_indices.resize(m_neq*numblocks);
  int *conv=&m_converted_indices[0];
  for (int i=0; i<(const int)numblocks; i++)
    for (int j=0; j<(const int)m_neq; j++)
      *conv++=m_p2m[values.indices[i]*m_neq+j];
  TRILINOS_ASSERT(m_vec->ReplaceMyValues());
*/
  cf_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  int *idxs=&m_converted_indices[0];
  TRILINOS_ASSERT(m_vec->ReplaceMyValues(numblocks,0,&values.rhs[0],idxs));
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::add_rhs_values(const BlockAccumulator& values)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get_rhs_values(BlockAccumulator& values)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set_sol_values(const BlockAccumulator& values)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::add_sol_values(const BlockAccumulator& values)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get_sol_values(BlockAccumulator& values)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::reset(Real reset_to)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::print(Common::LogStream& stream)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::print(std::ostream& stream)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::print(const std::string& filename, std::ios_base::openmode mode)
{
  cf_assert(m_is_created);
  std::ofstream stream(filename.c_str(),mode);
  print(stream);
  stream.close();
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::data(std::vector<Real>& values)
{
  cf_assert(m_is_created);
}

////////////////////////////////////////////////////////////////////////////////////////////

