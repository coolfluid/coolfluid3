// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/**
  @file lss-trilinos Testbed for designing the linear solver interface and implementing trilinos behind.
  Will use Epetra::FEVbrMatrix for the time beign, TPetra does not support Stratimikos. Conversion seems reasonably simple according to trilinos doc.
**/

#include "Math/MatrixTypes.hpp"
#include "lss-trilinos.hpp"
#include "test_matrix.hpp"

using namespace CF::Common;
using namespace CF::Common::Comm;

int main(void)
{
  // get to business
  Comm::PE::instance().init();

  // reference matrix
  boost::assign::test_matrix m;

  // make a commpattern as the most likely input
  CommPattern cp("cp");
  cp.insert("gid",m.global_numbering,1,false);
  cp.setup(cp.get_child_ptr("gid")->as_ptr<CommWrapper>(),m.irank_updatable);

  // setting up the matrix
  LSSTrilinosMatrix lssm("lssm");
  lssm.create_sparsity(cp,m.column_indices,m.rowstart_positions);

  // manual filling every entry
  lssm.reset_to_zero();
  int irow=0;
  int ientry=0;
  PEProcessSortedExecute(1,
  for (int i=0; i<m.rowstart_positions.size()-1; i++)
    if (m.irank_updatable[i]==Comm::PE::instance().rank())
    {
      for (int ieq=0; ieq<3; ieq++)
        for (int j=m.rowstart_positions[i]; j<m.rowstart_positions[i+1]; j++)
        {
          lssm.add_value(3*irow+ieq,3*m.column_indices[j]+0,m.mat_presolve[ientry++]);
          lssm.add_value(3*irow+ieq,3*m.column_indices[j]+1,m.mat_presolve[ientry++]);
          lssm.add_value(3*irow+ieq,3*m.column_indices[j]+2,m.mat_presolve[ientry++]);
        }
      irow++;
    }
  );
//  lssm.print_to_screen();

  // performance fill
  lssm.reset_to_zero();
  BlockAccumulator ba;
  ba.resize(3,3);
PEProcessSortedExecute(1,
  ba.mat(0,0)=100.;
  ba.mat(0,1)=101.;
  ba.mat(0,2)=102.;
  ba.mat(1,0)=110.;
  ba.mat(1,1)=111.;
  ba.mat(1,2)=112.;
  ba.mat(2,0)=220.;
  ba.mat(2,1)=221.;
  ba.mat(2,2)=222.;
  ba.indices[0]=0;
  ba.indices[1]=2;
  ba.indices[2]=10;
  lssm.set_values(ba);
);
//  lssm.print_to_screen();

  // afscheid
  Comm::PE::instance().finalize();
};
