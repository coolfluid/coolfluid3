// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include "common/Builder.hpp"
#include "common/LibCommon.hpp"

#include "common/Log.hpp"
#include "common/Signal.hpp"

#include "common/PE/Comm.hpp"

#include "Tools/solver/CWorker.hpp"

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace solver {

////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CWorker, Component, LibCommon > CWorker_Builder;

////////////////////////////////////////////////////////////////////////////

CWorker::CWorker ( const std::string & name ) :
  Component(name)
{
  regist_signal( "solve" )
    .description("Runs a fake simulation")
    .pretty_name("Solve")
    .connect( boost::bind(&CWorker::signal_solve, this, _1));

  signal("solve")->hidden(true);
}

////////////////////////////////////////////////////////////////////////////

CWorker::~CWorker()
{
}

////////////////////////////////////////////////////////////////////////////

void CWorker::signal_solve ( SignalArgs & args )
{
  CFinfo << "Worker[" <<  PE::Comm::instance().rank() << "] Starting to solve." << CFendl;

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

    PE::Comm::instance().barrier();
    CFinfo << "Worker[" <<  PE::Comm::instance().rank() << "] Iteration " << i << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////

} // solver
} // Tools
} // cf3
