// Copyright (C) 2005, 2006 Douglas Gregor.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Message Passing Interface 1.1 -- Section 4.6. Scatter
#ifndef BOOST_MPI_SCATTERV_HPP
#define BOOST_MPI_SCATTERV_HPP

#include <boost/mpi/exception.hpp>
#include <boost/mpi/datatype.hpp>
#include <vector>
//#include <boost/mpi/packed_oarchive.hpp>
//#include <boost/mpi/packed_iarchive.hpp>
//#include <boost/mpi/detail/point_to_point.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/assert.hpp>

namespace boost { namespace mpi {

namespace detail {

  // We're scattering from the root for a type that has an associated MPI
  // datatype, so we'll use MPI_Scatter to do all of the work.
  template<typename T>
  void
  scatterv_impl(const communicator& comm, const T* in_values, T* out_values,
               int *in_n, int out_n, int root, mpl::true_)
  {
    // computing displacement vector
    int *disp=new int[communicator::size()];
    disp[0]=0;
    for (int i=1;i<(const int)communicator::size();i++)
      disp[i]=disp[i-1]+in_n[i-1];

    // allocating a buffer if in_values points into the same memory than out_values
    T *buf=out_values;
    if (communicator::rank()==root)
      if (out_values==&in_values[disp[root]]){
        buf=new T[out_n];
        memcpy(buf,out_values,out_n*sizeof(T));
      }

    // calling the communication
    MPI_Datatype type = get_mpi_datatype<T>(*in_values);
    BOOST_MPI_CHECK_RESULT(
      MPI_Scatterv,(const_cast<T*>(in_values), in_n, disp, type, const_cast<T*>(buf), out_n, type, root, comm)
    );

    // copy back from buf when necessary and free temporary stuff
    if (buf!=out_values) {
      memcpy(out_values,buf,out_n*sizeof(T));
      delete buf;
    }
    delete disp;
  }


  // We're scattering from the root for a type that has an associated MPI
  // datatype, so we'll use MPI_Scatter to do all of the work.
  template<typename T>
  void
  scattervm_impl(const communicator& comm, const T* in_values, T* out_values,
                 int *in_n, int out_n, int *in_map, int *out_map, int root, mpl::true_)
  {
    // computing displacement vector and sum
    int *disp=new int[communicator::size()];
    int in_totn=0;
    disp[0]=0;
    for (int i=1;i<(const int)communicator::size();i++)
      disp[i]=disp[i-1]+in_n[i-1];
    for (int i=0;i<(const int)communicator::size();i++)
      in_totn+=(int)in_n[i];

    // allocating buffer for in and out and filling in_v buffer
    T *out_v=new T[out_n];
    T *in_v=new T[in_n[communicator::rank()]];
    if (in_map!=0) {
      for (int i=0; i<(const int)in_n[communicator::rank()]; i++)
        in_v[i]=in_values[in_map[i]];
    } else {
      memcpy(in_v,in_values,in_totn*sizeof(T));
    }

    // calling the communication
    MPI_Datatype type = get_mpi_datatype<T>(*in_values);
    BOOST_MPI_CHECK_RESULT(
      MPI_Scatterv,(const_cast<T*>(in_v), in_n, disp, type, const_cast<T*>(out_v), out_n, type, root, comm)
    );

    // distributing back to out_values and freeing buffers
    if (out_map!=0) {
      for (int i=0; i<out_n; i++)
        out_values[out_map[i]]=out_v[i];
    } else {
      memcpy(out_values,out_v,out_n*sizeof(T));
    }
    delete in_v;
    delete out_v;
    delete disp;
  }

} // end namespace detail



// pre-comm for receive count
template<typename T>
void
scatterv(const communicator& comm, const T* in_values, T* out_values, int *in_n, int root)
{
  int out_n;
  mpi::scatter(comm,in_n,&out_n,1,root);
  mpi::detail::scatterv_impl(comm,in_values,out_values,in_n,out_n,root);
}

// pre-comm for receive count,
template<typename T>
void
scatterv(const communicator& comm, const T* in_values, T* out_values, int *in_n, int *out_n, int root)
{
  mpi::scatter(comm,in_n,out_n,1,root);
  mpi::detail::scatterv_impl(comm,in_values,out_values,in_n,*out_n,root);
}

// knowing size on both sides
template<typename T>
void
scatterv(const communicator& comm, const T* in_values, T* out_values, int *in_n, int out_n, int root)
{
  mpi::detail::scatterv_impl(comm,in_values,out_values,in_n,out_n,root);
}

// knowing size on both sides with mapping
template<typename T>
void
scatterv(const communicator& comm, const T* in_values, T* out_values, int *in_n, int out_n, int *in_map, int *out_map, int root)
{
  mpi::detail::scattervm_impl(comm,in_values,out_values,in_n,out_n,in_map,out_map,root);
}



} } // end namespace boost::mpi

#endif // BOOST_MPI_SCATTERV_HPP
