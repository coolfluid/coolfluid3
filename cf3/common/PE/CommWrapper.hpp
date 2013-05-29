// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_PEOBJECTWRAPPER_HPP
#define cf3_common_PE_PEOBJECTWRAPPER_HPP

////////////////////////////////////////////////////////////////////////////////

#include <boost/weak_ptr.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_same.hpp>

#include "common/LibCommon.hpp"
#include "common/CF.hpp"
#include "common/Component.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common  {
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
  Does NOT work with bools!
**/

////////////////////////////////////////////////////////////////////////////////

/// Base wrapper class serving as interface.
class Common_API CommWrapper : public Component {

    /// making CommWrapperView as friend in order to access create_view and destroy_view
    template <typename T> friend class CommWrapperView;

  public:

    /// constructor
    /// @param name the component will appear under this name
    CommWrapper( const std::string& name ) : Component(name) {}

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    virtual const void* pack(std::vector<int>& map, void* buf=nullptr) const = 0;

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    virtual const void* pack(void* buf=nullptr) const = 0;

    /// extraction of data from the wrapped object into the specified vector
    /// for technical reason, only sizeof(T)==size_of() and sizeof(T)==1 (byte-based buffer) are allowed to cast to
    /// @param buf reference to std::vector to hold the data
    /// @param map vector of map
    template <typename T> inline void pack(std::vector<T>& buf, std::vector<int>& map) const
    {
      if (sizeof(T)==size_of()) { buf.resize(map.size()*stride()); }
      else if (sizeof(T)==1)    { buf.resize(map.size()*stride()*size_of()); }
      else throw common::BadValue(FromHere(),"Sizeof T is neither size_of() nor one.");
      pack(map,&buf[0]);
    }

    /// extraction of data from the wrapped object into the specified vector
    /// for technical reason, only sizeof(T)==size_of() and sizeof(T)==1 (byte-based buffer) are allowed to cast to
    /// @param buf reference to std::vector to hold the data
    template <typename T> inline void pack(std::vector<T>& buf) const
    {
      if (sizeof(T)==size_of()) { buf.resize(size()*stride()); }
      else if (sizeof(T)==1)    { buf.resize(size()*stride()*size_of()); }
      else throw common::BadValue(FromHere(),"Sizeof T is neither size_of() nor one.");
      pack(&buf[0]);
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf, std::vector<int>& map) const = 0;

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    virtual void unpack(void* buf) const = 0;

    /// returniong back data to the wrapped object into the specified vector
    /// for technical reason, only sizeof(T)==size_of() and sizeof(T)==1 (byte-based buffer) are allowed to cast from
    /// @param buf reference to std::vector to hold the data
    /// @param map vector of map
    template <typename T> inline void unpack(std::vector<T>& buf, std::vector<int>& map) const
    {
      if (sizeof(T)==size_of()) { }
      else if (sizeof(T)==1)    { }
      else throw common::BadValue(FromHere(),"Sizeof T is neither size_of() nor one.");
      unpack(&buf[0],map);
    }

    /// returniong back data to the wrapped object into the specified vector
    /// for technical reason, only sizeof(T)==size_of() and sizeof(T)==1 (byte-based buffer) are allowed to cast from
    /// @param buf reference to std::vector to hold the data
    template <typename T> inline void unpack(std::vector<T>& buf) const
    {
      if (sizeof(T)==size_of()) { }
      else if (sizeof(T)==1)    { }
      else throw common::BadValue(FromHere(),"Sizeof T is neither size_of() nor one.");
      unpack(&buf[0]);
    }

    /// resizes the underlying wrapped object
    /// @param size new dimension size
    virtual void resize(const int size) = 0;

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
    bool needs_update() const { return m_needs_update; }

    /// Get the class name
    static std::string type_name () { return "CommWrapper"; }

  private:

    /// Create an access to the raw data inside the wrapped class.
    /// @warning if underlying raw data is not linear, a copy is being made.
    /// @return pointer to data
    virtual void* start_view() = 0;

    /// Finalizes view to the raw data held by the class wrapped by the commwrapper.
    /// @warning if the underlying data is not linear the data is copied back, therefore performance is degraded
    /// @param data pointer to the data
    virtual void end_view(void* data) = 0;

  protected:

    /// number of elements to be groupped together and treat as once in communication pattern
    int m_stride;

    /// bool holding the info if data to be synchronized & kept up-to-date with commpattern or only keep up-to-date
    bool m_needs_update;

};

////////////////////////////////////////////////////////////////////////////////

