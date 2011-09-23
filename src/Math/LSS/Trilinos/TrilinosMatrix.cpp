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

#include "Common/Assertions.hpp"
#include "Common/PE/Comm.hpp"
#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
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
  m_p2m(0),
  m_converted_indices(0),
  m_comm(Common::PE::Comm::instance().communicator())
{
  options().add_option< CF::Common::OptionT<std::string> >( "settings_file" , "trilinos_settings.xml" );
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::create(CF::Common::PE::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs)
{
  /// @todo structurally symmetricize the matrix
  /// @todo ensure main diagonal blocks always existent

  // if already created
  if (m_is_created) destroy();

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
//      for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++) global_columns[j-starting_indices[i]]=gid[m_p2m[node_connectivity[j]]];
      for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++) global_columns[j-starting_indices[i]]=myglobalelements[m_p2m[node_connectivity[j]]];
      TRILINOS_THROW(m_mat->BeginInsertGlobalValues(gid[i],(int)(starting_indices[i+1]-starting_indices[i]),&global_columns[0]));
      for(int j=(const int)starting_indices[i]; j<(const int)starting_indices[i+1]; j++)
        TRILINOS_THROW(m_mat->SubmitBlockEntry(&dummy_entries[0],0,neq,neq));
      TRILINOS_THROW(m_mat->EndSubmitEntries());
    }
  TRILINOS_THROW(m_mat->FillComplete());
  TRILINOS_THROW(m_mat->OptimizeStorage()); // in theory fillcomplete calls optimizestorage from Trilinos 8.x+
  delete[] gid;

  // set class properties
  m_is_created=true;
  m_neq=neq;
  m_blockrow_size=nmyglobalelements;
  m_blockcol_size=cp.gid()->size();
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::destroy()
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

