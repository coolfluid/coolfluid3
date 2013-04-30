// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_CommWrapperMArray_HPP
#define cf3_common_PE_CommWrapperMArray_HPP

////////////////////////////////////////////////////////////////////////////////

#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_same.hpp>

#include "common/PE/CommWrapper.hpp"
#include "common/BoostArray.hpp"
#include "common/Foreach.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common  {
namespace PE  {

////////////////////////////////////////////////////////////////////////////////

/**
  @file CommWrapperMArray.hpp CommWrapper implementations for accepting boost::multi_array<T,1> and boost::multi_array<T,2>.
  @author Willem Deconinck
**/

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for Table components
template <typename T, std::size_t NumDims>
class CommWrapperMArray: public CommWrapper
{

    /// constructor
    /// @param name the component will appear under this name
    CommWrapperMArray(const std::string& name) : CommWrapper(name)
    {
      throw BadValue( FromHere() , "There is no CommWrapper for boost::multi_array with this dimension. Make specialization (see example for dim=1 and dim=2)" );
    }

};

template <typename T>
class CommWrapperMArray<T,1>: public CommWrapper{
  public:

    /// constructor
    /// @param name the component will appear under this name
    CommWrapperMArray(const std::string& name) : CommWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "CommWrapperMArray<"+common::class_name<T>()+",1>"; }


    /// setup of passing by reference
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(boost::multi_array<T,1>& data, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw cf3::common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=&data;
      m_stride = 1;
      m_needs_update=needs_update;
    }

    /// destructor
    ~CommWrapperMArray() {  }

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map, void* buf=nullptr) const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (buf==nullptr) buf=new T[map.size()*m_stride+1];
      if ( buf == nullptr ) throw cf3::common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* ibuf=(T*)buf;
      boost_foreach( int local_idx, map)
        *ibuf++ = (*m_data)[local_idx];
      return buf;
    }

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    virtual const void* pack(void* buf=nullptr) const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (buf==nullptr) buf=new T[m_data->num_elements()*m_stride+1];
      if ( buf == nullptr ) throw cf3::common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* ibuf=(T*)buf;
      for (int i=0; i<(const int)(m_data->num_elements()*m_stride); i++)
        *ibuf++=(*m_data)[i];
      return buf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf, std::vector<int>& map) const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      T* ibuf=(T*)buf;
      boost_foreach( int local_idx, map)
        (*m_data)[local_idx] = *ibuf++;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf) const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      T* ibuf=(T*)buf;
      for (int i=0; i<(const int)(m_data->num_elements()*m_stride); i++)
        (*m_data)[i]=*ibuf++;
    }

    /// resizes the underlying wrapped object
    /// @param size new dimension size
    void resize(const int size)
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      m_data->resize(boost::extents[size]);
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    int size_of() const { return sizeof(T); }

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    int size() const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      return m_data->num_elements();
    }

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    int stride() const { return m_stride; }

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    bool is_data_type_Uint() const { return boost::is_same<T,Uint>::value; }

  private:

    /// Create an access to the raw data inside the wrapped class.
    /// @warning if underlying raw data is not linear, a copy is being made.
    /// @return pointer to data
    void* start_view()
    {
      return (void*)&(*m_data)[0];
    }

    /// Finalizes view to the raw data held by the class wrapped by the commwrapper.
    /// @warning if the underlying data is not linear the data is copied back, therefore performance is degraded
    /// @param data pointer to the data
    void end_view(void* data) { return; }

  private:

    /// pointer to std::vector
    boost::multi_array<T,1>* m_data;
};

//////////////////////////////////////////////////////////////////////////////

template <typename T>
class CommWrapperMArray<T,2>: public CommWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< CommWrapperMArray > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< CommWrapperMArray const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    CommWrapperMArray(const std::string& name) : CommWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "CommWrapperMArray<"+common::class_name<T>()+",2>"; }

    /// setup of passing by reference
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(boost::multi_array<T,2>& data, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw cf3::common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=&data;
      m_stride = data.shape()[1];
      m_needs_update=needs_update;
    }

    /// destructor
    ~CommWrapperMArray() {  }

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map, void* buf=nullptr) const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (buf==nullptr) buf=new T[map.size()*m_stride+1];
      if ( buf == nullptr ) throw cf3::common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* ibuf=(T*)buf;
      boost_foreach( int local_idx, map)
      {
        cf3_assert(local_idx<m_data->size());
        boost_foreach( const T& val, (*m_data)[local_idx])
          *ibuf++ = val;
      }
      return buf;
    }

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    virtual const void* pack(void* buf=nullptr) const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (buf==nullptr) buf=new T[m_data->num_elements()*m_stride+1];
      if ( buf == nullptr ) throw cf3::common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* ibuf=(T*)buf;
      for (int i=0; i<(const int)m_data->size(); i++)
        for (int j=0; j<(const int)m_stride; j++)
          *ibuf++=(*m_data)[i][j];
      return buf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf, std::vector<int>& map) const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      T* ibuf=(T*)buf;
      boost_foreach( int local_idx, map)
      {
        for (int i=0; i<(const int)m_stride; ++i)
          (*m_data)[local_idx][i] = *ibuf++;
      }
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf) const
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      T* ibuf=(T*)buf;
      for (int i=0; i<(const int)m_data->size(); i++)
        for (int j=0; j<(const int)m_stride; j++)
          (*m_data)[i][j]=*ibuf++;
    }

    /// resizes the underlying wrapped object
    /// @param size new dimension size
    void resize(const int size)
    {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      m_data->resize(boost::extents[size][m_stride]);
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    int size_of() const { return sizeof(T); }

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    int size() const {
      if ( is_null(m_data) ) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      return m_data->size();
    }

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    int stride() const { return m_stride; }

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    bool is_data_type_Uint() const { return boost::is_same<T,Uint>::value; }

  private:

    /// Create an access to the raw data inside the wrapped class.
    /// @warning if underlying raw data is not linear, a copy is being made.
    /// @return pointer to data
    void* start_view()
    {
      return (void*)&(*m_data)[0][0];
    }

    /// Finalizes view to the raw data held by the class wrapped by the commwrapper.
    /// @warning if the underlying data is not linear the data is copied back, therefore performance is degraded
    /// @param data pointer to the data
    void end_view(void* data) { return; }

  private:

    /// pointer to std::vector
    boost::multi_array<T,2>* m_data;
};

////////////////////////////////////////////////////////////////////////////////

} // PE
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_CommWrapperMArray_HPP
