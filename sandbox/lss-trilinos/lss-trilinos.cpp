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
  lssm.reset();
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

  // performance fill
  lssm.reset();
  BlockAccumulator ba;
  ba.resize(3,3);
PEProcessSortedExecute(1,
  ba.mat.setConstant(-10.);
  ba.mat(0,0)=1.;
  ba.mat(0,3)=2.;
  ba.mat(0,6)=3.;
  ba.mat(3,0)=4.;
  ba.mat(3,3)=5.;
  ba.mat(3,6)=6.;
  ba.mat(6,0)=7.;
  ba.mat(6,3)=8.;
  ba.mat(6,6)=9.;
  ba.indices[0]=0;
  ba.indices[1]=2;
  ba.indices[2]=10;
  lssm.set_values(ba);
);

  // dirichletbc
  lssm.set_row(20,1,3.,4.);
  lssm.print_to_screen();


  // afscheid
  Comm::PE::instance().finalize();
};