/// fancy little class which asks for a view from a commwrapper specified as constructor's argument and frees the data upon destruction
/// @warning Very important: the data is copied back upon calling destructor. This means you end up in trouble if you modify the comwrapper behind.
template <typename T> class Common_API CommWrapperView : public boost::noncopyable
{
  public:

    /// Constructor.
    /// View is only supported if sizeof(T)==cw.size_of() and sizeof(T)==1, otherwisw error is thrown.
    /// @param cw reference to the comwrapper class
    CommWrapperView(const Handle<CommWrapper>& cw)
    {
      if (sizeof(T)==cw->size_of()) { m_size=cw->size()*cw->stride(); }
      else if (sizeof(T)==1)    { m_size=cw->size()*cw->stride()*cw->size_of(); }
      else throw common::BadValue(FromHere(),"Sizeof T is neither size_of() nor one.");
      m_cw=cw;
      m_data=(T*)m_cw->start_view();
    }

    /// Destructor.
    ~CommWrapperView()
    {
      // afterall can call ~CommWrapperView anyway in the code
      m_cw->end_view((void*)m_data);
      m_cw.reset();
      m_data=nullptr;
      m_size=0;
    }

    /// accessor to the naked pointer via accessor function
    /// @warning Very important: the data is copied back upon calling destructor. This means you end up in trouble if you modify the comwrapper behind.
    T* get_ptr() { return m_data; }

    /// accessor to the naked pointer via () operator
    /// @warning Very important: the data is copied back upon calling destructor. This means you end up in trouble if you modify the comwrapper behind.
    T* operator() () { return m_data; }

    /// accessor to the length of the array, measured in items of sizeof(T)
    int size() { return m_size; }

 private:

    /// raw pointer to data
    T* m_data;

    /// size of the array measured based on sizeof(T)
    int m_size;

    /// pointer to commwrapper
    Handle<CommWrapper> m_cw;
};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for raw ptr arrays allocated by new[]/malloc/calloc.
template<typename T> class CommWrapperPtr: public CommWrapper{
  public:

    /// destructor
    ~CommWrapperPtr() { /*delete m_data;*/ }

    /// constructor
    /// @param name the component will appear under this name
    CommWrapperPtr(const std::string& name) : CommWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "CommWrapperPtr<"+common::class_name<T>()+">"; }

    /// setup of passing by reference
    /// @param data pointer to data
    /// @param size length of the data
    /// @param stride number of array element grouping
    void setup(T*& data, const int size, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw cf3::common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=&data;
      m_stride=(int)stride;
      if (size%m_stride!=0) throw cf3::common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_size=size/m_stride;
      m_needs_update=needs_update;
    }

    /// setup of passing by pointer
    /// @param data pointer to data
    /// @param size length of the data
    /// @param stride number of array element grouping
    void setup(T** data, const int size, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw cf3::common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=data;
      m_stride=(int)stride;
      if (size%m_stride!=0) throw cf3::common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_size=size/m_stride;
      m_needs_update=needs_update;
    }

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    const void* pack(std::vector<int>& map, void* buf=nullptr) const
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (buf==nullptr) buf=new T[map.size()*m_stride+1];
      if ( buf == nullptr ) throw cf3::common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*m_data)[0];
      std::vector<int>::iterator imap=map.begin();
      for (T* ibuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          *ibuf++=data[*imap*m_stride + i];
      return buf;
    }

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    const void* pack(void* buf=nullptr) const
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (buf==nullptr) buf=new T[m_size*m_stride+1];
      if ( buf == nullptr ) throw cf3::common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*m_data)[0];
      T* ibuf=(T*)buf;
      for (int i=0; i<(const int)(m_size*m_stride); i++)
        *ibuf++=*data++;
      return buf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    void unpack(void* buf, std::vector<int>& map) const
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      std::vector<int>::iterator imap=map.begin();
      T* data=&(*m_data)[0];
      for (T* ibuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          data[*imap*m_stride + i]=*ibuf++;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    void unpack(void* buf) const
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      T* data=&(*m_data)[0];
      T* ibuf=(T*)buf;
      for (int i=0; i<(const int)(m_size*m_stride); i++)
        *data++=*ibuf++;
    }

    /// resizes the underlying wrapped object
    /// NOT SUPPORTED, because it would require messing with the pointers internally.
    /// If you are forced to use plain dynamic arrays (interface to an old code),
    /// Not even considering that CommWrapperPtr cannot know if new, new[] or malloc, calloc, realloc was used to allocate it.
    /// you need to take care of your own data in order not to loose the pointers outside CommWrapperPtr.
    /// However, if you catch the throw, the integer storing the size gets set to the new value before throw.
    /// @param size new dimension size
    void resize(const int size)
    {
      m_size=size;
      throw common::NotSupported(FromHere(), name() + " (CommWrapperPtr): Resizing a plain dynamic array is forbidden through CommWrapperPtr.");
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    int size_of() const { return sizeof(T); }

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    int size() const { return m_size; }

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
    void* start_view() { return (void*)(*m_data); }

    /// Finalizes view to the raw data held by the class wrapped by the commwrapper.
    /// @warning if the underlying data is not linear the data is copied back, therefore performance is degraded
    /// @param data pointer to the data
    void end_view(void* data) { return; }

  private:

    /// holder of the pointer
    T** m_data;

    /// holder of the element size
    int m_size;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for std::vectors.
template<typename T> class CommWrapperVector: public CommWrapper{

  public:

    /// constructor
    /// @param name the component will appear under this name
    CommWrapperVector(const std::string& name) : CommWrapper(name) {   }

    /// Get the class name
    static std::string type_name () { return "CommWrapperVector<"+common::class_name<T>()+">"; }

    /// setup of passing by reference
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(std::vector<T>& data, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw cf3::common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=&data;
      m_stride=(int)stride;
      if (data.size()%stride!=0) throw cf3::common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_needs_update=needs_update;
    }

    /// setup of passing by pointer
    /// @param std::vector of data
    /// @param stride number of array element grouping
    void setup(std::vector<T>* data, const unsigned int stride, const bool needs_update)
    {
      if (boost::is_pod<T>::value==false) throw cf3::common::BadValue(FromHere(),name()+": Data is not POD (plain old datatype).");
      m_data=*data;
      m_stride=(int)stride;
      if (data->size()%stride!=0) throw cf3::common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_needs_update=needs_update;
    }

    /// destructor
    ~CommWrapperVector() { /*delete m_data;*/ };

    /// extraction of sub-data from data wrapped by the objectwrapper, pattern specified by map
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @param map vector of map
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*map.size()
    const void* pack(std::vector<int>& map, void* buf=nullptr) const
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (buf==nullptr) buf=new T[map.size()*m_stride+1];
      if ( buf == nullptr ) throw cf3::common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      std::vector<int>::iterator imap=map.begin();
      for (T* ibuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          *ibuf++=(*m_data)[*imap*m_stride + i];
      return buf;
    }

    /// extraction of data from the wrapped object, returned memory is a copy, not a view
    /// if nullptr is passed (also default parameter), memory is allocated.
    /// @return pointer to the newly allocated data which is of size size_of()*stride()*size()
    const void* pack(void* buf=nullptr) const
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (buf==nullptr) buf=new T[m_data->size()+1];
      if ( buf == nullptr ) throw cf3::common::NotEnoughMemory(FromHere(),name()+": Could not allocate temporary buffer.");
      T* data=&(*m_data)[0];
      T* ibuf=(T*)buf;
      for (int i=0; i<(const int)(m_data->size()); i++)
        *ibuf++=*data++;
      return buf;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param map vector of map
    /// @param pointer to the data to be committed back
    void unpack(void* buf, std::vector<int>& map) const
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      std::vector<int>::iterator imap=map.begin();
      for (T* ibuf=(T*)buf; imap!=map.end(); imap++)
        for (int i=0; i<(const int)m_stride; i++)
          (*m_data)[*imap*m_stride + i]=*ibuf++;
    }

    /// returning back values into the data wrapped by objectwrapper
    /// @param pointer to the data to be committed back
    void unpack(void* buf) const
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      T* data=&(*m_data)[0];
      T* ibuf=(T*)buf;
      for (int i=0; i<(const int)m_data->size(); i++)
        *data++=*ibuf++;
    }

    /// resizes the underlying wrapped object
    /// @param size new dimension size
    void resize(const int size)
    {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (m_data->size()%m_stride!=0) throw cf3::common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      m_data->resize(size*m_stride);
    }

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    int size_of() const { return sizeof(T); }

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    int size() const {
      if (m_data==nullptr) throw cf3::common::BadPointer(FromHere(),name()+": Data expired.");
      if (m_data->size()%m_stride!=0) throw cf3::common::BadValue(FromHere(),name()+": Nonzero remainder of size()/stride().");
      return m_data->size()/m_stride;
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
    void* start_view() { return (void*)&(*m_data)[0]; }

    /// Finalizes view to the raw data held by the class wrapped by the commwrapper.
    /// @warning if the underlying data is not linear the data is copied back, therefore performance is degraded
    /// @param data pointer to the data
    void end_view(void* data) { return; }

  private:

    /// pointer to std::vector
    std::vector<T>* m_data;

};

////////////////////////////////////////////////////////////////////////////////

} // PE
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_PEOBJECTWRAPPER_HPP
