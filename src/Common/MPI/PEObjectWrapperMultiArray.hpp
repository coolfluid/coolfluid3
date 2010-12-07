// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_PEObjectWrapperMultiArray_HPP
#define CF_Common_MPI_PEObjectWrapperMultiArray_HPP

////////////////////////////////////////////////////////////////////////////////

#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_same.hpp>

#include "Common/MPI/PEObjectWrapper.hpp"
#include "Common/BoostArray.hpp"
#include "Common/Foreach.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for CTable components
/// @author Willem Deconinck
template <typename T, std::size_t NumDims>
class PEObjectWrapperMultiArray: public PEObjectWrapper
{

    /// constructor
    /// @param name the component will appear under this name
    PEObjectWrapperMultiArray(const std::string& name) : PEObjectWrapper(name) 
    {   
      throw BadValue( FromHere() , "There is no PEObjectWrapper for boost::multi_array with this dimension. Make specialization (see example for dim=1 and dim=2)" );
    }

};

template <typename T>
class PEObjectWrapperMultiArray<T,1>: public PEObjectWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperMultiArray > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperMultiArray const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    PEObjectWrapperMultiArray(const std::string& name) : PEObjectWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "PEObjectWrapperMultiArray<"+class_name<T>()+",1>"; }


    /// setup of passing by reference
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(boost::multi_array<T,1>& data, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw CF::Common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=&data;
      m_stride = 1;
      m_needs_update=needs_update;
    }

    /// destructor
    ~PEObjectWrapperMultiArray() {  }

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map)
    {
      if ( is_null(m_data) ) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      Uint* tbuf=new Uint[map.size()*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      Uint* itbuf=tbuf;

      boost_foreach( int local_idx, map)
      {
        *itbuf++ = (*m_data)[local_idx];
      }
      return (void*)tbuf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual const void unpack(std::vector<int>& map, void* buf)
    {
      if ( is_null(m_data) ) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* itbuf=(T*)buf;
      boost_foreach( int local_idx, map)
      {
        (*m_data)[local_idx] = *itbuf++;
      }
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); }

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    const int size() {
      if ( is_null(m_data) ) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      return m_data->num_elements();
    }

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; }

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    const bool is_data_type_Uint() { return boost::is_same<T,Uint>::value; }

  private:

    /// pointer to std::vector
    boost::multi_array<T,1>* m_data;
};

//////////////////////////////////////////////////////////////////////////////

template <typename T>
class PEObjectWrapperMultiArray<T,2>: public PEObjectWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperMultiArray > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperMultiArray const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    PEObjectWrapperMultiArray(const std::string& name) : PEObjectWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "PEObjectWrapperMultiArray<"+class_name<T>()+",2>"; }

    /// setup of passing by reference
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(boost::multi_array<T,2>& data, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw CF::Common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=&data;
      m_stride = data.shape()[1];
      m_needs_update=needs_update;
    }

    /// destructor
    ~PEObjectWrapperMultiArray() {  }

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map)
    {
      if ( is_null(m_data) ) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      Uint* tbuf=new Uint[map.size()*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      Uint* itbuf=tbuf;

      boost_foreach( int local_idx, map)
      {
        boost_foreach( const T& val, (*m_data)[local_idx])
          *itbuf++ = val;
      }
      return (void*)tbuf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual const void unpack(std::vector<int>& map, void* buf)
    {
      if ( is_null(m_data) ) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* itbuf=(T*)buf;
      boost_foreach( int local_idx, map)
      {
        for (Uint i=0; i<m_stride; ++i)
          (*m_data)[local_idx][i] = *itbuf++;
      }
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); }

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    const int size() {
      if ( is_null(m_data) ) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      return m_data->size();
    }

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; }

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    const bool is_data_type_Uint() { return boost::is_same<T,Uint>::value; }

  private:

    /// pointer to std::vector
    boost::multi_array<T,2>* m_data;
};

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_PEObjectWrapperMultiArray_HPP
