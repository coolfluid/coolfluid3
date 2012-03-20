// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Buffer.hpp
/// @author Willem Deconinck
/// @brief MPI communication buffer for mixed data types

#ifndef CF3_COMMON_PE_Buffer_hpp
#define CF3_COMMON_PE_Buffer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BoostArray.hpp"
#include "common/PE/Comm.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace PE {

////////////////////////////////////////////////////////////////////////////////

/// @brief Buffer that can hold multiple data types, useful for MPI communication
///
/// Buffer can be packed repetitively, with different data types.
/// The size of the buffer grows automatically
/// The buffer can be unpacked repetitively. Knowledge of every unpacked type
/// must be known.
/// @author Willem Deconinck
class Common_API Buffer
{
public:
  typedef char* iterator;

  /// @name Constructor/Destructor
  //@{
  /// @brief Constructor
  Buffer(const Uint size = 0u)
    : m_buffer(nullptr),
      m_size(0),
      m_capacity(0),
      m_unpacked_idx(0)
  {
    reserve(size);
  }

  /// @brief Destructor, deallocates internal buffer
  virtual ~Buffer()
  {
    delete_ptr_array(m_buffer);
  }
  //@}

  /// @brief access to the internal buffer
  const char* buffer() const { return m_buffer; }

  void* buffer() { return (void*)m_buffer; }

  /// @brief Allocated memory
  int capacity() const { return m_capacity; }

  /// @brief Size of valid parts of buffer
  int size() const { return m_size; }

  /// @brief Unpacked memory
  ///
  /// Remains to unpack = packed - unpacked
  int unpacked_idx() const { return m_unpacked_idx; }

  /// @brief Unpacked memory
  ///
  /// Remains to unpack = packed - unpacked
  int& unpacked_idx() { return m_unpacked_idx; }

  /// @brief Tell if everything is unpacked
  bool more_to_unpack() const { return m_unpacked_idx < m_size; }

  /// @brief reserve the buffer to fit memory "size".
  /// The buffer gets allocated bigger than necessary in order to reduce future resizes.
  void reserve(const Uint size);

  /// @brief resize the buffer to fit memory "size".
  /// The buffer gets resized bigger than necessary in order to reduce future resizes.
  /// @post Calling pack() afterwards, will grow the buffer and push_back
  void resize(const Uint size);

  /// @brief reset the buffer, without resizing
  ///
  /// This is useful to reuse an already allocated buffer.
  void reset();

  /// @brief Create a separation, defining different parts of the buffer
  /// This is useful for parallel communication, so that you can avoid the use
  /// of std::vector<PE::Buffer> in e.g. all_to_all communication
  int mark_pid_start();

  std::vector<int>& strides() { return m_strides; }
  std::vector<int>& displs() { return m_displs; }
  const std::vector<int>& strides() const { return m_strides; }
  const std::vector<int>& displs() const { return m_displs; }

  /// @brief begin iterator
  const iterator begin() const { return m_buffer; }

  /// @brief end iterator
  const iterator end() const { return m_buffer+m_size; }

  /// @name Pack/Unpackfunctions for POD arrays
  //@{
  /// @brief Pack data in the buffer. The data must be POD (plain old data).
  /// @param [in] data       data array
  /// @param [in] data_size  number of elements in the array. (data_size=1 in case the data is not an array)
  template <typename T>
      void pack(const T* data, const Uint data_size);

  /// @brief Unpack data in the buffer. The data must be POD (plain old data).
  /// @param [out] data       data array
  /// @param [in]  data_size  number of elements in the array. (data_size=1 in case the data is not an array)
  template <class T>
      void unpack(T* data, const Uint data_size);
  //@}

  /// @name Pack/Unpack functions for POD
  //@{
  void pack(const bool& data);
  void pack(const char& data)               { pack(&data,1); }
  void pack(const unsigned char& data)      { pack(&data,1); }
  void pack(const short& data)              { pack(&data,1); }
  void pack(const unsigned short& data)     { pack(&data,1); }
  void pack(const int& data)                { pack(&data,1); }
  void pack(const unsigned int& data)       { pack(&data,1); }
  void pack(const long& data)               { pack(&data,1); }
  void pack(const unsigned long& data)      { pack(&data,1); }
  void pack(const long long& data)          { pack(&data,1); }
  void pack(const unsigned long long& data) { pack(&data,1); }
  void pack(const float& data)              { pack(&data,1); }
  void pack(const double& data)             { pack(&data,1); }
  void pack(const long double& data)        { pack(&data,1); }
  void pack(const std::string& data);

