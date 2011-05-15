// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include "Common/Log.hpp"
#include "Common/Signal.hpp"

#include "Common/MPI/PE.hpp"

#include "Tools/Solver/CWorker.hpp"

using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Solver {

////////////////////////////////////////////////////////////////////////////

CWorker::CWorker ( const std::string & name ) :
  Component(name)
{
  regist_signal("solve", "Runs a fake simulation.", "Solve")
      ->signal->connect( boost::bind(&CWorker::signal_solve, this, _1));

  signal("solve")->is_hidden = true;
}

////////////////////////////////////////////////////////////////////////////

CWorker::~CWorker()
{

}

////////////////////////////////////////////////////////////////////////////

void CWorker::signal_solve ( SignalArgs & args )
{
  CFinfo << "Worker[" <<  mpi::PE::instance().rank() << "] Starting to solve." << CFendl;

  Real data[500000]; // 500 thousand elements

  // fill the array
  for( int val = 0 ; val < 500000 ; ++val)
    data[ val ] = (Real) val;

  for(int i = 0 ; i < 10000 ; ++i)
  {
    for( int j = 0 ; j < 500000 ; ++j )
    {
      /// @todo add some mpi operations like all_reduce

      Real value = std::sqrt( std::pow(data[j],3) * 2.71 / 3.141592 );
    }

    mpi::PE::instance().barrier();
    CFinfo << "Worker[" <<  mpi::PE::instance().rank() << "] Iteration " << i << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////

} // Solver
} // Tools
} // CF
