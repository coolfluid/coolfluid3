// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/**
  @file lss-trilinos Testbed for designing the linear solver interface and implementing trilinos behind.
  Will use Epetra::FECrsMatrix for the time beign, TPetra does not support Stratimikos. Conversion seems reasonably simple according to trilinos doc.
**/

#include <Common/MPI/PECommPattern.hpp>

#include "lss-trilinos.hpp"
#include "test_matrix.hpp"

using namespace CF::Common;

int main(void)
{
  // get to business
  mpi::PE::instance().init();

  // reference matrix
  boost::assign::test_matrix m;

  // make a commpattern as the most likely input
  PECommPattern cp("cp");
  cp.insert("gid",m.global_numbering,1,false);
  cp.setup(cp.get_child_ptr("gid")->as_ptr<PEObjectWrapper>(),m.irank_updatable);

  // setting up the matrix
  LSSTrilinosMatrix lssm;

  // afscheid
  mpi::PE::instance().finalize();
};