  void pack(const bool* data);
  void pack(const char* data)               { pack(data,1); }
  void pack(const unsigned char* data)      { pack(data,1); }
  void pack(const short* data)              { pack(data,1); }
  void pack(const unsigned short* data)     { pack(data,1); }
  void pack(const int* data)                { pack(data,1); }
  void pack(const unsigned int* data)       { pack(data,1); }
  void pack(const long* data)               { pack(data,1); }
  void pack(const unsigned long* data)      { pack(data,1); }
  void pack(const long long* data)          { pack(data,1); }
  void pack(const unsigned long long* data) { pack(data,1); }
  void pack(const float* data)              { pack(data,1); }
  void pack(const double* data)             { pack(data,1); }
  void pack(const long double* data)        { pack(data,1); }
  void pack(const std::string* data);

  void unpack(bool& data);
  void unpack(char& data)               { unpack(&data,1); }
  void unpack(unsigned char& data)      { unpack(&data,1); }
  void unpack(short& data)              { unpack(&data,1); }
  void unpack(unsigned short& data)     { unpack(&data,1); }
  void unpack(int& data)                { unpack(&data,1); }
  void unpack(unsigned int& data)       { unpack(&data,1); }
  void unpack(long& data)               { unpack(&data,1); }
  void unpack(unsigned long& data)      { unpack(&data,1); }
  void unpack(long long& data)          { unpack(&data,1); }
  void unpack(unsigned long long& data) { unpack(&data,1); }
  void unpack(float& data)              { unpack(&data,1); }
  void unpack(double& data)             { unpack(&data,1); }
  void unpack(long double& data)        { unpack(&data,1); }
  void unpack(std::string& data);

  void unpack(bool* data);
  void unpack(char* data)               { unpack(data,1); }
  void unpack(unsigned char* data)      { unpack(data,1); }
  void unpack(short* data)              { unpack(data,1); }
  void unpack(unsigned short* data)     { unpack(data,1); }
  void unpack(int* data)                { unpack(data,1); }
  void unpack(unsigned int* data)       { unpack(data,1); }
  void unpack(long* data)               { unpack(data,1); }
  void unpack(unsigned long* data)      { unpack(data,1); }
  void unpack(long long* data)          { unpack(data,1); }
  void unpack(unsigned long long* data) { unpack(data,1); }
  void unpack(float* data)              { unpack(data,1); }
  void unpack(double* data)             { unpack(data,1); }
  void unpack(long double* data)        { unpack(data,1); }
  void unpack(std::string* data);
  //@}

  /// @name Pack/Unpack of some container types
  //@{
  template <typename T>
      void pack(const std::vector<T>& data);

  template <class T>
      void unpack(std::vector<T>& data);

  template <typename T>
      void pack(boost::detail::multi_array::sub_array<T,1> data);

//  template <typename T>
//      void pack(const boost::detail::multi_array::sub_array<T,1>& data);

  template <typename T>
      void pack(const boost::detail::multi_array::const_sub_array<T, 1, T const *>& data);

  template <typename T>
      void unpack(boost::detail::multi_array::sub_array<T,1> data);

  //@}

  /// @name MPI collective operations
  //@{

  /// @brief Broadcast the buffer from the root process.
  ///
  /// The buffer on all receiving ranks gets resized and overwritten
  /// with the buffer from the broadcasting rank.
  /// @param [in] root  The broadcasting rank
  void broadcast(const Uint root);

  /// @brief All Gather collective operation for buffers.
  ///
  /// The received buffer contains all sent buffers from all ranks
  /// @param [out] recv output buffer
  inline void all_gather(Buffer& recv);

