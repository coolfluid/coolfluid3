// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_PEOBJECTWRAPPER_HPP
#define CF_Common_MPI_PEOBJECTWRAPPER_HPP

////////////////////////////////////////////////////////////////////////////////

#include <boost/weak_ptr.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_same.hpp>

#include "Common/LibCommon.hpp"
#include "Common/CF.hpp"
#include "Common/Component.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace PE  {

////////////////////////////////////////////////////////////////////////////////

/**
  @file CommWrapper.hpp
  @author Tamas Banyai
  This file provides the wrapper classes of various datatypes which is used within CommPattern.
  These object wrappers are not designed to be used outside of communication pattern, do not use them directly.
  The layout is that base class CommWrapper is an interface towards CommPattern and for each data type there is a template child class.
  Currently supports any: raw array, std::vector, boost::multiarray but their template type must be plain old data.
  Note that interface complies to the following relation: size of the data in bytes equal to size_of()*stride()*size().
**/

////////////////////////////////////////////////////////////////////////////////

/// Base wrapper class serving as interface.
/// @author Tamas Banyai
class Common_API CommWrapper : public Component {

  public:

    /// pointer to this type
    typedef boost::shared_ptr<CommWrapper> Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr<CommWrapper const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    CommWrapper( const std::string& name ) : Component(name) {};

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map) const = 0;

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    virtual const void* pack() const = 0;

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf,std::vector<int>& map) const = 0;

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf) const = 0;

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    virtual int size_of() const = 0;

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    virtual int size() const = 0;

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    virtual int stride() const = 0;

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    virtual bool is_data_type_Uint() const = 0;

    /// accessor to lag telling if wrapped data needs to be synchronized,
    /// if not then it will only be modified if commpattern changes (for example coordinates of a mesh)
    /// @return true or false depending if to be synchronized
    bool needs_update() const { return m_needs_update; };

    /// Get the class name
    static std::string type_name () { return "CommWrapper"; }

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
template<typename T> class CommWrapperPtr: public CommWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< CommWrapperPtr<T> > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< CommWrapperPtr<T> const> ConstPtr;

  public:

    /// destructor
    ~CommWrapperPtr() { /*delete m_data;*/ };

    /// constructor
    /// @param name the component will appear under this name
    CommWrapperPtr(const std::string& name) : CommWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "CommWrapperPtr<"+Common::class_name<T>()+">"; }

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
    virtual const void* pack(std::vector<int>& map) const
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* tbuf=new T[map.size()*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*m_data)[0];
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=tbuf; imap!=map.end(); imap++)
        for (int i=0; i<(int)m_stride; i++)
          *itbuf++=data[*imap*m_stride + i];
      return (void*)tbuf;
    }

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    virtual const void* pack() const
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* tbuf=new T[m_size*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*m_data)[0];
      T* itbuf=tbuf;
      for (int i=0; i<(m_size*m_stride); i++)
        *itbuf++=*data++;
      return (void*)tbuf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf, std::vector<int>& map) const
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      std::vector<int>::iterator imap=map.begin();
      T* data=&(*m_data)[0];
      for (T* itbuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(int)m_stride; i++)
          data[*imap*m_stride + i]=*itbuf++;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf) const
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* data=&(*m_data)[0];
      T* itbuf=(T*)buf;
      for (int i=0; i<(m_size*m_stride); i++)
        *data++=*itbuf++;
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    int size_of() const { return sizeof(T); };

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    int size() const { return m_size; };

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    int stride() const { return m_stride; };

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    bool is_data_type_Uint() const { return boost::is_same<T,Uint>::value; };

  private:

    /// holder of the pointer
    T** m_data;

    /// holder of the element size
    int m_size;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for std::vectors.
