// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_scatter_hpp
#define cf3_common_PE_scatter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Foreach.hpp"
#include "common/BasicExceptions.hpp"

#include "common/PE/types.hpp"
#include "common/PE/datatype.hpp"

// #include "common/PE/debug.hpp" // for debugging mpi

////////////////////////////////////////////////////////////////////////////////

/**
  @file scatter.hpp
  @author Tamas Banyai
  Scatter collective communication interface to MPI standard.
  Due to the nature of MPI standard, at the lowest level the memory required to be linear meaning &xyz[0] should give a single and continous block of memory.
  Some functions support automatic evaluation of number of items on the receive side but be very cautious with using them because it requires two collective communication and may end up with degraded performance.
  Currently, the interface supports raw pointers and std::vectors.
  Three types of communications is implemented:
  - Constant size +send and receive on all processors via MPI_alltoall
  - Variable size send and receive via MPI_alltoallv
  - Extension of the variable sized communication to support mapped storage both on send and receive side.
**/

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  namespace common {
    namespace PE {

////////////////////////////////////////////////////////////////////////////////

namespace detail {

////////////////////////////////////////////////////////////////////////////////

  /**
    Implementation to the scatter interface with constant size communication.
    Don't call this function directly, use MPI::alltoall instead.
    In_values and out_values must be linear in memory and their sizes should be #processes*n.
    @param comm Comm::Communicator
    @param in_values pointer to the send buffer
    @param in_n size of the send array (number of items)
    @param out_values pointer to the receive buffer
    @param stride is the number of items of type T forming one array element, for example if communicating coordinates together, then stride==3:  X0,Y0,Z0,X1,Y1,Z1,...,Xn-1,Yn-1,Zn-1
  **/
  template<typename T>
  inline void
  scatterc_impl(const Communicator& comm, const T* in_values, const int in_n, T* out_values, const int root, const  int stride )
  {
    // get data type and number of processors
    Datatype type = PE::get_mpi_datatype(*in_values);
    int nproc,irank;
    MPI_CHECK_RESULT(MPI_Comm_size,(comm,&nproc));
    MPI_CHECK_RESULT(MPI_Comm_rank,(comm,&irank));

    // if stride is greater than one
    cf3_assert( stride>0 );

    // set up out_buf
    T* out_buf=out_values;
    if ((irank==root)&&(in_values==out_values)) {
      if ( (out_buf=new T[nproc*in_n*stride+1]) == (T*)0 ) throw cf3::common::NotEnoughMemory(FromHere(),"Could not allocate temporary buffer."); // +1 for avoiding possible zero allocation
    }

    // do the communication
    MPI_CHECK_RESULT(MPI_Scatter, (const_cast<T*>(in_values), in_n*stride, type, out_buf, in_n*stride, type, root, comm));

    // deal with out_buf
    if ((irank==root)&&(in_values==out_values)) {
      memcpy(out_values,out_buf,nproc*in_n*stride*sizeof(T));
      delete[] out_buf;
    }
  }

////////////////////////////////////////////////////////////////////////////////

  /**
    Implementation to the scatter interface with variable size communication through in and out map.
    Don't call this function directly, use MPI::alltoallvm instead.
    In_values and out_values must be linear in memory and their sizes should be sum(in_n[i]) and sum(out_n[i]) i=0..#processes-1.
    @param comm Comm::Communicator
    @param in_values pointer to the send buffer
    @param in_n array holding send counts of size #processes
    @param in_map array of size #processes holding the mapping. If zero pointer passed, no mapping on send side.
    @param out_values pointer to the receive buffer
    @param out_n array holding receive counts of size #processes. If zero pointer passed, no mapping on receive side.
    @param out_map array of size #processes holding the mapping
    @param stride is the number of items of type T forming one array element, for example if communicating coordinates together, then stride==3:  X0,Y0,Z0,X1,Y1,Z1,...,Xn-1,Yn-1,Zn-1
  **/
  template<typename T>
  inline void
  scattervm_impl(const Communicator& comm, const T* in_values, const int* in_n, const int* in_map, T* out_values, int& out_n, const int *out_map, const int root, const int stride )
  {
    // get data type and number of processors
    Datatype type = PE::get_mpi_datatype(*in_values);
    int nproc,irank;
    MPI_CHECK_RESULT(MPI_Comm_size,(comm,&nproc));
    MPI_CHECK_RESULT(MPI_Comm_rank,(comm,&irank));

    // if stride is smaller than one and unsupported functionality
    cf3_assert( stride>0 );

    int *in_nstride=(int*)0;
    int *in_disp=(int*)0;
    T *in_buf=0;
    int in_sum;
    if (irank==root){
      // compute displacements on send side
      in_nstride=new int[nproc];
      in_disp=new int[nproc];
      in_disp[0]=0;
      for(int i=0; i<nproc-1; i++) {
        in_nstride[i]=stride*in_n[i];
        in_disp[i+1]=in_disp[i]+in_nstride[i];
      }
      in_nstride[nproc-1]=in_n[nproc-1]*stride;
      in_sum=in_disp[nproc-1]+stride*in_n[nproc-1];
      // set up in_buf
      if (in_map!=0) {
        if ( (in_buf=new T[in_sum+1]) == (T*)0 ) throw cf3::common::NotEnoughMemory(FromHere(),"Could not allocate temporary buffer."); // +1 for avoiding possible zero allocation
        if (stride==1) { for(int i=0; i<in_sum; i++) in_buf[i]=in_values[in_map[i]]; }
        else { for(int i=0; i<(const int)(in_sum/stride); i++) memcpy(&in_buf[stride*i],&in_values[stride*in_map[i]],stride*sizeof(T)); }
      } else {
        in_buf=(T*)in_values;
      }
    }

    // compute total number of send and receive items
    const int out_sum=stride*out_n;
    const int out_nstride=stride*out_n;

    // set up out_buf
    T *out_buf=0;
    if ((out_map!=0)||(in_values==out_values)) {
      if ( (out_buf=new T[out_sum+1]) == (T*)0 ) throw cf3::common::NotEnoughMemory(FromHere(),"Could not allocate temporary buffer."); // +1 for avoiding possible zero allocation
    } else {
      out_buf=out_values;
    }

    // do the communication
    MPI_CHECK_RESULT(MPI_Scatterv, (in_buf, in_nstride, in_disp, type, out_buf, out_nstride, type, root, comm));

    // re-populate out_values
    if (out_map!=0) {
      if (stride==1) { for(int i=0; i<out_sum; i++) out_values[out_map[i]]=out_buf[i]; }
      else { for(int i=0; i<(const int)(out_sum/stride); i++) memcpy(&out_values[stride*out_map[i]],&out_buf[stride*i],stride*sizeof(T)); }
      delete[] out_buf;
    } else if (in_values==out_values) {
      memcpy(out_values,out_buf,out_sum*sizeof(T));
      delete[] out_buf;
    }

    // free internal memory
    if (irank==root) if (in_map!=0) delete[] in_buf;
    delete[] in_disp;
    delete[] in_nstride;
  }

////////////////////////////////////////////////////////////////////////////////

} // end namespace detail

////////////////////////////////////////////////////////////////////////////////

/**
  Interface to the constant size scatter communication with specialization to raw pointer.
  If null pointer passed for out_values then memory is allocated and the pointer to it is returned, otherwise out_values is returned.
  @param comm Comm::Communicator
  @param in_values pointer to the send buffer
  @param in_n size of the send array (number of items)
  @param out_values pointer to the receive buffer
  @param stride is the number of items of type T forming one array element, for example if communicating coordinates together, then stride==3:  X0,Y0,Z0,X1,Y1,Z1,...,Xn-1,Yn-1,Zn-1
**/
template<typename T>
inline T*
scatter(const Communicator& comm, const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
{
  // get nproc
  int nproc,irank;
  MPI_CHECK_RESULT(MPI_Comm_size,(comm,&nproc));
  MPI_CHECK_RESULT(MPI_Comm_rank,(comm,&irank));

  // allocate out_buf if incoming pointer is null
  T* out_buf=out_values;
  if (out_values==0) {
    const int size=stride*nproc*in_n>1?stride*nproc*in_n:1;
    if ( (out_buf=new T[size]) == (T*)0 ) throw cf3::common::NotEnoughMemory(FromHere(),"Could not allocate temporary buffer.");
  }

  // call c_impl
  if (irank==root){
    detail::scatterc_impl(comm, in_values, in_n, out_buf, root, stride);
  } else {
    detail::scatterc_impl(comm, (T*)0, in_n, out_buf, root, stride);
  }
  return out_buf;
}

////////////////////////////////////////////////////////////////////////////////

/**
  Interface to the constant size scatter communication with specialization to std::vector.
  @param comm Comm::Communicator
  @param in_values send buffer
  @param out_values receive buffer
  @param stride is the number of items of type T forming one array element, for example if communicating coordinates together, then stride==3:  X0,Y0,Z0,X1,Y1,Z1,...,Xn-1,Yn-1,Zn-1
**/
template<typename T>
inline void
scatter(const Communicator& comm, const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
{
  // get number of processors
  int nproc,irank;
  MPI_CHECK_RESULT(MPI_Comm_size,(comm,&nproc));
  MPI_CHECK_RESULT(MPI_Comm_rank,(comm,&irank));

  // set out_values's sizes
  cf3_assert( in_values.size() % (nproc*stride) == 0 );
  if (((in_values!=out_values)&&(irank!=root))||(out_values.size()==0)) {
    out_values.resize(in_values.size()/nproc);
    out_values.reserve(in_values.size()/nproc);
  }

  // call c_impl
  if (irank==root){
    detail::scatterc_impl(comm, (T*)(&in_values[0]), in_values.size()/(nproc*stride), (T*)(&out_values[0]), root, stride);
  } else {
    detail::scatterc_impl(comm, (T*)0, in_values.size()/(nproc*stride), (T*)(&out_values[0]), root, stride);
  }

  // reduce size if in_values=out_values
  if (irank==root) {
    out_values.resize(in_values.size()/nproc);
    out_values.reserve(in_values.size()/nproc);
  }
}

////////////////////////////////////////////////////////////////////////////////

//needs a forward
template<typename T>
inline T*
scatter(const Communicator& comm, const T* in_values, const int *in_n, const int *in_map, T* out_values, int& out_n, const int *out_map, const int root, const int stride=1);

/**
  Interface to the variable size scatter communication with specialization to raw pointer.
  If null pointer passed for out_values then memory is allocated and the pointer to it is returned, otherwise out_values is returned.
  If out_n (receive counts) contains only -1, then a pre communication occurs to fill out_n.
  @param comm Comm::Communicator
  @param in_values pointer to the send buffer
  @param in_n array holding send counts of size #processes
  @param out_values pointer to the receive buffer
  @param out_n array holding receive counts of size #processes
  @param stride is the number of items of type T forming one array element, for example if communicating coordinates together, then stride==3:  X0,Y0,Z0,X1,Y1,Z1,...,Xn-1,Yn-1,Zn-1
**/
template<typename T>
inline T*
scatter(const Communicator& comm, const T* in_values, const int* in_n, T* out_values, int& out_n, const int root, const int stride=1)
{
  // call mapped variable scatter
  return scatter(comm,in_values,in_n,0,out_values,out_n,0,root,stride);
}

////////////////////////////////////////////////////////////////////////////////

//needs a forward
template<typename T>
inline void
scatter(const Communicator& comm, const std::vector<T>& in_values, const std::vector<int>& in_n, const std::vector<int>& in_map, std::vector<T>& out_values, int& out_n, const std::vector<int>& out_map, const int root, const int stride=1);

/**
  Interface to the constant size scatter communication with specialization to std::vector.
  If out_values's size is zero or in_values==out_values then its resized.
  If out_n (receive counts) is not of size of #processes, then error occurs.
  If out_n (receive counts) is filled with -1s, then a pre communication occurs to fill out_n.
  @param comm Comm::Communicator
  @param in_values send buffer
  @param in_n send counts of size #processes
  @param out_values receive buffer
  @param out_n receive counts of size #processes
  @param stride is the number of items of type T forming one array element, for example if communicating coordinates together, then stride==3:  X0,Y0,Z0,X1,Y1,Z1,...,Xn-1,Yn-1,Zn-1
**/
template<typename T>
inline void
scatter(const Communicator& comm, const std::vector<T>& in_values, const std::vector<int>& in_n, std::vector<T>& out_values, int& out_n, const int root, const int stride=1)
{
  // call mapped variable scatter
  std::vector<int> in_map(0);
  std::vector<int> out_map(0);

  int irank;
  MPI_CHECK_RESULT(MPI_Comm_rank,(comm,&irank));

  if ((irank==root)&&(&in_values[0]==&out_values[0]))
  {
    std::vector<T> out_tmp(0);
    scatter(comm,in_values,in_n,in_map,out_tmp,out_n,out_map,root,stride);
    out_values.assign(out_tmp.begin(),out_tmp.end());
  }
  else
  {
    scatter(comm,in_values,in_n,in_map,out_values,out_n,out_map,root,stride);
  }

}

////////////////////////////////////////////////////////////////////////////////

/**
  Interface to the variable size mapped scatter communication with specialization to raw pointer.
  If null pointer passed for out_values then memory is allocated to fit the max in map and the pointer is returned, otherwise out_values is returned.
  If out_n (receive counts) contains only -1, then a pre communication occurs to fill out_n.
  However due to the fact that map already needs all the information if you use scatter to allocate out_values and fill out_n then you most probably doing something wrong.
  @param comm Comm::Communicator
  @param in_values pointer to the send buffer
  @param in_n array holding send counts of size #processes
  @param in_map array of size #processes holding the mapping. If zero pointer passed, no mapping on send side.
  @param out_values pointer to the receive buffer
  @param out_n array holding receive counts of size #processes. If zero pointer passed, no mapping on receive side.
  @param out_map array of size #processes holding the mapping
  @param stride is the number of items of type T forming one array element, for example if communicating coordinates together, then stride==3:  X0,Y0,Z0,X1,Y1,Z1,...,Xn-1,Yn-1,Zn-1
**/
template<typename T>
inline T*
scatter(const Communicator& comm, const T* in_values, const int *in_n, const int *in_map, T* out_values, int& out_n, const int *out_map, const int root, const int stride)
{
  // number of processes
  int nproc,irank;
  MPI_CHECK_RESULT(MPI_Comm_size,(comm,&nproc));
  MPI_CHECK_RESULT(MPI_Comm_rank,(comm,&irank));

  // if out_n consist of -1s then communicate for number of receives
  int out_sum=out_n;
  if (out_sum==-1) {
    if (out_map!=0) throw cf3::common::ParallelError(FromHere(),"Trying to perform communication with receive map while receive counts are unknown, this is bad usage of parallel environment.");
    detail::scatterc_impl(comm,in_n,1,&out_n,root,1);
    out_sum=out_n;
  }

  // allocate out_buf if incoming pointer is null
  T* out_buf=out_values;
  if (out_values==0) {
    if (out_map!=0){
      int out_sum_tmp=0;
      for (int i=0; i<out_sum; i++) out_sum_tmp=out_map[i]>out_sum_tmp?out_map[i]:out_sum_tmp;
      out_sum=out_sum_tmp+1;
    }
    if ( (out_buf=new T[stride*out_sum]) == (T*)0 ) throw cf3::common::NotEnoughMemory(FromHere(),"Could not allocate temporary buffer.");
  }

  // call vm_impl
  if (irank==root) {
    detail::scattervm_impl(comm, in_values, in_n, in_map, out_buf, out_n, out_map, root, stride);
  } else {
    detail::scattervm_impl(comm, (T*)0, in_n, in_map, out_buf, out_n, out_map, root, stride);
  }
  return out_buf;
}

////////////////////////////////////////////////////////////////////////////////

/**
  Interface to the constant size scatter communication with specialization to raw pointer.
  If out_values's size is zero then its resized.
  If out_n (receive counts) is not of size of #processes, then error occurs.
  If out_n (receive counts) is filled with -1s, then a pre communication occurs to fill out_n.
  However due to the fact that map already needs all the information if you use scatter to allocate out_values and fill out_n then you most probably doing something wrong.
  @param comm Comm::Communicator
  @param in_values send buffer
  @param in_n send counts of size #processes
  @param in_map array of size #processes holding the mapping. If zero pointer or zero size vector passed, no mapping on send side.
  @param out_values receive buffer
  @param out_n receive counts of size #processes
  @param out_map array of size #processes holding the mapping. If zero pointer or zero size vector passed, no mapping on receive side.
  @param stride is the number of items of type T forming one array element, for example if communicating coordinates together, then stride==3:  X0,Y0,Z0,X1,Y1,Z1,...,Xn-1,Yn-1,Zn-1
**/
template<typename T>
inline void
scatter(const Communicator& comm, const std::vector<T>& in_values, const std::vector<int>& in_n, const std::vector<int>& in_map, std::vector<T>& out_values, int& out_n, const std::vector<int>& out_map, const int root, const int stride)
{
  // number of processes and checking in_n and out_n (out_n deliberately throws exception because the vector can arrive from arbitrary previous usage)
  int nproc,irank;
  MPI_CHECK_RESULT(MPI_Comm_size,(comm,&nproc));
  MPI_CHECK_RESULT(MPI_Comm_rank,(comm,&irank));
  cf3_assert( (int)in_n.size() == nproc );

  // compute number of send and receive
  int in_sum=0;
  int out_sum=out_n;
  boost_foreach( int i, in_n ) in_sum+=i;

  // if necessary, do communication for out_n
  if (out_sum == -1){
    if (out_map.size()!=0) throw cf3::common::ParallelError(FromHere(),"Trying to perform communication with receive map while receive counts are unknown, this is bad usage of parallel environment.");
    detail::scatterc_impl(comm,&in_n[0],1,&out_n,root,1);
    out_sum=out_n;
  }

  // resize out_values if vector size is zero
  if (out_values.size() == 0 ){
    if (out_map.size()!=0) {
      out_sum=0;
      boost_foreach( int i, out_map ) out_sum=i>out_sum?i:out_sum;
      if (out_sum!=0) out_sum++;
    }
    out_values.resize(stride*out_sum);
    out_values.reserve(stride*out_sum);
  }

  // call vm_impl
  if (irank==root) {
    detail::scattervm_impl(comm, (T*)(&in_values[0]), &in_n[0], (in_map.empty() ? nullptr : &in_map[0]), (T*)(&out_values[0]), out_n, (out_map.empty() ? nullptr : &out_map[0]), root, stride);
  } else {
    detail::scattervm_impl(comm, (T*)0, &in_n[0], (in_map.empty() ? nullptr : &in_map[0]), (T*)(&out_values[0]), out_n, (out_map.empty() ? nullptr : &out_map[0]), root, stride);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace PE
} // namespace common
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_scatter_hpp
