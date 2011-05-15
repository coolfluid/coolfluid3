// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <mpi.h>

#include "Common/Core.hpp"
#include "Common/Log.hpp"

#include "Common/MPI/PE.hpp"
#include "Common/MPI/ListeningThread.hpp"

using namespace MPI;
using namespace CF::Common;
using namespace CF::Common::mpi;

int main(int argc, char ** argv)
{
  // initiate the CF core and MPI environment
  Core::instance().initiate(argc, argv);
  PE::instance().init(argc, argv);
  Logger::instance().getStream( Logger::INFO ).setFilterRankZero(false);

  if( mpi::PE::instance().get_parent() != COMM_NULL )
  {
    CFinfo << "Worker[" << PE::instance().rank() << "] -> I have a parent. " <<
              "We are " << mpi::PE::instance().size() << " in this universe." << CFendl;
  }
  else
    CFerror << "A worker cannot be run without a manager." << CFendl;

  PE::instance().barrier();

  // terminate the CF core and MPI environment
  PE::instance().finalize();
  Core::instance().terminate();

  return 0;
}
