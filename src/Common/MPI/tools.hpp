// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef BOOST_MPI_tools_HPP
#define BOOST_MPI_tools_HPP

////////////////////////////////////////////////////////////////////////////////

#include <boost/mpi/communicator.hpp>

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
  mpiProcessSortedExecute is a macro for executing something ensured that the execution order is 0..nproc-1.
  @param comm communicator of the mpi environment
  @param irank rank of the process where the command is executed (-1 for all processes)
  @param expression stuff to execute
**/
#define ProcessSortedExecute(comm,irank,expression) {                                                                           \
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

} // end namespace mpi
} // end namespace boost

////////////////////////////////////////////////////////////////////////////////

#endif // BOOST_MPI_tools_HPP
