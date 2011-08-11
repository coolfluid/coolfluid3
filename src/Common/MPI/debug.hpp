// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_mpi_debug_HPP
#define CF_Common_mpi_debug_HPP

////////////////////////////////////////////////////////////////////////////////

#include <iostream> // should be avoided but this is a debug header which usually is not included

#include <boost/thread/thread.hpp>

#include "Common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

/**
  @file debug.hpp
  @author Tamas Banyai

  This header defines additional debug support functions/macros/classes.
**/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
    namespace mpi {

////////////////////////////////////////////////////////////////////////////////

#define PERank "["<<::CF::Common::mpi::PE::instance().rank() << "] "

/**
  Macro for executing something ensured that the execution order is 0..nproc-1.
  @param irank rank of the process where the command is executed (-1 for all processes)
  @param expression stuff to execute
**/
#define PEProcessSortedExecute(irank,expression) {                                                                           \
  if (irank<0){                                                                                                                 \
    int _process_sorted_execute_i_;                                                                                             \
    int _process_sorted_execute_n_=(int)(::CF::Common::mpi::PE::instance().size());                                                                            \
    int _process_sorted_execute_r_=(int)(::CF::Common::mpi::PE::instance().rank());                                                                            \
    ::CF::Common::mpi::PE::instance().barrier();                                                                                                               \
    std::cout << std::flush;                                                                                                          \
    ::CF::Common::mpi::PE::instance().barrier();                                                                                                               \
    for(_process_sorted_execute_i_=0; _process_sorted_execute_i_<_process_sorted_execute_n_; _process_sorted_execute_i_++){     \
      ::CF::Common::mpi::PE::instance().barrier();                                                                                                             \
      if(_process_sorted_execute_i_ == _process_sorted_execute_r_){                                                             \
        expression;                                                                                                             \
        std::cout << std::flush;                                                                                                      \
        ::CF::Common::mpi::PE::instance().barrier();                                                                                                           \
      }                                                                                                                         \
    }                                                                                                                           \
    ::CF::Common::mpi::PE::instance().barrier();                                                                                                               \
    std::cout << std::flush;                                                                                                          \
    ::CF::Common::mpi::PE::instance().barrier();                                                                                                               \
  } else if (irank==(int)(::CF::Common::mpi::PE::instance().rank())){                                                                                          \
    expression;                                                                                                                 \
  }                                                                                                                             \
}

////////////////////////////////////////////////////////////////////////////////

/**
  Macro for a checkpoint, all possible effort is made to have a non-aliased, rank ordered message output.
  @param msec milliseconds to wait, called before and after so overall delay is 2*msec
  @param msg message to print on stdout
**/

#define  PECheckPoint(msec,msg) {                                                                                               \
::CF::Common::mpi::PE::instance().barrier();                                                                                                     \
std::cout << std::flush;                                                                                                      \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
::CF::Common::mpi::PE::instance().barrier();                                                                                                     \
PEProcessSortedExecute(-1,                                                                                                    \
  std::cout << std::flush;                                                                                                    \
  std::cout << "["<<::CF::Common::mpi::PE::instance().rank()  << ":" << __FILE__ << ":" << __LINE__ << "] " << msg << "\n";                                                                   \
  std::cout << std::flush;                                                                                                    \
);                                                                                                                            \
::CF::Common::mpi::PE::instance().barrier();                                                                                                     \
std::cout << std::flush;                                                                                                      \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
::CF::Common::mpi::PE::instance().barrier();                                                                                                     \
}

////////////////////////////////////////////////////////////////////////////////

#define  PECheckArrivePoint(msec,msg) {                                                                                               \
std::cout << std::flush;                                                                                                      \
std::cout << "["<<::CF::Common::mpi::PE::instance().rank() << "] " << msg << "\n";                                                                   \
std::cout << std::flush;                                                                                                    \
::CF::Common::mpi::PE::instance().barrier();                                                                                                     \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
}


////////////////////////////////////////////////////////////////////////////////

/**
 Macro for printing a vector
**/
#define PEDebugVector(v,length) { \
  std::cout << ::CF::Common::mpi::PE::instance().rank() << "/" << ::CF::Common::mpi::PE::instance().size() << ": " << #v << " " << length << " ( " << std::flush; \
  for(int _tmp_i_=0; _tmp_i_<length; _tmp_i_++)  std::cout << v[_tmp_i_] << " "; \
  std::cout << " )\n" << std::flush; \
}

////////////////////////////////////////////////////////////////////////////////

    } // end namespace mpi
  } // end namespace Common
} // end namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_mpi_debug_HPP
