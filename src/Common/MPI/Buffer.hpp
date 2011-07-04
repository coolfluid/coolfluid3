// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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
#include "Common/MPI/PE.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace mpi{

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
      m_unpacked_size(0),
      m_index(0)
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
  int allocated_size() const { return m_size; }

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

   Uint new_index()
   {
     m_index.push_back((Uint)m_packed_size);
     return m_index.size()-1;
   }

   Buffer& operator[] (const Uint index)
   {
     m_unpacked_size = m_index[index];
     return *this;
   }

   Uint size() const { return m_index.size(); }

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
      void pack(const boost::detail::multi_array::sub_array<T,1>& data);

  template <typename T>
      void pack(const boost::detail::multi_array::const_sub_array<T, 1, T const *>& data);

  const std::vector<Uint> indexes() const { return m_index; }
  std::vector<Uint>& indexes() { return m_index; }

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
    char* tmp = new char[m_size];
    memcpy(tmp,m_buffer,m_packed_size);
    delete_ptr_array(m_buffer);
    m_buffer = tmp;
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
  if (m_index.empty())  new_index();

  // get size of the package
  int size;
  MPI_Pack_size(data_size, get_mpi_datatype<T>() , PE::instance(), &size);

  // resize buffer to fit the package
  resize(size);

  // pack the package in the buffer, and modify the packed_size
  int index = static_cast<int>(m_packed_size);
  MPI_Pack((void*)data, data_size , get_mpi_datatype<T>(), m_buffer, m_size, &index, PE::instance());
  m_packed_size = index;
  cf_assert(m_packed_size <= m_size);
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline void Buffer::unpack(T* data, const Uint data_size)
{
  // unpack the package and modify the unpacked_size
  int index=static_cast<int>(m_unpacked_size);
  MPI_Unpack(m_buffer, m_size, &index, (void*)data, data_size, get_mpi_datatype<T>(), PE::instance());
  m_unpacked_size = index;
  cf_assert(m_unpacked_size <= m_packed_size);
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
inline void Buffer::pack(const boost::detail::multi_array::sub_array<T,1>& data)
{
  pack(data.size());
  pack(&data[0],data.size());
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
    pack( data[i] ? 'T' : 'F');
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
    char c;
    unpack(c);
    data[i] = (c=='T') ? true : false;
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

void Buffer::broadcast(const Uint root)
{
  // broadcast buffer size
  int p = m_packed_size;
  MPI_Bcast( &p, 1, get_mpi_datatype(p), root, PE::instance() );

  // resize the buffer on receiving ranks
  if (PE::instance().rank()!=root)
  {
    resize(p);
    m_packed_size=p;
  }

  // broadcast buffer as MPI_PACKED
  MPI_Bcast( m_buffer, m_packed_size, MPI_PACKED, root, PE::instance() );
}

////////////////////////////////////////////////////////////////////////////////

/// @brief Pack buffer via stream operator
template <typename T>
inline Buffer& operator<< (Buffer& buffer, const T& data)
{
  buffer.pack(data);
  return buffer;
}

/// @brief Unpack buffer via stream operator
template <typename T>
inline Buffer& operator>> (Buffer& buffer, T& data)
{
  buffer.unpack(data);
  return buffer;
}


std::ostream& operator<< (std::ostream& out, const Buffer& buffer)
{
  const char* endPtr = buffer.buffer() + buffer.packed_size();
  for(char* ptr = (char*)buffer.buffer(); (const char*) ptr < endPtr; ptr++)
    out << *ptr;


  return out;
}

////////////////////////////////////////////////////////////////////////////////

} // mpi
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_Buffer_hpp