  /// @brief All To All collective operation for buffers.
  ///
  /// @pre This buffer needs to have separations inserted for each pid,
  ///      to mark chunks that need to be sent to each pid.
  ///
  /// The received buffer contains those chunks from the sent buffer, that
  /// were marked for the receiving pid.
  /// @param [out] recv output buffer
  inline void all_to_all(Buffer& recv);

  //@}

private:

  /// @brief internal buffer
  char* m_buffer;

  /// @brief allocated memory in buffer
  int m_capacity;

  /// @brief packed memory in buffer
  int m_size;

  /// @brief unpacked memory in buffer
  int m_unpacked_idx;

  /// @brief markers for separation
  std::vector<int> m_displs;
  std::vector<int> m_strides;

};

////////////////////////////////////////////////////////////////////////////////

inline void Buffer::reserve(const Uint size)
{
  if(m_size+static_cast<int>(size) > m_capacity)
  {
    m_capacity = std::max(2*m_capacity,m_size+static_cast<int>(size));
    char* new_buffer = new char[m_capacity];
    if (is_not_null(m_buffer))
    {
      memcpy(new_buffer,m_buffer,m_size);
      delete_ptr_array(m_buffer);
    }
    m_buffer = new_buffer;
  }
}

////////////////////////////////////////////////////////////////////////////////

inline void Buffer::resize(const Uint size)
{
  reserve(size);
  m_size = size;
}

////////////////////////////////////////////////////////////////////////////////

inline void Buffer::reset()
{
  m_size = 0;
  m_unpacked_idx = 0;
  m_strides.clear();
  m_displs.clear();
}

////////////////////////////////////////////////////////////////////////////////

