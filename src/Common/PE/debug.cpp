// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_MPI_debug_HPP
#define CF_Common_MPI_debug_HPP

////////////////////////////////////////////////////////////////////////////////

#include <boost/thread/thread.hpp>

#include "Common/PE/debug.hpp"
#include "Common/PE/Comm.hpp"

namespace CF {
  namespace Common {
    namespace PE {

void wait_for_debugger(const int rank)
{
  const Uint irank = Comm::instance().rank();
  if(rank >= 0 && rank != irank)
    return;
    
  int stopped = 1;
  std::cout << "Rank " << irank << " with PID " << getpid() << " is waiting for debugger attach" << std::endl;
  while (0 != stopped)
    boost::this_thread::sleep(boost::posix_time::milliseconds(250));
}


    } // end namespace PE
  } // end namespace Common
} // end namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_debug_HPP
