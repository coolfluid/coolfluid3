// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_common_PE_debug_HPP
#define cf3_common_PE_debug_HPP

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <boost/thread/thread.hpp>

#include "common/PE/debug.hpp"
#include "common/PE/Comm.hpp"

namespace cf3 {
namespace common {
namespace PE {

void wait_for_debugger(const int rank)
{
  const Uint irank = Comm::instance().rank();
  if(rank >= 0 && rank != irank)
    return;

  int stopped = 1;
  int i = 0;
  std::cout << "Rank " << irank << " with PID " << getpid() << " is waiting for debugger attach" << std::endl;
  while (0 != stopped && i++ < 80)
    boost::this_thread::sleep(boost::posix_time::milliseconds(250));
}


} // PE
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_debug_HPP
