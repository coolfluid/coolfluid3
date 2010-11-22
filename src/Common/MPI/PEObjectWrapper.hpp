// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_PEOBJECTWRAPPER_HPP
#define CF_Common_MPI_PEOBJECTWRAPPER_HPP

////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <boost/shared_ptr.hpp>

#include <Common/BasicExceptions.hpp>
#include <Common/CodeLocation.hpp>

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
class ObjectWrapper{
  public:

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

  protected:

    /// number of elements to be groupped together and treat as once in communication pattern
    int m_stride;
};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for raw ptr arrays allocated by new[]/malloc/calloc.
/// @todo realloc technically passes through, but since the new size is unknown to object wrapper, therefore it will complain.
/// @todo maybe provide boost::shared_array version too.
template<typename T> class ObjectWrapperPtr: public ObjectWrapper{

  public:

    /// constructor
    /// @param data pointer to data
    /// @param size length of the data
    /// @param
    ObjectWrapperPtr(T* data, const int size, const unsigned int stride=1)
    {
      m_data=(void*)(&data);
      m_stride=(int)stride;
      if (size%m_stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
      m_size=size/m_stride;
    }

    /// destructor
    ~ObjectWrapperPtr(){};

    /// accessor to the linear memory of the data
    /// @return pointer to the raw data
    const void* data() { return (void*)(*((T**)m_data)); };

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
    void *m_data;

    /// holder of the element size
    int m_size;

};

////////////////////////////////////////////////////////////////////////////////

/// Wrapper class for raw ptr arrays allocated by new[]/malloc/calloc.
template<typename T> class ObjectWrapperVector: public ObjectWrapperBase{

  public:

    /// constructor
    /// @param data pointer to data
    /// @param size length of the data
    /// @param
    ObjectWrapperVector(std::vector<T> data, const unsigned int stride=1){
      m_data=&data;
      m_stride=(int)stride;
      if (data.size()%stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
    }

    /// destructor
    ~ObjectWrapperVector();

    /// accessor to the linear memory of the data
    /// @return pointer to the raw data
    const void* data() { return (void*)(&(*m_data)[0]); };

    /// acts like a sizeof() operator
    /// @return size of the data members in bytes
    const int size_of() { return sizeof(T); };

    /// accessor to the size of the array (without divided by stride)
    /// @return length of the array
    const int size() {
      if (m_data->size()%stride!=0) throw CF::Common::BadValue(FromHere(),"Nonzero remainder of size()/stride().");
      return (*m_data).size()/m_stride;
    };

    /// accessor to the stride which tells how many array elements count as one  in the communication pattern
    /// @return number of items to be treated as one
    const int stride() { return m_stride; };

  private:

    std::vector<T>* m_data;

};


/*
int main(void){

  // different pointer types
  int *ia=new int[10];
  ObjectWrapperBase *ai=new ObjectWrapperPtr<int>(ia,10);
  double *da=new double[11];
  ObjectWrapperBase *ad=new ObjectWrapperPtr<double>(da,11);

  // different vector template types
  std::vector<int> iv(12,0);
  ObjectWrapperBase *vi=new ObjectWrapperVector<int>(iv,3);
  std::vector<double> dv(13,0.);
  ObjectWrapperBase *vd=new ObjectWrapperVector<double>(dv);

  // output properties
  std::cout << " what | count | itemsize | stride\n";
  std::cout << "---------------------------------\n";
  std::cout << " ai   | " << ai->count() << " | " << ai->size() << " | " << ai->stride() << "\n";
  std::cout << " ad   | " << ad->count() << " | " << ad->size() << " | " << ad->stride() << "\n";
  std::cout << " vi   | " << vi->count() << " | " << vi->size() << " | " << vi->stride() << "\n";
  std::cout << " vd   | " << vd->count() << " | " << vd->size() << " | " << vd->stride() << "\n";

  return 0;
}

*/

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_PEOBJECTWRAPPER_HPP
