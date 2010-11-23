// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_PEOBJECTWRAPPER_HPP
#define CF_Common_MPI_PEOBJECTWRAPPER_HPP

////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <boost/weak_ptr.hpp>

#include <Common/BasicExceptions.hpp>
#include <Common/CodeLocation.hpp>
#include <Common/Component.hpp>

////////////////////////////////////////////////////////////////////////////////

/**
  @file PEObjectWrapper.hpp
  @author Tamas Banyai
  This file provides the wrapper classes of various datatypes which is used within PECommPattern.
  These object wrappers are not designed to be used outside of communication pattern, do not use them directly.
  The layout is that base class PEObjectWrapper is an interface towards PECommPattern and for each data type there is a template child class.
  Currently supports any: raw array, std::vector, boost::multiarray but their template type must be plain old data.
  Note that interface complies to the following relation: size of the data in bytes equal to size_of()*stride()*size().
**/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

/// Base wrapper class serving as interface.
class PEObjectWrapper: public Component{
  public:

    /// constructor
    /// @param name the component will appear under this name
    PEObjectWrapper( const CName& name ) : Component(name)
    {
      BUILD_COMPONENT;
    }

    /// accessor to m_data
    /// @return pointer to the raw data
    virtual const void* data()=0;

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    virtual const int size_of()=0;

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    virtual const int size()=0;

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    virtual const int stride()=0;

  private:

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {};

  protected:

    /// number of elements to be groupped together and treat as once in communication pattern
    int m_stride;
};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for raw ptr arrays allocated by new[]/malloc/calloc.
/// @todo realloc technically passes through, but since the new size is unknown to object wrapper, therefore it will complain.
/// @todo maybe provide boost::shared_array version too.
template<typename T> class PEObjectWrapperPtr: public PEObjectWrapper{

  public:

    /// constructor of passing by reference
    /// @param name the component will appear under this name
    /// @param data pointer to data
    /// @param size length of the data
    /// @param stride number of array element grouping
    PEObjectWrapperPtr(const CName& name, T*& data, const int size, const unsigned int stride=1) : PEObjectWrapper(name)
    {
      m_data=&data;
      m_stride=(int)stride;
      if (size%m_stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
      m_size=size/m_stride;
    }

    /// constructor of passing by pointer
    /// @param name the component will appear under this name
    /// @param data pointer to data
    /// @param size length of the data
    /// @param stride number of array element grouping
    PEObjectWrapperPtr(const CName& name, T** data, const int size, const unsigned int stride=1) : PEObjectWrapper(name)
    {
      m_data=data;
      m_stride=(int)stride;
      if (size%m_stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
      m_size=size/m_stride;
    }

    /// destructor
    ~PEObjectWrapperPtr() { /*delete m_data;*/ };

    /// accessor to the linear memory of the data
    /// @return pointer to the raw data
    const void* data() { return (void*)(*m_data); };

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); };

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    const int size() { return m_size; };

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; };

  private:

    /// holder of the pointer
    T** m_data;

    /// holder of the element size
    int m_size;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for std::vectors.
template<typename T> class PEObjectWrapperVector: public PEObjectWrapper{

  public:

    /// constructor of passing by reference
    /// @param name the component will appear under this name
    /// @param std::vector of data
    /// @param stride number of array element grouping
    PEObjectWrapperVector(const CName& name, std::vector<T>& data, const unsigned int stride=1) : PEObjectWrapper(name)
    {
      m_data=&data;
      m_stride=(int)stride;
      if (data.size()%stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
    }

    /// constructor of passing by pointer
    /// @param name the component will appear under this name
    /// @param std::vector of data
    /// @param stride number of array element grouping
    PEObjectWrapperVector(const CName& name, std::vector<T>* data, const unsigned int stride=1) : PEObjectWrapper(name)
    {
      m_data=*data;
      m_stride=(int)stride;
      if (data.size()%stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
    }

    /// destructor
    ~PEObjectWrapperVector() { /*delete m_data;*/ };

    /// accessor to the linear memory of the data
    /// @return pointer to the raw data
    const void* data() { return (void*)(&(*m_data)[0]); };

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); };

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    const int size() {
      if (m_data->size()%m_stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
      return m_data->size()/m_stride;
    };

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; };

  private:

    /// pointer to std::vector
    std::vector<T>* m_data;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for std::vectors via boost's weak pointer.
template<typename T> class PEObjectWrapperVectorWeakPtr: public PEObjectWrapper{

  public:

    /// constructor
    /// @param name the component will appear under this name
    /// @param std::vector of data
    /// @param stride number of array element grouping
    PEObjectWrapperVectorWeakPtr(const CName& name, boost::weak_ptr< std::vector<T> > data, const unsigned int stride=1) : PEObjectWrapper(name)
    {
      m_data=data;
      m_stride=(int)stride;
      boost::shared_ptr< std::vector<T> > sp=data.lock();
      if (sp->size()%stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
    }

    /// destructor
    ~PEObjectWrapperVectorWeakPtr() { /*delete m_data;*/ };

    /// accessor to the linear memory of the data
    /// @return pointer to the raw data, if pointer is invalid then returns null pointer
    const void* data()
    {
      if (!m_data.expired())
      {
        boost::shared_ptr< std::vector<T> > sp=m_data.lock();
        return (void*)(&(*sp)[0]);
      }
      return nullptr;
    };

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); };

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array, if pointer is invalid then returns zero
    const int size() {
      if (!m_data.expired())
      {
        boost::shared_ptr< std::vector<T> > sp=m_data.lock();
        if (sp->size()%m_stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
        return sp->size()/m_stride;
      }
      return 0;
    };

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; };

  private:

    /// pointer to std::vector
    boost::weak_ptr< std::vector<T> > m_data;

};

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_PEOBJECTWRAPPER_HPP
