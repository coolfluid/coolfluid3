// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Buffer.hpp
/// @author Willem Deconinck
/// @brief MPI communication buffer for mixed data types

#ifndef CF_Common_MPI_Buffer_hpp
#define CF_Common_MPI_Buffer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostArray.hpp"
#include "Common/PE/Comm.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
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

  /// @name Constructor/Destructor
  //@{
  /// @brief Constructor
  Buffer(const Uint size = 0u)
    : m_buffer(nullptr),
      m_size(0),
      m_packed_size(0),
      m_unpacked_size(0)
  {
    resize(size);
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
  int size() const { return m_size; }

  /// @brief Packed memory
  int packed_size() const { return m_packed_size; }

  /// @brief Packed memory
  int& packed_size() { return m_packed_size; }

  /// @brief Unpacked memory
  ///
  /// Remains to unpack = packed - unpacked
  int unpacked_size() const { return m_unpacked_size; }

  /// @brief Unpacked memory
  ///
  /// Remains to unpack = packed - unpacked
  int& unpacked_size() { return m_unpacked_size; }

  /// @brief Tell if everything is unpacked
  bool more_to_unpack() const { return m_unpacked_size < m_packed_size; }

  /// @brief resize the buffer to fit memory "size".
  /// The buffer gets resized bigger than necessary in order to reduce future resizes.
  void resize(const Uint size);

  /// @brief reset the buffer, without resizing
  ///
  /// This is useful to reuse an already allocated buffer.
  void reset();

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
  /// @brief Broadcast the buffer from the root process
  /// The buffer on all receiving ranks gets resized and overwritten
  /// with the buffer from the broadcasting rank.
  /// @param [in] root  The broadcasting rank
  void broadcast(const Uint root);
  //@}

private:

  /// @brief internal buffer
  char* m_buffer;

  /// @brief allocated memory in buffer
  int m_size;

  /// @brief packed memory in buffer
  int m_packed_size;

  /// @brief unpacked memory in buffer
  int m_unpacked_size;

  /// @brief index
  std::vector<Uint> m_index;
};

////////////////////////////////////////////////////////////////////////////////

inline void Buffer::resize(const Uint size)
{
  if(m_packed_size+static_cast<int>(size) > m_size)
  {
    m_size = std::max(2*m_size,m_packed_size+static_cast<int>(size));
    char* new_buffer = new char[m_size];
    if (is_not_null(m_buffer))
    {
      memcpy(new_buffer,m_buffer,m_packed_size);
      delete_ptr_array(m_buffer);
    }
    m_buffer = new_buffer;
  }
}

////////////////////////////////////////////////////////////////////////////////

