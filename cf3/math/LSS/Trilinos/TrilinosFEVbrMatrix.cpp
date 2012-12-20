// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <boost/pointer_cast.hpp>

#include "Stratimikos_DefaultLinearSolverBuilder.hpp"
#include "Thyra_LinearOpWithSolveFactoryHelpers.hpp"
#include "Thyra_EpetraThyraWrappers.hpp"
#include "Thyra_EpetraLinearOp.hpp"
#include "Epetra_MsrMatrix.h"
#include "Epetra_Vector.h"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_CommandLineProcessor.hpp"

#include "common/Assertions.hpp"
#include "common/Builder.hpp"
#include "common/PE/Comm.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"
#include "math/LSS/Trilinos/TrilinosFEVbrMatrix.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosFEVbrMatrix.cpp implementation of LSS::TrilinosFEVbrMatrix
  @author Tamas Banyai

  It is based on Trilinos's FEVbrMatrix.
**/

////////////////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::math;
using namespace cf3::math::LSS;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LSS::TrilinosFEVbrMatrix, LSS::Matrix, LSS::LibLSS > TrilinosFEVbrMatrix_Builder;

TrilinosFEVbrMatrix::TrilinosFEVbrMatrix(const std::string& name) :
  LSS::Matrix(name),
  m_mat(0),
  m_is_created(false),
  m_neq(0),
  m_blockrow_size(0),
  m_blockcol_size(0),
  m_p2m(0),
  m_converted_indices(0),
  m_comm(common::PE::Comm::instance().communicator())
{
  properties().add("vector_type", std::string("cf3.math.LSS.TrilinosVector"));
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::create(cf3::common::PE::CommPattern& cp, const Uint neq, const std::vector<Uint>& node_connectivity, const std::vector<Uint>& starting_indices, LSS::Vector& solution, LSS::Vector& rhs)
{
  /// @todo structurally symmetricize the matrix
  /// @todo ensure main diagonal blocks always existent

  // if already created
  if (m_is_created) destroy();

  // Copy node connectivity
  m_node_connectivity.resize(node_connectivity.size());
  m_starting_indices.resize(starting_indices.size());
  std::copy(node_connectivity.begin(), node_connectivity.end(), m_node_connectivity.begin());
  std::copy(starting_indices.begin(), starting_indices.end(), m_starting_indices.begin());

  // Symmetric matrix
  const int nb_nodes = cp.isUpdatable().size();
  m_keep_node.assign(m_node_connectivity.size(), true);
//   for(int this_node = 0; this_node != nb_nodes; ++this_node)
//   {
//     const int this_node_begin = m_starting_indices[this_node];
//     const int this_node_end = m_starting_indices[this_node+1];
//     for(int i = this_node_begin; i != this_node_end; ++i)
//     {
//       const int other_node = m_node_connectivity[i];
//       if(other_node == this_node || !m_keep_node[i])
//         continue;
//       const int other_node_begin = m_starting_indices[other_node];
//       const int other_node_end = m_starting_indices[other_node+1];
//       for(int j = other_node_begin; j != other_node_end; ++j)
//       {
//         if(m_node_connectivity[j] == this_node)
//           m_keep_node[j] = false;
//       }
//     }
//   }

  // get global ids vector
  int *gid=(int*)cp.gid()->pack();

  // prepare intermediate data
  int nmyglobalelements=0;
  int maxrowentries=0;
  std::vector<int> rowelements(0);
  std::vector<int> myglobalelements(0);

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

  // blockmaps (colmap is gid 1 to 1, rowmap is gid with ghosts filtered out)
  Epetra_BlockMap rowmap(-1,nmyglobalelements,&myglobalelements[0],neq,0,m_comm);
  for (int i=0; i<(const int)cp.isUpdatable().size(); i++)
    if (!cp.isUpdatable()[i])
      myglobalelements.push_back((int)gid[i]);
  Epetra_BlockMap colmap(-1,cp.isUpdatable().size(),&myglobalelements[0],neq,0,m_comm);
  myglobalelements.clear();

  // create matrix
  m_mat=Teuchos::rcp(new Epetra_FEVbrMatrix(Copy,rowmap,colmap,&rowelements[0]));
/*must be a bug in Trilinos, Epetra_FEVbrMatrix constructor is in Copy mode but it hangs up anyway
  more funny, when it gets out of scope and gets dealloc'd, everything survives according to memcheck
  rowmap.~Epetra_BlockMap();
  colmap.~Epetra_BlockMap();
*/
  rowelements.clear();

  // prepare the entries
  for (int i=0; i<(const int)cp.isUpdatable().size(); i++)
    if (cp.isUpdatable()[i])
    {
      int nb_added = 0;
      for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++)
      {
        if(m_keep_node[j])
        {
          global_columns[nb_added++]=myglobalelements[m_p2m[node_connectivity[j]]];
        }
      }
      TRILINOS_THROW(m_mat->BeginInsertGlobalValues(gid[i],nb_added,&global_columns[0]));
      for(int j=0; j<(const int)nb_added; j++)
        TRILINOS_THROW(m_mat->SubmitBlockEntry(&dummy_entries[0],0,neq,neq));
      TRILINOS_THROW(m_mat->EndSubmitEntries());
    }
  TRILINOS_THROW(m_mat->FillComplete());
  //TRILINOS_THROW(m_mat->OptimizeStorage()); // in theory fillcomplete calls optimizestorage from Trilinos 8.x+
  delete[] gid;

  // set class properties
  m_is_created=true;
  m_neq=neq;
  m_blockrow_size=nmyglobalelements;
  m_blockcol_size=cp.gid()->size();
  CFdebug << "Created a " << m_mat->NumGlobalCols() << " x " << m_mat->NumGlobalRows() << " trilinos matrix with " << m_mat->NumGlobalNonzeros() << " non-zero elements." << CFendl;
}