void TrilinosMatrix::set_value(const Uint icol, const Uint irow, const Real value)
{
  cf_assert(m_is_created);
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
  throw Common::BadValue(FromHere(),"Trying to access an illegal entry.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::add_value(const Uint icol, const Uint irow, const Real value)
{
  cf_assert(m_is_created);
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
  throw Common::BadValue(FromHere(),"Trying to access an illegal entry.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::get_value(const Uint icol, const Uint irow, Real& value)
{
  cf_assert(m_is_created);
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
  throw Common::BadValue(FromHere(),"Trying to access an illegal entry.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::solve(LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs)
{
  cf_assert(m_is_created);
  cf_assert(solution->is_created());
  cf_assert(rhs->is_created());

  // general setup phase
  Stratimikos::DefaultLinearSolverBuilder linearSolverBuilder(options().option("settings_file").value_str());
  /// @todo decouple from fancyostream to ostream or to C stdout when possible
  Teuchos::RCP<Teuchos::FancyOStream> out = Teuchos::VerboseObjectBase::getDefaultOStream();
  Teuchos::CommandLineProcessor  clp(false); // false: don't throw exceptions
  linearSolverBuilder.setupCLP(&clp); // not used, TODO: see if can be removed safely since not really used
  /// @todo check whgats wrtong with input options via string
  //clp.setOption( "tol",            &tol,            "Tolerance to check against the scaled residual norm." ); // input options via string, not working for some reason
  int argc=0; char** argv=0; Teuchos::CommandLineProcessor::EParseCommandLineReturn parse_return = clp.parse(argc,argv);
  if( parse_return != Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL ) throw Common::ParsingFailed(FromHere(),"Emulated command line parsing for stratimikos failed");

  // wrapping epetra to thyra
  Teuchos::RCP<const Thyra::LinearOpBase<double> > A = Thyra::epetraLinearOp( m_mat );
  boost::shared_ptr<LSS::TrilinosVector> tsol = boost::dynamic_pointer_cast<LSS::TrilinosVector>(solution);
  Teuchos::RCP<Thyra::VectorBase<double> >         x = Thyra::create_Vector( tsol->epetra_vector() , A->domain() );
  boost::shared_ptr<LSS::TrilinosVector> trhs = boost::dynamic_pointer_cast<LSS::TrilinosVector>(rhs);
  Teuchos::RCP<const Thyra::VectorBase<double> >   b = Thyra::create_Vector( trhs->epetra_vector() , A->range() );

  // r = b - A*x, initial L2 norm
  double norm2_in=0.;
  {
    Epetra_Vector epetra_r(*trhs->epetra_vector());
    Epetra_Vector m_mat_x(m_mat->OperatorRangeMap());
    m_mat->Apply(*tsol->epetra_vector(),m_mat_x);
    epetra_r.Update(-1.0,m_mat_x,1.0);
    epetra_r.Norm2(&norm2_in);
  }

  // Reading in the solver parameters from the parameters file and/or from
  // the command line.  This was setup by the command-line options
  // set by the setupCLP(...) function above.
  linearSolverBuilder.readParameters(0); // out.get() if want confirmation about the xml file within trilinos
  Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<double> > lowsFactory = linearSolverBuilder.createLinearSolveStrategy(""); // create linear solver strategy
/// @todo verbosity level from option
  lowsFactory->setVerbLevel((Teuchos::EVerbosityLevel)4); // set verbosity

  // print back default and current settings
  if (false) {
    std::ofstream ofs("./trilinos_default.txt");
    linearSolverBuilder.getValidParameters()->print(ofs,Teuchos::ParameterList::PrintOptions().indent(2).showTypes(true).showDoc(true)); // the last true-false is the deciding about whether printing documentation to option or not
    ofs.flush();ofs.close();
    ofs.open("./trilinos_default.xml");
    Teuchos::writeParameterListToXmlOStream(*linearSolverBuilder.getValidParameters(),ofs);
    ofs.flush();ofs.close();
  }
  if (false) {
    linearSolverBuilder.writeParamsFile(*lowsFactory,"./trilinos_current.xml");
  }

  // solve the matrix
  Teuchos::RCP<Thyra::LinearOpWithSolveBase<double> > lows = Thyra::linearOpWithSolve(*lowsFactory, A);
  Thyra::solve(*lows, Thyra::NOTRANS, *b, &*x); // solve

  // r = b - A*x, final L2 norm
  double norm2_out=0.;
  {
    Epetra_Vector epetra_r(*trhs->epetra_vector());
    Epetra_Vector m_mat_x(m_mat->OperatorRangeMap());
    m_mat->Apply(*tsol->epetra_vector(),m_mat_x);
    epetra_r.Update(-1.0,m_mat_x,1.0);
    epetra_r.Norm2(&norm2_out);
  }

  // print in and out residuals
  CFinfo << "Solver residuals: in " << norm2_in << ", out " << norm2_out << CFendl;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::set_values(const BlockAccumulator& values)
{
  cf_assert(m_is_created);
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
            double *bav=(double*)&values.mat(irow*m_neq,icol*m_neq);
            for (double* l=emv; emv<(const double*)(l+neqneq); bav+=rowoffset)
              for (double* m=emv; emv<(const double*)(m+m_neq);)
                *emv++ = *bav++;

            hits++;
            //break; // this is arguable, can be that the blockaccumulator fills to the same id more than once (repetitive same id entries in values.indices)
          }
        if (hits==numblocks) break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::add_values(const BlockAccumulator& values)
{
/* TRILINOS-ADVICED
  cf_assert(m_is_created);
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
  cf_assert(m_is_created);
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
  cf_assert(m_is_created);
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
            double *bav=(double*)&values.mat(irow*m_neq,icol*m_neq);
            for (double* l=emv; emv<(const double*)(l+neqneq); bav+=rowoffset)
              for (double* m=emv; emv<(const double*)(m+m_neq);)
                *emv++ += *bav++;

            hits++;
            //break; // this is arguable, can be that the blockaccumulator fills to the same id more than once (repetitive same id entries in values.indices)
          }
        if (hits==numblocks) break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::get_values(BlockAccumulator& values)
{
  cf_assert(m_is_created);
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
            double *bav=(double*)&values.mat(irow*m_neq,icol*m_neq);
            for (double* l=emv; emv<(const double*)(l+neqneq); bav+=rowoffset)
              for (double* m=emv; emv<(const double*)(m+m_neq);)
                *bav++ = *emv++;

            hits++;
            //break; // this is arguable, can be that the blockaccumulator fills to the same id more than once (repetitive same id entries in values.indices)
          }
        if (hits==numblocks) break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::set_row(const Uint iblockrow, const Uint ieq, Real diagval, Real offdiagval)
{
/* TRILINOS ADVICED
  cf_assert(m_is_created);
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
  cf_assert(m_is_created);
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

void TrilinosMatrix::get_column_and_replace_to_zero(const Uint iblockcol, Uint ieq, std::vector<Real>& values)
{
  /// @note this could be made faster if structural symmetry is ensured during create, because then involved rows could be determined by indices in the ibloccol-th row
  /// @attention COMPUTATIONALLY VERY EXPENSIVE!
  cf_assert(m_is_created);
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

void TrilinosMatrix::tie_blockrow_pairs (const Uint iblockrow_to, const Uint iblockrow_from)
{
  cf_assert(m_is_created);
  Epetra_SerialDenseMatrix **val_to,**val_from;
  int* colindices_to, *colindices_from;
  int blockrowsize_to,blockrowsize_from;
  int diag=-1,pair=-1;
  int dummy_neq;
  const int br_to=m_p2m[iblockrow_to];
  const int br_from=m_p2m[iblockrow_from];
  cf_assert(!(((br_to>=m_blockrow_size)&&(br_from<m_blockrow_size))||((br_to<m_blockrow_size)&&(br_from>=m_blockrow_size))));
  if ((br_to<m_blockrow_size)&&(br_from<m_blockrow_size))
  {
    TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(br_from,dummy_neq,blockrowsize_from,colindices_from,val_from));
    TRILINOS_ASSERT(m_mat->ExtractMyBlockRowView(br_to,  dummy_neq,blockrowsize_to,  colindices_to  ,val_to));
    if (blockrowsize_to!=blockrowsize_from) throw Common::BadValue(FromHere(),"Number of blocks do not match for the two block rows to be tied together.");
    for (int i=0; i<blockrowsize_to; i++)
    {
      cf_assert(colindices_to[i]==colindices_from[i]);
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

void TrilinosMatrix::set_diagonal(const std::vector<Real>& diag)
{
  cf_assert(m_is_created);
  int *dummy_rowcoldim;
  double *entryvals;
  int numblocks,dummy_stride;
  const unsigned long stride=m_neq+1;
  const unsigned long blockadvance=m_neq*m_neq;
  TRILINOS_ASSERT(m_mat->BeginExtractBlockDiagonalView(numblocks,dummy_rowcoldim));
  cf_assert(diag.size()==m_blockcol_size*m_neq);
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

void TrilinosMatrix::add_diagonal(const std::vector<Real>& diag)
{
  cf_assert(m_is_created);
  int *dummy_rowcoldim;
  double *entryvals;
  int numblocks,dummy_stride;
  const unsigned long stride=m_neq+1;
  const unsigned long blockadvance=m_neq*m_neq;
  TRILINOS_ASSERT(m_mat->BeginExtractBlockDiagonalView(numblocks,dummy_rowcoldim));
  cf_assert(diag.size()==m_blockcol_size*m_neq);
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

void TrilinosMatrix::get_diagonal(std::vector<Real>& diag)
{
  cf_assert(m_is_created);
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

void TrilinosMatrix::print(std::ostream& stream)
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

void TrilinosMatrix::print(const std::string& filename, std::ios_base::openmode mode )
{
  std::ofstream stream(filename.c_str(),mode);
  stream << "VARIABLES=COL,ROW,VAL\n" << std::flush;
  stream << "ZONE T=\"" << type_name() << "::" << name() <<  "\"\n" << std::flush;
  print(stream);
  stream.close();
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosMatrix::debug_data(std::vector<Uint>& row_indices, std::vector<Uint>& col_indices, std::vector<Real>& values)
{
  cf_assert(m_is_created);
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
