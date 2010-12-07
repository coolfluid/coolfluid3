// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_PEOBJECTWRAPPER_HPP
#define CF_Common_MPI_PEOBJECTWRAPPER_HPP

////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <boost/weak_ptr.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_same.hpp>

#include "Common/LibCommon.hpp"
#include "Common/CF.hpp"
#include "Common/Component.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {

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

/// Base wrapper class serving as interface.
/// @author Tamas Banyai
class Common_API PEObjectWrapper : public Component {

  public:

    /// pointer to this type
    typedef boost::shared_ptr<PEObjectWrapper> Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr<PEObjectWrapper const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    PEObjectWrapper( const std::string& name ) : Component(name) {};

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map) = 0;

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual const void unpack(std::vector<int>& map, void* buf) = 0;

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    virtual const int size_of() = 0;

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    virtual const int size() = 0;

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    virtual const int stride() = 0;

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    virtual const bool is_data_type_Uint() = 0;

    /// accessor to lag telling if wrapped data needs to be synchronized,
    /// if not then it will only be modified if commpattern changes (for example coordinates of a mesh)
    /// @return true or false depending if to be synchronized
    const bool needs_update() { return m_needs_update; };

    /// Get the class name
    static std::string type_name () { return "PEObjectWrapper"; }

  protected:

    /// number of elements to be groupped together and treat as once in communication pattern
    int m_stride;

    /// bool holding the info if data to be synchronized & kept up-to-date with commpattern or only keep up-to-date
    bool m_needs_update;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for raw ptr arrays allocated by new[]/malloc/calloc.
/// @author Tamas Banyai
/// @todo realloc technically passes through, but since the new size is unknown to object wrapper, therefore it will complain.
/// @todo maybe provide boost::shared_array version too.
template<typename T> class PEObjectWrapperPtr: public PEObjectWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperPtr<T> > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperPtr<T> const> ConstPtr;

  public:

    /// destructor
    ~PEObjectWrapperPtr() { /*delete m_data;*/ };

    /// constructor
    /// @param name the component will appear under this name
    PEObjectWrapperPtr(const std::string& name) : PEObjectWrapper(name) {   }

    /// setup of passing by reference
    /// @param data pointer to data
    /// @param size length of the data
    /// @param stride number of array element grouping
    void setup(T*& data, const int size, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw CF::Common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=&data;
      m_stride=(int)stride;
      if (size%m_stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_size=size/m_stride;
      m_needs_update=needs_update;
    }

    /// setup of passing by pointer
    /// @param data pointer to data
    /// @param size length of the data
    /// @param stride number of array element grouping
    void setup(T** data, const int size, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw CF::Common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=data;
      m_stride=(int)stride;
      if (size%m_stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_size=size/m_stride;
      m_needs_update=needs_update;
    }

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map)
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* tbuf=new T[map.size()*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*m_data)[0];
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=tbuf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          *itbuf++=data[*imap*m_stride + i];
      return (void*)tbuf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual const void unpack(std::vector<int>& map, void* buf)
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      std::vector<int>::iterator imap=map.begin();
      T* data=&(*m_data)[0];
      for (T* itbuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          data[*imap*m_stride + i]=*itbuf++;
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); };

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    const int size() { return m_size; };

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; };

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    const bool is_data_type_Uint() { return boost::is_same<T,Uint>::value; };

  private:

    /// holder of the pointer
    T** m_data;

    /// holder of the element size
    int m_size;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for std::vectors.
/// @author Tamas Banyai
template<typename T> class PEObjectWrapperVector: public PEObjectWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperVector<T> > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperVector<T> const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    PEObjectWrapperVector(const std::string& name) : PEObjectWrapper(name) {   }

    /// setup of passing by reference
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(std::vector<T>& data, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw CF::Common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=&data;
      m_stride=(int)stride;
      if (data.size()%stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_needs_update=needs_update;
    }

    /// setup of passing by pointer
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(std::vector<T>* data, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw CF::Common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=*data;
      m_stride=(int)stride;
      if (data->size()%stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_needs_update=needs_update;
    }

    /// destructor
    ~PEObjectWrapperVector() { /*delete m_data;*/ };

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map)
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* tbuf=new T[map.size()*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*m_data)[0];
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=tbuf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          *itbuf++=data[*imap*m_stride + i];
      return (void*)tbuf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual const void unpack(std::vector<int>& map, void* buf)
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* data=&(*m_data)[0];
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          data[*imap*m_stride + i]=*itbuf++;
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); };

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    const int size() {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      if (m_data->size()%m_stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      return m_data->size()/m_stride;
    };

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; };

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    const bool is_data_type_Uint() { return boost::is_same<T,Uint>::value; };

  private:

    /// pointer to std::vector
    std::vector<T>* m_data;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for std::vectors via boost's weak pointer.
/// @author Tamas Banyai
template<typename T> class PEObjectWrapperVectorWeakPtr: public PEObjectWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperVectorWeakPtr<T> > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< PEObjectWrapperVectorWeakPtr<T> const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    PEObjectWrapperVectorWeakPtr(const std::string& name) : PEObjectWrapper(name) {   }

    /// setup
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(boost::weak_ptr< std::vector<T> > data, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw CF::Common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=data;
      m_stride=(int)stride;
      boost::shared_ptr< std::vector<T> > sp=data.lock();
      if (sp->size()%stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_needs_update=needs_update;
    }

    /// destructor
    ~PEObjectWrapperVectorWeakPtr() { /*delete m_data;*/ };

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map)
    {
      if (m_data.expired()) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* tbuf=new T[map.size()*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      boost::shared_ptr< std::vector<T> > sp=m_data.lock();
      T* data=&(*sp)[0];
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=tbuf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          *itbuf++=data[*imap*m_stride + i];
      return (void*)tbuf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual const void unpack(std::vector<int>& map, void* buf)
    {
      if (m_data.expired()) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      boost::shared_ptr< std::vector<T> > sp=m_data.lock();
      T* data=&(*sp)[0];
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          data[*imap*m_stride + i]=*itbuf++;
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); };

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array, if pointer is invalid then returns zero
    const int size() {
      if (m_data.expired()) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      boost::shared_ptr< std::vector<T> > sp=m_data.lock();
      if (sp->size()%m_stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      return sp->size()/m_stride;
    };

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; };

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    const bool is_data_type_Uint() { return boost::is_same<T,Uint>::value; };

  private:

    /// pointer to std::vector
    boost::weak_ptr< std::vector<T> > m_data;

};

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_PEOBJECTWRAPPER_HPP
