// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_common_PE_debug_HPP
#define cf3_common_PE_debug_HPP

////////////////////////////////////////////////////////////////////////////////

#include <iostream> // should be avoided but this is a debug header which usually is not included

#include <boost/thread/thread.hpp>

#include "common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

/**
  @file debug.hpp
  @author Tamas Banyai

  This header defines additional debug support functions/macros/classes.
**/

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  namespace common {
    namespace PE {

////////////////////////////////////////////////////////////////////////////////

#define PERank "["<<::cf3::common::PE::Comm::instance().rank() << "] "

/**
  Macro for executing something ensured that the execution order is 0..nproc-1.
  @param irank rank of the process where the command is executed (-1 for all processes)
  @param expression stuff to execute
**/
#define PEProcessSortedExecute(irank,expression) {                                                                           \
  if (irank<0){                                                                                                                 \
    int _process_sorted_execute_i_;                                                                                             \
    int _process_sorted_execute_n_=(int)(::cf3::common::PE::Comm::instance().size());                                                                            \
    int _process_sorted_execute_r_=(int)(::cf3::common::PE::Comm::instance().rank());                                                                            \
    ::cf3::common::PE::Comm::instance().barrier();                                                                                                               \
    std::cout << std::flush;                                                                                                          \
    ::cf3::common::PE::Comm::instance().barrier();                                                                                                               \
    for(_process_sorted_execute_i_=0; _process_sorted_execute_i_<_process_sorted_execute_n_; _process_sorted_execute_i_++){     \
      ::cf3::common::PE::Comm::instance().barrier();                                                                                                             \
      if(_process_sorted_execute_i_ == _process_sorted_execute_r_){                                                             \
        expression;                                                                                                             \
        std::cout << std::flush;                                                                                                      \
        ::cf3::common::PE::Comm::instance().barrier();                                                                                                           \
      }                                                                                                                         \
    }                                                                                                                           \
    ::cf3::common::PE::Comm::instance().barrier();                                                                                                               \
    std::cout << std::flush;                                                                                                          \
    ::cf3::common::PE::Comm::instance().barrier();                                                                                                               \
  } else if (irank==(int)(::cf3::common::PE::Comm::instance().rank())){                                                                                          \
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
::cf3::common::PE::Comm::instance().barrier();                                                                                                     \
std::cout << std::flush;                                                                                                      \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
::cf3::common::PE::Comm::instance().barrier();                                                                                                     \
PEProcessSortedExecute(-1,                                                                                                    \
  std::cout << std::flush;                                                                                                    \
  std::cout << "["<<::cf3::common::PE::Comm::instance().rank()  << ":" << __FILE__ << ":" << __LINE__ << "] " << msg << "\n";                                                                   \
  std::cout << std::flush;                                                                                                    \
);                                                                                                                            \
::cf3::common::PE::Comm::instance().barrier();                                                                                                     \
std::cout << std::flush;                                                                                                      \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
::cf3::common::PE::Comm::instance().barrier();                                                                                                     \
}

////////////////////////////////////////////////////////////////////////////////

#define  PECheckArrivePoint(msec,msg) {                                                                                               \
std::cout << std::flush;                                                                                                      \
std::cout << "["<<::cf3::common::PE::Comm::instance().rank() << "] " << msg << "\n";                                                                   \
std::cout << std::flush;                                                                                                    \
::cf3::common::PE::Comm::instance().barrier();                                                                                                     \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
}


////////////////////////////////////////////////////////////////////////////////

/**
 Macro for printing a vector
**/
#define PEDebugVector(v,length) { \
  std::cout << ::cf3::common::PE::Comm::instance().rank() << "/" << ::cf3::common::PE::Comm::instance().size() << ": " << #v << " " << length << " ( " << std::flush; \
  for(int _tmp_i_=0; _tmp_i_<length; _tmp_i_++)  std::cout << v[_tmp_i_] << " "; \
  std::cout << " )\n" << std::flush; \
}

////////////////////////////////////////////////////////////////////////////////

/**
 Macro for printing a member of a struct/class inside a vector.
 You have to put . or -> for the member, or can even call a function like ->size().
**/
#define PEDebugVectorMember(v,length,member) { \
  std::cout << ::cf3::common::PE::Comm::instance().rank() << "/" << ::cf3::common::PE::Comm::instance().size() << ": " << #v#member << " " << length << " ( " << std::flush; \
  for(int _tmp_i_=0; _tmp_i_<length; _tmp_i_++)  std::cout << v[_tmp_i_]member << " "; \
  std::cout << " )\n" << std::flush; \
}

////////////////////////////////////////////////////////////////////////////////

/// Called to wait for a debugger. You need to use the debugger interface to set the variable
/// stopped to 0
/// @param rank Rank that has to wait, or if -1 all processes will wait
void wait_for_debugger(const int rank = -1);

////////////////////////////////////////////////////////////////////////////////

    } // end namespace PE
  } // end namespace common
} // end namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_debug_HPP