inline void Buffer::reset()
{
  m_packed_size = 0;
  m_unpacked_size = 0;
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

    // resize buffer to fit the package
    resize(size);

    // pack the package in the buffer, and modify the packed_size
    int index = static_cast<int>(m_packed_size);
    MPI_Pack((void*)data, data_size , get_mpi_datatype<T>(), m_buffer, m_size, &index, Comm::instance().communicator());
    m_packed_size = index;
    cf_assert(m_packed_size <= m_size);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline void Buffer::unpack(T* data, const Uint data_size)
{
  if (data_size)
  {
    // unpack the package and modify the unpacked_size
    int index=static_cast<int>(m_unpacked_size);
    MPI_Unpack(m_buffer, m_size, &index, (void*)data, data_size, get_mpi_datatype<T>(), Comm::instance().communicator());
    m_unpacked_size = index;
    cf_assert(m_unpacked_size <= m_packed_size);
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
  int p = m_packed_size;
  MPI_Bcast( &p, 1, get_mpi_datatype(p), root, Comm::instance().communicator() );

  // resize the buffer on receiving ranks
  if (Comm::instance().rank()!=root)
  {
    resize(p);
    m_packed_size=p;
  }

  // broadcast buffer as MPI_PACKED
  MPI_Bcast( m_buffer, m_packed_size, MPI_PACKED, root, Comm::instance().communicator() );
}

////////////////////////////////////////////////////////////////////////////////

#define CF_COMMON_MPI_BUFFER_PACK_OPERATOR(TYPE)\
  inline Buffer& operator<< (Buffer& buffer, const TYPE& data)\
  {\
    buffer.pack(data);\
    return buffer;\
  }

#define CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(TYPE)\
  inline Buffer& operator>> (Buffer& buffer, TYPE& data)\
  {\
    buffer.unpack(data);\
    return buffer;\
  }

#define CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(TYPE)\
  inline Buffer& operator<< (Buffer& buffer, const std::vector<TYPE>& data)\
  {\
    buffer.pack(data);\
    return buffer;\
  }

#define CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(TYPE)\
  inline Buffer& operator>> (Buffer& buffer, std::vector<TYPE>& data)\
  {\
    buffer.unpack(data);\
    return buffer;\
  }


#define CF_COMMON_MPI_BUFFER_PACK_OPERATOR_SUB_ARRAY(TYPE)\
  inline Buffer& operator<< (Buffer& buffer, const boost::detail::multi_array::sub_array<TYPE,1>& data)\
  {\
    buffer.pack(data);\
    return buffer;\
  }

#define CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(TYPE)\
  inline Buffer& operator>> (Buffer& buffer, boost::detail::multi_array::sub_array<TYPE,1>& data)\
  {\
    buffer.unpack(data);\
    return buffer;\
  }

#define CF_COMMON_MPI_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(TYPE)\
  inline Buffer& operator<< (Buffer& buffer, const boost::detail::multi_array::const_sub_array<TYPE, 1, TYPE const *>& data)\
  {\
    buffer.pack(data);\
    return buffer;\
  }

CF_COMMON_MPI_BUFFER_PACK_OPERATOR(bool);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(char);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(unsigned char);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(short);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(unsigned short);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(int);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(unsigned int);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(long);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(unsigned long);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(long long);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(unsigned long long);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(float);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(double);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(long double);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR(std::string);

//CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(bool); // Doesn't compile
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(char);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(unsigned char);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(short);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(unsigned short);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(int);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(unsigned int);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(long);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(unsigned long);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(long long);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(unsigned long long);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(float);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(double);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(long double);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR(std::string);

CF_COMMON_MPI_BUFFER_PACK_OPERATOR_SUB_ARRAY(Uint);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_SUB_ARRAY(int);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_SUB_ARRAY(Real);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_SUB_ARRAY(bool);

CF_COMMON_MPI_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(Uint);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(int);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(Real);
CF_COMMON_MPI_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY(bool);

CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(bool);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(char);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(unsigned char);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(short);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(unsigned short);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(int);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(unsigned int);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(long);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(unsigned long);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(long long);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(unsigned long long);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(float);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(double);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(long double);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR(std::string);

//CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(bool); // Doesn't compile
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(char);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned char);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(short);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned short);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(int);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned int);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(long);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned long);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(long long);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(unsigned long long);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(float);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(double);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(long double);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_VECTOR(std::string);

CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(Uint);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(int);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(Real);
CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_SUB_ARRAY(bool);

#undef CF_COMMON_MPI_BUFFER_PACK_OPERATOR
#undef CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR
#undef CF_COMMON_MPI_BUFFER_PACK_OPERATOR_VECTOR
#undef CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR
#undef CF_COMMON_MPI_BUFFER_PACK_OPERATOR_SUB_ARRAY
#undef CF_COMMON_MPI_BUFFER_UNPACK_OPERATOR_SUB_ARRAY
#undef CF_COMMON_MPI_BUFFER_PACK_OPERATOR_CONST_SUB_ARRAY

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
  const char* endPtr = buffer.buffer() + buffer.packed_size();
  for(char* ptr = (char*)buffer.buffer(); (const char*) ptr < endPtr; ptr++)
    out << *ptr;
  return out;
}

////////////////////////////////////////////////////////////////////////////////

} // PE
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_Buffer_hpp