/// @author Tamas Banyai
template<typename T> class CommWrapperVector: public CommWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< CommWrapperVector<T> > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< CommWrapperVector<T> const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    CommWrapperVector(const std::string& name) : CommWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "CommWrapperVector<"+Common::class_name<T>()+">"; }

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
    ~CommWrapperVector() { /*delete m_data;*/ };

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map) const
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* tbuf=new T[map.size()*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=tbuf; imap!=map.end(); imap++)
        for (int i=0; i<(int)m_stride; i++)
          *itbuf++=(*m_data)[*imap*m_stride + i];
      return (void*)tbuf;
    }

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    virtual const void* pack() const
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* tbuf=new T[m_data->size()+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*m_data)[0];
      T* itbuf=tbuf;
      for (int i=0; i<(m_data->size()); i++)
        *itbuf++=*data++;
      return (void*)tbuf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf, std::vector<int>& map) const
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(int)m_stride; i++)
          (*m_data)[*imap*m_stride + i]=*itbuf++;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf) const
    {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* data=&(*m_data)[0];
      T* itbuf=(T*)buf;
      for (int i=0; i<m_data->size(); i++)
        *data++=*itbuf++;
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    int size_of() const { return sizeof(T); }

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    int size() const {
      if (m_data==nullptr) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      if (m_data->size()%m_stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      return m_data->size()/m_stride;
    }

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    int stride() const { return m_stride; }

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    bool is_data_type_Uint() const { return boost::is_same<T,Uint>::value; }

  private:

    /// pointer to std::vector
    std::vector<T>* m_data;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for std::vectors via boost's weak pointer.
/// @author Tamas Banyai
template<typename T> class CommWrapperVectorWeakPtr: public CommWrapper{

  public:

    /// pointer to this type
    typedef boost::shared_ptr< CommWrapperVectorWeakPtr<T> > Ptr;
    /// const pointer to this type
    typedef boost::shared_ptr< CommWrapperVectorWeakPtr<T> const> ConstPtr;

  public:

    /// constructor
    /// @param name the component will appear under this name
    CommWrapperVectorWeakPtr(const std::string& name) : CommWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "CommWrapperVectorWeakPtr<"+Common::class_name<T>()+">"; }

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
    ~CommWrapperVectorWeakPtr() { /*delete m_data;*/ };

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map) const
    {
      if (m_data.expired()) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      T* tbuf=new T[map.size()*m_stride+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      boost::shared_ptr< std::vector<T> > sp=m_data.lock();
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=tbuf; imap!=map.end(); imap++)
        for (int i=0; i<(int)m_stride; i++)
          *itbuf++=(*sp)[*imap*m_stride + i];
      return (void*)tbuf;
    }

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    virtual const void* pack() const
    {
      if (m_data.expired()) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      boost::shared_ptr< std::vector<T> > sp=m_data.lock();
      T* tbuf=new T[sp->size()+1];
      if ( tbuf == nullptr ) throw CF::Common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*sp)[0];
      T* itbuf=tbuf;
      for (int i=0; i<sp->size(); i++)
        *itbuf++=*data++;
      return (void*)tbuf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf, std::vector<int>& map) const
    {
      if (m_data.expired()) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      boost::shared_ptr< std::vector<T> > sp=m_data.lock();
      std::vector<int>::iterator imap=map.begin();
      for (T* itbuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(int)m_stride; i++)
          (*sp)[*imap*m_stride + i]=*itbuf++;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf) const
    {
      if (m_data.expired()) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      boost::shared_ptr< std::vector<T> > sp=m_data.lock();
      T* data=&(*sp)[0];
      T* itbuf=(T*)buf;
      for (int i=0; i<sp->size(); i++)
        *data++=*itbuf++;
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    int size_of() const { return sizeof(T); }

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array, if pointer is invalid then returns zero
    int size() const {
      if (m_data.expired()) throw CF::Common::BadPointer(FromHere(),name()+": Data expired.");
      boost::shared_ptr< std::vector<T> > sp=m_data.lock();
      if (sp->size()%m_stride!=0) throw CF::Common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      return sp->size()/m_stride;
    }

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    int stride() const { return m_stride; };

    /// Check for Uint, necessary for cheking type of gid in commpattern
    /// @return true or false depending if registered data's type was Uint or not
    bool is_data_type_Uint() const { return boost::is_same<T,Uint>::value; }

  private:

    /// pointer to std::vector
    boost::weak_ptr< std::vector<T> > m_data;

};

////////////////////////////////////////////////////////////////////////////////

} // PE
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_PEOBJECTWRAPPER_HPP