inline int Buffer::mark_pid_start()
{
  m_displs.push_back(m_size);
  m_strides.push_back(0);
  return m_size;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void Buffer::pack(const T* data, const Uint data_size)
{
  if (data_size)
  {
    // get size of the package
    int size;
    MPI_Pack_size(data_size, get_mpi_datatype<T>() , Comm::instance().communicator() , &size);

    // reserve buffer to fit the package
    reserve(size);

    // pack the package in the buffer, and modify the packed_size
    int index = static_cast<int>(m_size);
    MPI_Pack((void*)data, data_size , get_mpi_datatype<T>(), m_buffer, m_capacity, &index, Comm::instance().communicator());
    m_size = index;
    cf3_assert(m_size <= m_capacity);
    if (m_strides.size())
      m_strides.back()=m_size-m_displs.back();
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline void Buffer::unpack(T* data, const Uint data_size)
{
  if (data_size)
  {
    // unpack the package and modify the unpacked_size
    int index=static_cast<int>(m_unpacked_idx);
    MPI_Unpack(m_buffer, m_capacity, &index, (void*)data, data_size, get_mpi_datatype<T>(), Comm::instance().communicator());
    m_unpacked_idx = index;
    cf3_assert(m_unpacked_idx <= m_size);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void Buffer::pack(const std::vector<T>& data)
{
  pack(data.size());
  pack(&data[0],data.size());
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline void Buffer::unpack(std::vector<T>& data)
{
  size_t size;
  unpack(size);
  data.resize(size);
  unpack(&data[0],size);
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void Buffer::pack(boost::detail::multi_array::sub_array<T,1> data)
{
  pack(data.size());
  pack(&data[0],data.size());
}

template <typename T>
inline void Buffer::unpack(boost::detail::multi_array::sub_array<T,1> data)
{
  size_t size;
  unpack(size);
  unpack(&data[0],data.size());
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void Buffer::pack(const boost::detail::multi_array::const_sub_array<T, 1, T const *>& data)
{
  pack(data.size());
  pack(&data[0],data.size());
}

////////////////////////////////////////////////////////////////////////////////

/// pack specialization for bool
template <>
inline void Buffer::pack<bool>(const bool* data, const Uint data_size)
{
  for (Uint i=0; i<data_size; i++)
  {
    pack( data[i] ? 1u : 0u);
  }
}

inline void Buffer::pack(const bool& data)     { pack<bool>(&data,1); }
inline void Buffer::pack(const bool* data)     { pack<bool>(data,1); }

////////////////////////////////////////////////////////////////////////////////

/// unpack specialization for bool
template <>
inline void Buffer::unpack<bool>(bool* data, const Uint data_size)
{
  for (Uint i=0; i<data_size; i++)
  {
    Uint u;
    unpack(u);
    data[i] = (u==1u) ? true : false;
  }
}

inline void Buffer::unpack(bool& data)         { unpack<bool>(&data,1); }
inline void Buffer::unpack(bool* data)         { unpack<bool>(data,1); }

////////////////////////////////////////////////////////////////////////////////

/// pack specialization for std::string
template <>
inline void Buffer::pack<std::string>(const std::string* data, const Uint data_size)
{
  for (Uint i=0; i<data_size; i++)
  {
    Uint len = data[i].size();
    pack(len);
    for (Uint j=0; j<len; ++j)
      pack(data[i][j]);
  }
}

inline void Buffer::pack(const std::string& data)     { pack<std::string>(&data,1); }
inline void Buffer::pack(const std::string* data)     { pack<std::string>(data,1); }

////////////////////////////////////////////////////////////////////////////////

/// unpack specialization for std::string
template <>
inline void Buffer::unpack<std::string>(std::string* data, const Uint data_size)
{
  for (Uint i=0; i<data_size; i++)
  {
    Uint len;
    unpack(len);
    data[i].resize(len);
    for (Uint j=0; j<len; ++j)
      unpack(data[i][j]);
  }
}

inline void Buffer::unpack(std::string& data)         { unpack<std::string>(&data,1); }
inline void Buffer::unpack(std::string* data)         { unpack<std::string>(data,1); }

////////////////////////////////////////////////////////////////////////////////

inline void Buffer::broadcast(const Uint root)
{
  // broadcast buffer size
  int p = m_size;
  MPI_Bcast( &p, 1, get_mpi_datatype(p), root, PE::Comm::instance().communicator() );

  // resize the buffer on receiving ranks
  if (Comm::instance().rank()!=root)
  {
    reserve(p);
    m_size = p;
  }

  // broadcast buffer as MPI_PACKED
  MPI_Bcast( m_buffer, m_size, MPI_PACKED, root, PE::Comm::instance().communicator() );
}

////////////////////////////////////////////////////////////////////////////////

inline void Buffer::all_gather(Buffer& recv)
{
  std::vector<int> strides;
  Comm::instance().all_gather((int)size(),strides);
  std::vector<int> displs(strides.size());
  if (strides.size())
  {
    int sum_strides = strides[0];
    displs[0] = 0;
    for (Uint i=1; i<strides.size(); ++i)
    {
      displs[i] = displs[i-1] + strides[i-1];
      sum_strides += strides[i];
    }
    recv.reset();
    recv.resize(displs.back()+strides.back());
    MPI_CHECK_RESULT(MPI_Allgatherv, (begin(), size(), MPI_PACKED, recv.begin(), &strides[0], &displs[0], MPI_PACKED, Comm::instance().communicator()));
  }
  else
  {
    recv.reset();
  }
}

////////////////////////////////////////////////////////////////////////////////

inline void Buffer::all_to_all(PE::Buffer& recv)
{
  recv.reset();
  Comm::instance().all_to_all(strides(),recv.strides());
  recv.displs().resize(recv.strides().size());
  recv.displs()[0]=0;
  for (Uint pid=1; pid<Comm::instance().size(); ++pid)
    recv.displs()[pid] = recv.displs()[pid-1] + recv.strides()[pid-1];
  recv.resize(recv.displs().back()+recv.strides().back());
  MPI_CHECK_RESULT(MPI_Alltoallv, ((void*)begin(), &strides()[0], &displs()[0], MPI_PACKED, (void*)recv.begin(), &recv.strides()[0], &recv.displs()[0], MPI_PACKED, Comm::instance().communicator()));
}

////////////////////////////////////////////////////////////////////////////////

#define CF3_COMMON_PE_BUFFER_PACK_OPERATOR(TYPE)\
  inline Buffer& operator<< (Buffer& buffer, const TYPE& data)\
  {\
    buffer.pack(data);\
    return buffer;\
  }

#define CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(TYPE)\
  inline Buffer& operator>> (Buffer& buffer, TYPE& data)\
  {\
    buffer.unpack(data);\
    return buffer;\
  }

#define CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(TYPE)\
  inline Buffer& operator<< (Buffer& buffer, const std::vector<TYPE>& data)\
  {\
    buffer.pack(data);\
    return buffer;\
  }

#define CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(TYPE)\
  inline Buffer& operator>> (Buffer& buffer, std::vector<TYPE>& data)\
  {\
    buffer.unpack(data);\
    return buffer;\
  }


#define CF3_COMMON_PE_BUFFER_PACK_OPERATOR_SUB_ARRAY(TYPE)\
  inline Buffer& operator<< (Buffer& buffer, const boost::detail::multi_array::sub_array<TYPE,1>& data)\
  {\
    buffer.pack(data);\
    return buffer;\
  }

#define CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(TYPE)\
  inline Buffer& operator>> (Buffer& buffer, boost::detail::multi_array::sub_array<TYPE,1>& data)\
  {\
    buffer.unpack(data);\
    return buffer;\
  }

#define CF3_COMMON_PE_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(TYPE)\
  inline Buffer& operator<< (Buffer& buffer, const boost::detail::multi_array::const_sub_array<TYPE, 1, TYPE const *>& data)\
  {\
    buffer.pack(data);\
    return buffer;\
  }

CF3_COMMON_PE_BUFFER_PACK_OPERATOR(bool);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(char);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(unsigned char);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(short);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(unsigned short);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(int);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(unsigned int);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(long);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(unsigned long);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(long long);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(unsigned long long);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(float);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(double);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(long double);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR(std::string);

//CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(bool); // Doesn't compile
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(char);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(unsigned char);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(short);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(unsigned short);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(int);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(unsigned int);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(long);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(unsigned long);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(long long);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(unsigned long long);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(float);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(double);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(long double);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR(std::string);

CF3_COMMON_PE_BUFFER_PACK_OPERATOR_SUB_ARRAY(Uint);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_SUB_ARRAY(int);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_SUB_ARRAY(Real);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_SUB_ARRAY(bool);

CF3_COMMON_PE_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(Uint);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(int);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(Real);
CF3_COMMON_PE_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(bool);

CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(bool);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(char);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(unsigned char);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(short);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(unsigned short);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(int);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(unsigned int);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(long);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(unsigned long);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(long long);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(unsigned long long);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(float);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(double);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(long double);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR(std::string);

//CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(bool); // Doesn't compile
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(char);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned char);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(short);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned short);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(int);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned int);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(long);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned long);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(long long);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned long long);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(float);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(double);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(long double);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_VECTOR(std::string);

CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(Uint);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(int);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(Real);
CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(bool);

#undef CF3_COMMON_PE_BUFFER_PACK_OPERATOR
#undef CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR
#undef CF3_COMMON_PE_BUFFER_PACK_OPERATOR_VECTOR
#undef CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR
#undef CF3_COMMON_PE_BUFFER_PACK_OPERATOR_SUB_ARRAY
#undef CF3_COMMON_PE_BUFFER_UNPACK_OPERATOR_SUB_ARRAY
#undef CF3_COMMON_PE_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY

////////////////////////////////////////////////////////////////////////////////

struct PackedObject
{
  PackedObject() {}

  virtual void pack(PE::Buffer& buffer) = 0;
  virtual void unpack(PE::Buffer& buffer) = 0;
};

inline Buffer& operator<< (Buffer& buffer, PackedObject& obj)
{
  obj.pack(buffer);
  return buffer;
}

inline Buffer& operator>> (Buffer& buffer, PackedObject& obj)
{
  obj.unpack(buffer);
  return buffer;
}

////////////////////////////////////////////////////////////////////////////////

inline std::ostream& operator<< (std::ostream& out, const Buffer& buffer)
{
  const char* endPtr = buffer.end();
  for(char* ptr = (char*)buffer.buffer(); (const char*) ptr < endPtr; ptr++)
    out << *ptr;
  return out;
}

////////////////////////////////////////////////////////////////////////////////

} // PE
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_COMMON_PE_Buffer_hpp
