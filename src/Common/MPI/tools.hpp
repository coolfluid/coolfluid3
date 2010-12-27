// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef BOOST_MPI_tools_HPP
#define BOOST_MPI_tools_HPP

////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <boost/thread/thread.hpp>

#include <Common/Log.hpp>
#include <Common/LogStream.hpp>
#include <Common/BasicExceptions.hpp>
#include <Common/CodeLocation.hpp>

////////////////////////////////////////////////////////////////////////////////

/**
  @file tools.hpp
  @author Tamas Banyai

  This header defines additional support functions/macros/classes.
**/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
    namespace mpi {

////////////////////////////////////////////////////////////////////////////////

/**
  Macro for checking return values of any mpi calls and throws exception on error.
**/
#define MPI_CHECK_RESULT( MPIFunc, Args )                                                                         \
 {                                                                                                                \
   int _check_result = MPIFunc Args;                                                                              \
   if (_check_result != MPI_SUCCESS)                                                                              \
   {                                                                                                              \
     std::stringstream errmsg;                                                                                    \
     errmsg << "Function: " << #MPIFunc << " did not return MPI_SUCCESS (" << _check_result << ").";              \
     throw Common::ParallelError(FromHere(),errmsg.str() );                                                       \
   }                                                                                                              \
 }

////////////////////////////////////////////////////////////////////////////////

/**
  Macro for executing something ensured that the execution order is 0..nproc-1.
  @param comm communicator of the mpi environment
  @param irank rank of the process where the command is executed (-1 for all processes)
  @param expression stuff to execute
**/
#define PEProcessSortedExecute(pe,irank,expression) {                                                                           \
  CFinfo.setFilterRankZero(false);                                                                                              \
  if (irank<0){                                                                                                                 \
    int _process_sorted_execute_i_;                                                                                             \
    int _process_sorted_execute_n_=(int)(pe.size());                                                                            \
    int _process_sorted_execute_r_=(int)(pe.rank());                                                                            \
    pe.barrier();                                                                                                               \
    CFinfo << CFflush;                                                                                                          \
    pe.barrier();                                                                                                               \
    for(_process_sorted_execute_i_=0; _process_sorted_execute_i_<_process_sorted_execute_n_; _process_sorted_execute_i_++){     \
      pe.barrier();                                                                                                             \
      if(_process_sorted_execute_i_ == _process_sorted_execute_r_){                                                             \
        expression;                                                                                                             \
        CFinfo << CFflush;                                                                                                      \
        pe.barrier();                                                                                                           \
      }                                                                                                                         \
    }                                                                                                                           \
    pe.barrier();                                                                                                               \
    CFinfo << CFflush;                                                                                                          \
    pe.barrier();                                                                                                               \
  } else if (irank==(int)(pe.rank())){                                                                                          \
    expression;                                                                                                                 \
  }                                                                                                                             \
  CFinfo.setFilterRankZero(true);                                                                                               \
}

////////////////////////////////////////////////////////////////////////////////

/**
  Macro for a checkpoint, all possible effort is made to have a non-aliased, rank ordered message output.
  @param msec milliseconds to wait, called before and after so overall delay is 2*msec
  @param msg message to print on stdout
**/

#define  PECheckPoint(msec,msg) {                                                                                               \
PE::instance().barrier();                                                                                                     \
std::cout << std::flush;                                                                                                      \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
PE::instance().barrier();                                                                                                     \
PEProcessSortedExecute(PE::instance(),-1,                                                                                                    \
std::cout << std::flush;                                                                                                    \
std::cout << "["<<PE::instance().rank() << "] " << msg << "\n";                                                                   \
std::cout << std::flush;                                                                                                    \
);                                                                                                                            \
PE::instance().barrier();                                                                                                     \
std::cout << std::flush;                                                                                                      \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
}
	
////////////////////////////////////////////////////////////////////////////////

#define  PECheckArrivePoint(msec,msg) {                                                                                               \
std::cout << std::flush;                                                                                                      \
std::cout << "["<<PE::instance().rank() << "] " << msg << "\n";                                                                   \
std::cout << std::flush;                                                                                                    \
PE::instance().barrier();                                                                                                     \
boost::this_thread::sleep(boost::posix_time::milliseconds(msec));                                                             \
}

	
////////////////////////////////////////////////////////////////////////////////

/**
 Macro for printing an std::Vector
**/
#define PEDebugVector(v,size) { \
  CFinfo << PE::instance().rank() << "/" << PE::instance().rank() << ": " << #v << CFendl; \
  for(int _tmp_i_=0; _tmp_i_<(int)size; _tmp_i_++)  CFinfo << v[_tmp_i_] << " "; \
  CFinfo << CFendl; \
}

////////////////////////////////////////////////////////////////////////////////

    } // end namespace mpi
  } // end namespace Common
} // end namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // BOOST_MPI_tools_HPP
