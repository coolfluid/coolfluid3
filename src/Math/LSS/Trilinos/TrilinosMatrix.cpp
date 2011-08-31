// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "Common/Assertions.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/Log.hpp"
#include "Math/LSS/Trilinos/TrilinosMatrix.hpp"
#include "Math/LSS/Trilinos/TrilinosVector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosMatrix.cpp implementation of LSS::TrilinosMatrix
  @author Tamas Banyai

  It is based on Trilinos's FEVbrMatrix.
**/

////////////////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Math;
using namespace CF::Math::LSS;

////////////////////////////////////////////////////////////////////////////////////////////

TrilinosMatrix::TrilinosMatrix(const std::string& name) :
  LSS::Matrix(name),
  m_mat(0),
  m_is_created(false),
  m_neq(0),
  m_blockrow_size(0),
  m_blockcol_size(0),
  m_comm(Common::Comm::PE::instance().communicator())
{
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::create(CF::Common::Comm::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs)
{
  // get global ids vector
  int *gid=(int*)cp.gid()->pack();

  // blockmaps (colmap is gid 1 to 1, rowmap is gid with ghosts filtered out)
  int nmyglobalelements=0;
  int maxrowentries=0;
  std::vector<int> myglobalelements(0);
  std::vector<int> rowelements(0);

  for (int i=0; i<(const int)cp.isUpdatable().size(); i++)
    if (cp.isUpdatable()[i])
    {
      ++nmyglobalelements;
      myglobalelements.push_back((int)gid[i]);
      rowelements.push_back((int)(starting_indices[i+1]-starting_indices[i]));
      maxrowentries=maxrowentries<(starting_indices[i+1]-starting_indices[i])?(starting_indices[i+1]-starting_indices[i]):maxrowentries;
    }
  std::vector<double>dummy_entries(maxrowentries*neq*neq,0.);
  std::vector<int>global_columns(maxrowentries);

  Epetra_BlockMap rowmap(-1,nmyglobalelements,&myglobalelements[0],neq,0,m_comm);
  Epetra_BlockMap colmap(-1,cp.isUpdatable().size(),gid,neq,0,m_comm);
  myglobalelements.resize(0);
  myglobalelements.reserve(0);

  // matrix
  m_mat=Teuchos::rcp(new Epetra_FEVbrMatrix(Copy,rowmap,colmap,&rowelements[0]));
  rowmap.~Epetra_BlockMap();

  for(int i=0; i<(const int)nmyglobalelements; i++)
  {
    for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++) global_columns[j-starting_indices[i]]=gid[node_connectivity[j]];
    TRILINOS_THROW(m_mat->BeginInsertGlobalValues(gid[i],rowelements[i],&global_columns[0]));
    for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++)
      TRILINOS_THROW(m_mat->SubmitBlockEntry(&dummy_entries[0],0,3,3));
    TRILINOS_THROW(m_mat->EndSubmitEntries());
  }
  TRILINOS_THROW(m_mat->FillComplete());
  TRILINOS_THROW(m_mat->OptimizeStorage()); // in theory fillcomplete calls optimizestorage from Trilinos 8.x+
  delete gid;

  // set class properties
  m_is_created=true;
  m_neq=neq;
  m_blockrow_size=nmyglobalelements;
  m_blockcol_size=maxrowentries;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::destroy()
{
  if (m_is_created) m_mat.reset();
  m_neq=0;
  m_blockrow_size=0;
  m_blockcol_size=0;
  m_is_created=false;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::set_value(const Uint icol, const Uint irow, const Real value)
{
  cf_assert(m_is_created);
  const int colblock=(int)(icol/m_neq);
  const int colsub=(int)(icol%m_neq);
  const int rowblock=(int)(irow/m_neq);
  const int rowsub=(int)(irow%m_neq);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummyneq;
  TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(rowblock,dummyneq,blockrowsize,colindices,val));
  for (int i=0; i<(const int)blockrowsize; i++)
    if (*colindices++==colblock)
    {
      val[i][0](rowsub,colsub)=value;
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::add_value(const Uint icol, const Uint irow, const Real value)
{
  cf_assert(m_is_created);
  const int colblock=(int)(icol/m_neq);
  const int colsub=(int)(icol%m_neq);
  const int rowblock=(int)(irow/m_neq);
  const int rowsub=(int)(irow%m_neq);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummyneq;
  TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(rowblock,dummyneq,blockrowsize,colindices,val));
  for (int i=0; i<(const int)blockrowsize; i++)
    if (*colindices++==colblock)
    {
      val[i][0](rowsub,colsub)+=value;
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::get_value(const Uint icol, const Uint irow, Real& value)
{
  cf_assert(m_is_created);
  const int colblock=(int)(icol/m_neq);
  const int colsub=(int)(icol%m_neq);
  const int rowblock=(int)(irow/m_neq);
  const int rowsub=(int)(irow%m_neq);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummyneq;
  TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(rowblock,dummyneq,blockrowsize,colindices,val));
  for (int i=0; i<(const int)blockrowsize; i++)
    if (*colindices++==colblock)
    {
      value=val[i][0](rowsub,colsub);
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::solve(LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs)
{
  cf_assert(m_is_created);
  cf_assert(solution->is_created());
  cf_assert(rhs->is_created());

  /// @todo finish solve
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::set_values(const BlockAccumulator& values)
{
  cf_assert(m_is_created);
  const int numblocks=values.indices.size();
  int* idxs=(int*)&values.indices[0];
  for (int irow=0; irow<(const int)numblocks; irow++)
  {
    TRILINOS_ASSERT(m_mat->BeginReplaceMyValues(idxs[irow],numblocks,idxs));
    for (int icol=0; icol<numblocks; icol++)
      TRILINOS_ASSERT(m_mat->SubmitBlockEntry((double*)values.mat.data()+irow*m_neq+icol*m_neq*m_neq*numblocks,numblocks*m_neq,m_neq,m_neq));
    TRILINOS_ASSERT(m_mat->EndSubmitEntries());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::add_values(const BlockAccumulator& values)
{
/** @attention
 * issue1: its stupid to submitt each block separately, cant submit a single blockrow at once!!!
 * issue2: so it seems that on trilinos side there is another "blockaccumulator"
 * issue3: investigate performance if matrix is View mode, the copy due to issue2 could be circumvented (then epetra only stores pointers to values)
 * issue4: investigate prformance of extracting a blockrowview and fill manually, all blocks at once in the current blockrow
**/
  cf_assert(m_is_created);
  const int numblocks=values.indices.size();
  int* idxs=(int*)&values.indices[0];
  for (int irow=0; irow<(const int)numblocks; irow++)
  {
    TRILINOS_ASSERT(m_mat->BeginSumIntoMyValues(idxs[irow],numblocks,idxs));
    for (int icol=0; icol<numblocks; icol++)
      TRILINOS_ASSERT(m_mat->SubmitBlockEntry((double*)values.mat.data()+irow*m_neq+icol*m_neq*m_neq*numblocks,numblocks*m_neq,m_neq,m_neq));
    TRILINOS_ASSERT(m_mat->EndSubmitEntries());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::get_values(BlockAccumulator& values)
{
  /// @todo extracting values does not have a beginextract/submit/ensubmit scheme, needs to be implemented via extractrowview
/*
  cf_assert(m_is_created);
  const int numblocks=values.indices.size();
  int* idxs=(int*)&values.indices[0];
  for (int irow=0; irow<(const int)numblocks; irow++)
  {
    TRILINOS_ASSERT(m_mat->BeginSumIntoMyValues(idxs[irow],numblocks,idxs));
    for (int icol=0; icol<numblocks; icol++)
      TRILINOS_ASSERT(m_mat->SubmitBlockEntry((double*)values.mat.data()+irow*m_neq+icol*m_neq*m_neq*numblocks,numblocks*m_neq,m_neq,m_neq));
    TRILINOS_ASSERT(m_mat->EndSubmitEntries());
  }

int Epetra_VbrMatrix::ExtractGlobalBlockRowView 	( 	int 	BlockRow,
    int & 	RowDim,
    int & 	NumBlockEntries,
    int *& 	BlockIndices,
    Epetra_SerialDenseMatrix **& 	Values
  ) 	const
*/
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::set_row(const Uint iblockrow, const Uint ieq, Real diagval, Real offdiagval)
{
  cf_assert(m_is_created);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int diagonalblock=-1;
  int dummy_neq;
  TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView((int)iblockrow,dummy_neq,blockrowsize,colindices,val));
  for (int i=0; i<blockrowsize; i++)
  {
    if (colindices[i]==iblockrow) diagonalblock=i;
    for (int j=0; j<m_neq; j++)
      val[i][0](ieq,j)=offdiagval;
  }
  val[diagonalblock][0](ieq,ieq)=diagval;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::get_column_and_replace_to_zero(const Uint iblockcol, Uint ieq, std::vector<Real>& values)
{
  cf_assert(m_is_created);
  /// @todo implement
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::tie_blockrow_pairs (const Uint iblockrow_to, const Uint iblockrow_from)
{
  cf_assert(m_is_created);
  /// @todo implement
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::set_diagonal(const std::vector<Real>& diag)
{
  cf_assert(m_is_created);
  int *dummy_rowcoldim;
  double *entryvals;
  int numblocks,dummy_stride;
  const unsigned long stride=m_neq+1;
  const unsigned long blockadvance=m_neq*m_neq;
  TRILINOS_ASSERT(m_mat->BeginExtractBlockDiagonalView(numblocks,dummy_rowcoldim));
  cf_assert(diag.size()==numblocks*m_neq);
  double *diagvals=(double*)&diag[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    TRILINOS_ASSERT(m_mat->ExtractBlockDiagonalEntryView(entryvals,dummy_stride));
    for (; entryvals<(const double*)(entryvals+blockadvance); entryvals+=stride)
      *entryvals=*diagvals++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::add_diagonal(const std::vector<Real>& diag)
{
  cf_assert(m_is_created);
  int *dummy_rowcoldim;
  double *entryvals;
  int numblocks,dummy_stride;
  const unsigned long stride=m_neq+1;
  const unsigned long blockadvance=m_neq*m_neq;
  TRILINOS_ASSERT(m_mat->BeginExtractBlockDiagonalView(numblocks,dummy_rowcoldim));
  cf_assert(diag.size()==numblocks*m_neq);
  double *diagvals=(double*)&diag[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    TRILINOS_ASSERT(m_mat->ExtractBlockDiagonalEntryView(entryvals,dummy_stride));
    for (; entryvals<(const double*)(entryvals+blockadvance); entryvals+=stride)
      *entryvals+=*diagvals++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::get_diagonal(std::vector<Real>& diag)
{
  cf_assert(m_is_created);
  int *dummy_rowcoldim;
  double *entryvals;
  int numblocks,dummy_stride;
  const unsigned long stride=m_neq+1;
  const unsigned long blockadvance=m_neq*m_neq;
  TRILINOS_ASSERT(m_mat->BeginExtractBlockDiagonalView(numblocks,dummy_rowcoldim));
  diag.resize(numblocks*m_neq);
  double *diagvals=&diag[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    TRILINOS_ASSERT(m_mat->ExtractBlockDiagonalEntryView(entryvals,dummy_stride));
    for (; entryvals<(const double*)(entryvals+blockadvance); entryvals+=stride)
      *diagvals++=*entryvals;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::reset(Real reset_to)
{
  cf_assert(m_is_created);
  m_mat->PutScalar(reset_to);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::print(Common::LogStream& stream)
{
  if (m_is_created)
  {
    int sumentries=0;
    int maxcol=0;
    double *vals=new double[m_blockcol_size];
    int *idxs=new int[m_blockcol_size];
    for (int i=0; i<(const int)m_mat->NumMyRows(); i++)
    {
      int rowentries;
      m_mat->NumMyRowEntries(i,rowentries);
      TRILINOS_ASSERT(m_mat->ExtractMyRowCopy(i,rowentries,rowentries,vals,idxs));
      sumentries+=rowentries;
      for (int j=0; j<rowentries; j++)
        stream << idxs[j] << " " << -i << " " << vals[j] << "\n";
    }
    delete[] vals;
    delete[] idxs;
    stream << "# name:                 " << name() << "\n";
    stream << "# type_name:            " << type_name() << "\n";
    stream << "# number of equations:  " << m_neq << "\n";
    stream << "# number of rows:       " << m_blockrow_size*m_neq << "\n";
    stream << "# number of cols:       " << m_blockcol_size*m_neq << "\n";
    stream << "# number of block rows: " << m_blockrow_size << "\n";
    stream << "# number of block cols: " << m_blockcol_size << "\n";
    stream << "# number of entries:    " << sumentries << "\n";
  } else {
    stream << name() << " of type " << type_name() << "::is_created() is false, nothing is printed.";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::print(std::ostream& stream)
{
  if (m_is_created)
  {
    int sumentries=0;
    int maxcol=0;
    double *vals=new double[m_blockcol_size];
    int *idxs=new int[m_blockcol_size];
    for (int i=0; i<(const int)m_mat->NumMyRows(); i++)
    {
      int rowentries;
      m_mat->NumMyRowEntries(i,rowentries);
      TRILINOS_ASSERT(m_mat->ExtractMyRowCopy(i,rowentries,rowentries,vals,idxs));
      sumentries+=rowentries;
      for (int j=0; j<rowentries; j++)
        stream << idxs[j] << " " << -i << " " << vals[j] << "\n" << flush;
    }
    delete[] vals;
    delete[] idxs;
    stream << "# name:                 " << name() << "\n";
    stream << "# type_name:            " << type_name() << "\n";
    stream << "# number of equations:  " << m_neq << "\n";
    stream << "# number of rows:       " << m_blockrow_size*m_neq << "\n";
    stream << "# number of cols:       " << m_blockcol_size*m_neq << "\n";
    stream << "# number of block rows: " << m_blockrow_size << "\n";
    stream << "# number of block cols: " << m_blockcol_size << "\n";
    stream << "# number of entries:    " << sumentries << "\n";
  } else {
    stream << name() << " of type " << type_name() << "::is_created() is false, nothing is printed.";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::print(const std::string& filename)
{
  std::ofstream stream(filename.c_str());
  print(stream);
  stream.close();
}

////////////////////////////////////////////////////////////////////////////////////////////

