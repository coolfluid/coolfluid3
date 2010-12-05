// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef BOOST_MPI_tools_HPP
#define BOOST_MPI_tools_HPP

////////////////////////////////////////////////////////////////////////////////

#include <boost/mpi/communicator.hpp>
#include <boost/thread/thread.hpp>

////////////////////////////////////////////////////////////////////////////////

/**
  @file tools.hpp
  @author Tamas Banyai

  This header defines additional support functions/macros/classes.
**/

////////////////////////////////////////////////////////////////////////////////

namespace boost { namespace mpi {

////////////////////////////////////////////////////////////////////////////////

/**
  Macro for executing something ensured that the execution order is 0..nproc-1.
  @param comm communicator of the mpi environment
  @param irank rank of the process where the command is executed (-1 for all processes)
  @param expression stuff to execute
**/
#define PEProcessSortedExecute(comm,irank,expression) {                                                                         \
  if (irank<0){                                                                                                                 \
    int _process_sorted_execute_i_;                                                                                             \
    int _process_sorted_execute_n_=(int)comm.size();                                                                            \
    int _process_sorted_execute_r_=(int)comm.rank();                                                                            \
    comm.barrier();                                                                                                             \
    for(_process_sorted_execute_i_=0; _process_sorted_execute_i_<_process_sorted_execute_n_; _process_sorted_execute_i_++){     \
      comm.barrier();                                                                                                           \
      if(_process_sorted_execute_i_ == _process_sorted_execute_r_){                                                             \
        expression;                                                                                                             \
        comm.barrier();                                                                                                         \
      }                                                                                                                         \
    }                                                                                                                           \
    comm.barrier();                                                                                                             \
  } else if (irank==(int)comm.rank()){                                                                                          \
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

} // end namespace mpi
} // end namespace boost

////////////////////////////////////////////////////////////////////////////////

#endif // BOOST_MPI_tools_HPP