void TrilinosFEVbrMatrix::create_blocked(common::PE::CommPattern& cp, const VariablesDescriptor& vars, const std::vector< Uint >& node_connectivity, const std::vector< Uint >& starting_indices, Vector& solution, Vector& rhs)
{
  throw common::NotImplemented(FromHere(), "create_blocked is not implemented for TrilinosFEVbrMatrix");
}


////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::destroy()
{
  if (m_is_created) m_mat.reset();
  m_p2m.resize(0);
  m_p2m.reserve(0);
  m_neq=0;
  m_blockrow_size=0;
  m_blockcol_size=0;
  m_is_created=false;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::set_value(const Uint icol, const Uint irow, const Real value)
{
  cf3_assert(m_is_created);
  const int colblock=(int)m_p2m[icol/m_neq];
  const int colsub=(int)(icol%m_neq);
  const int rowblock=(int)m_p2m[irow/m_neq];
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
      return;
    }
  throw common::BadValue(FromHere(),"Trying to access an illegal entry.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::add_value(const Uint icol, const Uint irow, const Real value)
{
  cf3_assert(m_is_created);
  const int colblock=(int)m_p2m[icol/m_neq];
  const int colsub=(int)(icol%m_neq);
  const int rowblock=(int)m_p2m[irow/m_neq];
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
      return;
    }
  throw common::BadValue(FromHere(),"Trying to access an illegal entry.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::get_value(const Uint icol, const Uint irow, Real& value)
{
  cf3_assert(m_is_created);
  const int colblock=(int)m_p2m[icol/m_neq];
  const int colsub=(int)(icol%m_neq);
  const int rowblock=(int)m_p2m[irow/m_neq];
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
      return;
    }
  throw common::BadValue(FromHere(),"Trying to access an illegal entry.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::set_values(const BlockAccumulator& values)
{
  cf3_assert(m_is_created);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummyneq;
  int hits=0;
  const int numblocks=values.indices.size();
  const int rowoffset=(numblocks-1)*m_neq;
  const int neqneq=m_neq*m_neq;
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  for (int i=0; i<(const int)numblocks; i++) m_converted_indices[i]=m_p2m[values.indices[i]];
  int* idxs=(int*)&m_converted_indices[0];
  for (int irow=0; irow<(const int)numblocks; irow++)
  {
    if (idxs[irow]<m_blockrow_size)
    {
      hits=0;
      TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(idxs[irow],dummyneq,blockrowsize,colindices,val));
      for (int j=0; j<(const int)blockrowsize; j++)
      {
        for (int icol=0; icol<(const int)numblocks; icol++)
          if (colindices[j]==idxs[icol])
          {
            double *emv=val[j][0].A();
            int col_idx = icol*m_neq;
            for (double* l=emv; emv<(const double*)(l+neqneq); ++col_idx)
            {
              int row_idx = irow*m_neq;
              for (double* m=emv; emv<(const double*)(m+m_neq);)
                *emv++ = values.mat(row_idx++, col_idx);
            }

            hits++;
            //break; // this is arguable, can be that the blockaccumulator fills to the same id more than once (repetitive same id entries in values.indices)
          }
        if (hits==numblocks) break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::add_values(const BlockAccumulator& values)
{
/* TRILINOS-ADVICED
  cf3_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  for (int i=0; i<(const int)numblocks; i++) m_converted_indices[i]=m_p2m[values.indices[i]];
  int* idxs=(int*)&m_converted_indices[0];
  for (int irow=0; irow<(const int)numblocks; irow++)
    if (idxs[irow]<m_blockrow_size)
    {
      TRILINOS_ASSERT(m_mat->BeginSumIntoMyValues(idxs[irow],numblocks,idxs));
      for (int icol=0; icol<numblocks; icol++)
        TRILINOS_ASSERT(m_mat->SubmitBlockEntry((double*)values.mat.data()+irow*m_neq+icol*m_neq*m_neq*numblocks,numblocks*m_neq,m_neq,m_neq));
      TRILINOS_ASSERT(m_mat->EndSubmitEntries());
    }
*/
/* MANUAL FILL
  cf3_assert(m_is_created);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummyneq;
  int hits=0;
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  for (int i=0; i<(const int)numblocks; i++) m_converted_indices[i]=m_p2m[values.indices[i]];
  int* idxs=(int*)&m_converted_indices[0];
  for (int irow=0; irow<(const int)numblocks; irow++)
  {
    if (idxs[irow]<m_blockrow_size)
    {
      TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(idxs[irow],dummyneq,blockrowsize,colindices,val));
      for (int j=0; j<(const int)blockrowsize; j++)
        for (int k=0; k<(const int)numblocks; k++)
          if (colindices[j]==idxs[k])
            for (int l=0; l<(const int)m_neq; l++)
              for (int m=0; m<(const int)m_neq; m++)
                val[j][0](l,m)+=values.mat(irow*m_neq+l,k*m_neq+m);
      hits++;
      if (hits==numblocks) break;
    }
  }
*/
/* FINAL OPTIMIZED */
  cf3_assert(m_is_created);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummyneq;
  int hits=0;
  const int numblocks=values.indices.size();
  const int rowoffset=(numblocks-1)*m_neq;
  const int neqneq=m_neq*m_neq;
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  for (int i=0; i<(const int)numblocks; i++) m_converted_indices[i]=m_p2m[values.indices[i]];
  int* idxs=(int*)&m_converted_indices[0];
  for (int irow=0; irow<(const int)numblocks; irow++)
  {
    if (idxs[irow]<m_blockrow_size)
    {
      hits=0;
      TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(idxs[irow],dummyneq,blockrowsize,colindices,val));
      for (int j=0; j<(const int)blockrowsize; j++)
      {
        for (int icol=0; icol<(const int)numblocks; icol++)
          if (colindices[j]==idxs[icol])
          {
            double *emv=val[j][0].A();
            int col_idx = icol*m_neq;
            for (double* l=emv; emv<(const double*)(l+neqneq); ++col_idx)
            {
              int row_idx = irow*m_neq;
              for (double* m=emv; emv<(const double*)(m+m_neq);)
                *emv++ += values.mat(row_idx++, col_idx);
            }

            hits++;
            //break; // this is arguable, can be that the blockaccumulator fills to the same id more than once (repetitive same id entries in values.indices)
          }
        if (hits==numblocks) break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::get_values(BlockAccumulator& values)
{
  cf3_assert(m_is_created);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummyneq;
  int hits=0;
  const int numblocks=values.indices.size();
  const int rowoffset=(numblocks-1)*m_neq;
  const int neqneq=m_neq*m_neq;
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  for (int i=0; i<(const int)numblocks; i++) m_converted_indices[i]=m_p2m[values.indices[i]];
  int* idxs=(int*)&m_converted_indices[0];
  values.mat.setConstant(0.);
  for (int irow=0; irow<(const int)numblocks; irow++)
  {
    if (idxs[irow]<m_blockrow_size)
    {
      hits=0;
      TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(idxs[irow],dummyneq,blockrowsize,colindices,val));
      for (int j=0; j<(const int)blockrowsize; j++)
      {
        for (int icol=0; icol<(const int)numblocks; icol++)
          if (colindices[j]==idxs[icol])
          {
            double *emv=val[j][0].A();
            int col_idx = icol*m_neq;
            for (double* l=emv; emv<(const double*)(l+neqneq); ++col_idx)
            {
              int row_idx = irow*m_neq;
              for (double* m=emv; emv<(const double*)(m+m_neq);)
                values.mat(row_idx++, col_idx) = *emv++;
            }

            hits++;
            //break; // this is arguable, can be that the blockaccumulator fills to the same id more than once (repetitive same id entries in values.indices)
          }
        if (hits==numblocks) break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::set_row(const Uint iblockrow, const Uint ieq, Real diagval, Real offdiagval)
{
/* TRILINOS ADVICED
  cf3_assert(m_is_created);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int diagonalblock=-1;
  int dummy_neq;
  const int br=m_p2m[iblockrow];
  if (br<m_blockrow_size)
  {
    TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(br,dummy_neq,blockrowsize,colindices,val));
    for (int i=0; i<blockrowsize; i++)
    {
      if (colindices[i]==br) diagonalblock=i;
      for (int j=0; j<m_neq; j++)
        val[i][0](ieq,j)=offdiagval;
    }
    val[diagonalblock][0](ieq,ieq)=diagval;
  }
*/
/* OPTIMIZED */
  cf3_assert(m_is_created);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int diagonalblock=-1;
  int dummy_neq;
  const int br=m_p2m[iblockrow];
  const int neqneq=m_neq*m_neq;
  if (br<m_blockrow_size)
  {
    TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(br,dummy_neq,blockrowsize,colindices,val));
    for (int i=0; i<(const int)blockrowsize; i++)
    {
      if (*colindices++==br) diagonalblock=i;
      double *emv=&val[i][0](ieq,0);
      for (double* j=emv; emv<(const double*)(j+neqneq); emv+=m_neq)
        *emv = offdiagval;
    }
    val[diagonalblock][0](ieq,ieq)=diagval;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::get_column_and_replace_to_zero(const Uint iblockcol, Uint ieq, std::vector<Real>& values)
{
  /// @note this could be made faster if structural symmetry is ensured during create, because then involved rows could be determined by indices in the ibloccol-th row
  /// @attention COMPUTATIONALLY VERY EXPENSIVE!
  cf3_assert(m_is_created);
  values.resize(m_blockcol_size*m_neq);
  values.assign(m_blockcol_size*m_neq,0.);
  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummy_neq;
  for (int k=0; k<(const int)m_blockcol_size; k++)
    if (m_p2m[k]<m_blockrow_size)
    {
      TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(m_p2m[k],dummy_neq,blockrowsize,colindices,val));
      for (int i=0; i<blockrowsize; i++)
        if (colindices[i]==m_p2m[iblockcol])
        {
          for (int j=0; j<m_neq; j++)
          {
            values[k*m_neq+j]=val[i][0](j,ieq);
            val[i][0](j,ieq)=0.;
          }
          break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::symmetric_dirichlet(const Uint blockrow, const Uint ieq, const Real value, Vector& rhs)
{
  const int columns_begin = m_starting_indices[blockrow];
  const int columns_end = m_starting_indices[blockrow+1];

  Epetra_SerialDenseMatrix **val;
  int* colindices;
  int blockrowsize;
  int dummy_neq;

  const Uint bc_col = m_p2m[blockrow];

  for(int col_idx = columns_begin; col_idx != columns_end; ++col_idx)
  {
    const int col = m_node_connectivity[col_idx];
    const Uint other_row = m_p2m[col];
    if(other_row >= m_blockrow_size)
      continue;

    const int other_cols_begin = m_starting_indices[col];
    const int other_cols_end = m_starting_indices[col+1];
    bool row_has_node = false;
    for(int i = other_cols_begin; i != other_cols_end; ++i)
    {
      if(m_node_connectivity[i] == blockrow && m_keep_node[i])
      {
        row_has_node = true;
        break;
      }
    }

    if(!row_has_node)
      continue;

    TRILINOS_THROW(m_mat->ExtractMyBlockRowView(other_row,dummy_neq,blockrowsize,colindices,val));
    const int nb_entries_const = blockrowsize;
    Uint i;

    for(i = 0; i != nb_entries_const; )
    {
      if(colindices[i] == bc_col)
      {
        for(int j = 0; j != m_neq; ++j)
        {
          rhs.add_value(col, j, -val[i][0](j, ieq) * value);
          val[i][0](j, ieq) = 0;
        }
        break;
      }
      ++i;
    }

    cf3_assert(i != nb_entries_const);

    if(other_row == bc_col)
    {
      for(i = 0; i != nb_entries_const; ++i)
      {
        for(int j = 0; j != m_neq; ++j)
        {
          val[i][0](ieq, j) = 0;
        }
        if(colindices[i] == bc_col)
        {
          val[i][0](ieq, ieq) = 1.;
        }
      }
    }
  }

  rhs.set_value(blockrow, ieq, value);
}


////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::tie_blockrow_pairs (const Uint iblockrow_to, const Uint iblockrow_from)
{
  cf3_assert(m_is_created);
  Epetra_SerialDenseMatrix **val_to,**val_from;
  int* colindices_to, *colindices_from;
  int blockrowsize_to,blockrowsize_from;
  int diag=-1,pair=-1;
  int dummy_neq;
  const int br_to=m_p2m[iblockrow_to];
  const int br_from=m_p2m[iblockrow_from];
  cf3_assert(!(((br_to>=m_blockrow_size)&&(br_from<m_blockrow_size))||((br_to<m_blockrow_size)&&(br_from>=m_blockrow_size))));
  if ((br_to<m_blockrow_size)&&(br_from<m_blockrow_size))
  {
    TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(br_from,dummy_neq,blockrowsize_from,colindices_from,val_from));
    TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(br_to,  dummy_neq,blockrowsize_to,  colindices_to  ,val_to));
    if (blockrowsize_to!=blockrowsize_from) throw common::BadValue(FromHere(),"Number of blocks do not match for the two block rows to be tied together.");
    for (int i=0; i<blockrowsize_to; i++)
    {
      cf3_assert(colindices_to[i]==colindices_from[i]);
      if (colindices_from[i]==br_from) diag=i;
      if (colindices_to[i]  ==br_to)   pair=i;
      val_to[i][0]+=val_from[i][0];
      val_from[i][0].Scale(0.);
    }
    for (int i=0; i<m_neq; i++)
    {
      val_from[diag][0](i,i)=1.;
      val_from[pair][0](i,i)=-1.;
   }
    val_to[pair][0]+=val_to[diag][0];
    val_to[diag][0].Scale(0.);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::set_diagonal(const std::vector<Real>& diag)
{
  cf3_assert(m_is_created);
  int *dummy_rowcoldim;
  double *entryvals;
  int numblocks,dummy_stride;
  const unsigned long stride=m_neq+1;
  const unsigned long blockadvance=m_neq*m_neq;
  TRILINOS_ASSERT(m_mat->BeginExtractBlockDiagonalView(numblocks,dummy_rowcoldim));
  cf3_assert(diag.size()==m_blockcol_size*m_neq);
  for (int i=0; i<(const int)m_blockcol_size; i++)
    if (m_p2m[i]<m_blockrow_size)
  {
    double *diagvals=(double*)&diag[i*m_neq];
    TRILINOS_ASSERT(m_mat->ExtractBlockDiagonalEntryView(entryvals,dummy_stride));
    for (double* ev=entryvals; ev<(const double*)(&entryvals[blockadvance]); ev+=stride)
      *ev=*diagvals++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::add_diagonal(const std::vector<Real>& diag)
{
  cf3_assert(m_is_created);
  int *dummy_rowcoldim;
  double *entryvals;
  int numblocks,dummy_stride;
  const unsigned long stride=m_neq+1;
  const unsigned long blockadvance=m_neq*m_neq;
  TRILINOS_ASSERT(m_mat->BeginExtractBlockDiagonalView(numblocks,dummy_rowcoldim));
  cf3_assert(diag.size()==m_blockcol_size*m_neq);
  for (int i=0; i<(const int)m_blockcol_size; i++)
    if (m_p2m[i]<m_blockrow_size)
  {
    double *diagvals=(double*)&diag[i*m_neq];
    TRILINOS_ASSERT(m_mat->ExtractBlockDiagonalEntryView(entryvals,dummy_stride));
    for (double* ev=entryvals; ev<(const double*)(&entryvals[blockadvance]); ev+=stride)
      *ev+=*diagvals++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::get_diagonal(std::vector<Real>& diag)
{
  cf3_assert(m_is_created);
  int *dummy_rowcoldim;
  double *entryvals;
  int numblocks,dummy_stride;
  const unsigned long stride=m_neq+1;
  const unsigned long blockadvance=m_neq*m_neq;
  TRILINOS_ASSERT(m_mat->BeginExtractBlockDiagonalView(numblocks,dummy_rowcoldim));
  diag.clear();
  diag.resize(m_blockcol_size*m_neq,0.);
  for (int i=0; i<(const int)m_blockcol_size; i++)
    if (m_p2m[i]<m_blockrow_size)
  {
    double *diagvals=(double*)&diag[i*m_neq];
    TRILINOS_ASSERT(m_mat->ExtractBlockDiagonalEntryView(entryvals,dummy_stride));
    for (double* ev=entryvals; ev<(const double*)(&entryvals[blockadvance]); ev+=stride)
      *diagvals++=*ev;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::reset(Real reset_to)
{
  cf3_assert(m_is_created);
  m_mat->PutScalar(reset_to);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::print(common::LogStream& stream)
{
  if (m_is_created)
  {
    int sumentries=0;
    int rowentries,dummy_neq;
    int *idxs;
    Epetra_SerialDenseMatrix** vals;
    std::vector<int> m2p(m_blockcol_size,-1);
    for (int i=0; i<(const int)m_p2m.size(); i++) m2p[m_p2m[i]]=i;
    for (int i=0; i<(const int)m_mat->NumMyBlockRows(); i++)
    {
      TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(i,dummy_neq,rowentries,idxs,vals));
      for (int j=0; j<(const int)m_neq; j++)
        for (int k=0; k<(const int)rowentries; k++)
          for (int l=0; l<(const int)m_neq; l++)
            stream << m2p[idxs[k]]*m_neq+l << " " << -(int)(m2p[i]*m_neq+j) << " " << vals[k][0](j,l) << "\n";
      sumentries+=rowentries*m_neq*m_neq;
    }
    stream << "# name:                 " << name() << "\n";
    stream << "# type_name:            " << type_name() << "\n";
    stream << "# process:              " << m_comm.MyPID() << "\n";
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

void TrilinosFEVbrMatrix::print(std::ostream& stream)
{
  if (m_is_created)
  {
    int sumentries=0;
    int rowentries,dummy_neq;
    int *idxs;
    Epetra_SerialDenseMatrix** vals;
    std::vector<int> m2p(m_blockcol_size,-1);
    for (int i=0; i<(const int)m_p2m.size(); i++) m2p[m_p2m[i]]=i;
    for (int i=0; i<(const int)m_mat->NumMyBlockRows(); i++)
    {
      TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(i,dummy_neq,rowentries,idxs,vals));
      for (int j=0; j<(const int)m_neq; j++)
        for (int k=0; k<(const int)rowentries; k++)
          for (int l=0; l<(const int)m_neq; l++)
            stream << m2p[idxs[k]]*m_neq+l << " " << -(int)(m2p[i]*m_neq+j) << " " << vals[k][0](j,l) << "\n" << std::flush;
      sumentries+=rowentries*m_neq*m_neq;
    }
    stream << "# name:                 " << name() << "\n";
    stream << "# type_name:            " << type_name() << "\n";
    stream << "# process:              " << m_comm.MyPID() << "\n";
    stream << "# number of equations:  " << m_neq << "\n";
    stream << "# number of rows:       " << m_blockrow_size*m_neq << "\n";
    stream << "# number of cols:       " << m_blockcol_size*m_neq << "\n";
    stream << "# number of block rows: " << m_blockrow_size << "\n";
    stream << "# number of block cols: " << m_blockcol_size << "\n";
    stream << "# number of entries:    " << sumentries << "\n" << std::flush;
  } else {
    stream << name() << " of type " << type_name() << "::is_created() is false, nothing is printed.";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::print(const std::string& filename, std::ios_base::openmode mode )
{
  std::ofstream stream(filename.c_str(),mode);
  stream << "VARIABLES=COL,ROW,VAL\n" << std::flush;
  stream << "ZONE T=\"" << type_name() << "::" << name() <<  "\"\n" << std::flush;
  print(stream);
  stream.close();
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::print_native(ostream& stream)
{
  m_mat->Print(stream);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosFEVbrMatrix::debug_data(std::vector<Uint>& row_indices, std::vector<Uint>& col_indices, std::vector<Real>& values)
{
  cf3_assert(m_is_created);
  row_indices.clear();
  col_indices.clear();
  values.clear();
  int rowentries,dummy_neq;
  int *idxs;
  Epetra_SerialDenseMatrix** vals;
  std::vector<int> m2p(m_blockcol_size,-1);
  for (int i=0; i<(const int)m_p2m.size(); i++) m2p[m_p2m[i]]=i;
  for (int i=0; i<(const int)m_mat->NumMyBlockRows(); i++)
  {
    TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(i,dummy_neq,rowentries,idxs,vals));
    for (int j=0; j<(const int)m_neq; j++)
      for (int k=0; k<(const int)rowentries; k++)
        for (int l=0; l<(const int)m_neq; l++)
        {
          row_indices.push_back((m2p[i]*m_neq+j));
          col_indices.push_back(m2p[idxs[k]]*m_neq+l);
          values.push_back(vals[k][0](j,l));
        }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

Teuchos::RCP< const Thyra::LinearOpBase< Real > > TrilinosFEVbrMatrix::thyra_operator() const
{
  return Thyra::epetraLinearOp(m_mat);
}

////////////////////////////////////////////////////////////////////////////////////////////

Teuchos::RCP< Thyra::LinearOpBase< Real > > TrilinosFEVbrMatrix::thyra_operator()
{
  return Thyra::nonconstEpetraLinearOp(m_mat);
}

////////////////////////////////////////////////////////////////////////////////////////////
