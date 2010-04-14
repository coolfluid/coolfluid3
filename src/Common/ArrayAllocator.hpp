#ifndef ARRAYALLOCATOR_HH
#define ARRAYALLOCATOR_HH

///
/// ArrayAllocator
///
///  TODO:
///    - Use templates to add automagic padding?
///    - Implement removal of points (hint: free list inside the nodes)
///    - Make object-safe. ?? Is this needed?
///       --> Seems like Meshreader relies on it!! (strange)


#include "Common/BigAllocator.hh"
#include "Common/Obj_Helper.hh"
#include "Common/CFLog.hh"

namespace CF {
    namespace Common {


template <class T, typename ALLOC = BigAllocator>
class ArrayAllocator
{
public:

  typedef unsigned int IndexType;


  /// Constructor
  ///   It allocates InitialSize elements of ElementSize bytes
  inline ArrayAllocator (const T & init, IndexType InitialSize,
    unsigned int ElementSize = sizeof (T));

  /// destructor
  inline ~ArrayAllocator ();

  ///   It allocates InitialSize elements of ElementSize bytes
  void initialize (const T & init, IndexType InitialSize,
                   unsigned int ElementSize = sizeof (T));

  ///   Clean up some local data built during initialization
  void cleanup();

  /// Return the allocated size in number of elements
  /// (not in number of bytes!)
  /// This means the index is allowed from 0 .. GetSize()-1
  inline IndexType GetSize () const;

  /// Resize the array
  ///   !!! This can change MOVE the elements in memory!!!!
  ///   (but you're not supposed to apply the &-operator to
  ///   an element of this array)
  void Resize (IndexType NewSize);

  /// Grow the array by a number of elements
  /// (at least 1)
  /// The new number of elements is selected in function of
  /// resizing cost and granularity
      void Grow ();

  /// Access to the array
  /// !!! Don't take the adress of an element!!!
  inline T & operator[] (IndexType index);

  /// Access to the array
  /// !!! Don't take the adress of an element!!!
  inline const T & operator[] (IndexType index) const;


  /// Try to guess the index of the element pointed to
  IndexType PointerToIndex (const void * Ptr) const;

  /// For valarray compatibility
  inline size_t size () const;

  /// For valarray compatibility
  inline void resize (size_t NewSize);


private:

  /// Disallow copy
  ArrayAllocator (const ArrayAllocator & A);

  /// Disallow assignment
  ArrayAllocator & operator = (const ArrayAllocator & A);

private:
    ALLOC MemAlloc;
    IndexType CurrentSize;

    void * cf_restrict DataPtr;
    unsigned int ElementSize;

    inline bool CheckIndex(IndexType I) const;
    inline unsigned int ByteIndex(const IndexType I) const;
    inline T * PointerToIndex (const IndexType I) const;
};


///  Implementation
///***************************************************************************/

/// For valarray compatibility
template <class T, typename A>
inline size_t ArrayAllocator<T,A>::size () const
{
    return GetSize();
}

/// For valarray compatibility
template <class T, typename A>
inline void ArrayAllocator<T,A>::resize (size_t NewSize)
{
    Resize (NewSize);
}


template <class T, typename A>
inline unsigned int ArrayAllocator<T,A>::ByteIndex(const IndexType I) const
{
    cf_assert(I<CurrentSize);
    cf_assert(DataPtr != CFNULL);
    return (I*ElementSize);
}


template <class T, typename A>
inline T * ArrayAllocator<T,A>::PointerToIndex (const IndexType I) const
{
    return (T *) (static_cast<char *>(DataPtr) + ByteIndex(I));
}

template <class T, typename A>
inline T & ArrayAllocator<T,A>::operator[] (IndexType index)
{
    CheckIndex(index);
    return *(T *) (static_cast<char *>(DataPtr)+ByteIndex(index));
}

template <class T, typename A>
inline const T & ArrayAllocator<T,A>::operator[] (IndexType index) const
{
    CheckIndex(index);
    return *(T*)((T *)((char *) (DataPtr)+ByteIndex(index)));
}


template <class T, typename A>
typename ArrayAllocator<T,A>::IndexType ArrayAllocator<T,A>::PointerToIndex
                                    (const void * Ptr) const
{
    // We have to cast to avoid pointer arithmetic corrections
    const char * BasePtr = reinterpret_cast<const char *>(DataPtr);
    const char * ElePtr = reinterpret_cast<const char *>(Ptr);

    if (ElePtr < BasePtr)
  return IndexType(-1);

    unsigned long Diff = ElePtr - BasePtr;
    if (Diff % ElementSize)
  return static_cast<typename ArrayAllocator<T,A>::Indextype>(-1);

    Diff /= ElementSize;
    if (Diff >= GetSize())
  return static_cast<IndexType>(-1);

    return IndexType(Diff);
}

template<class T, typename A>
inline bool ArrayAllocator<T,A>::CheckIndex(IndexType I) const
{
    //TODO
    cf_assert(I<GetSize());
    return true;
}


template<class T, typename A>
void ArrayAllocator<T,A>::Grow ()
{
    IndexType NewSize;
    const unsigned int MinGrow = 1;  // Expand at least this much elements
    unsigned int Grow = MemAlloc.GetGranularity () / ElementSize;
    Grow = (Grow > MinGrow ? Grow : MinGrow);

    cf_assert (Grow != 0);

    if (MemAlloc.IsZeroCopy ())
      NewSize=CurrentSize+Grow;
    else
            // If our current size is zero,
      // we grow at least one element and at most
      // up to the granularity
      NewSize=(CurrentSize ? 2*CurrentSize : Grow);

    cf_assert (NewSize > CurrentSize);

    Resize(NewSize);
}

/// Return the capacity (NUMBER OF ELEMENTS!)
template<class T, typename A>
inline
typename ArrayAllocator<T,A>::IndexType
ArrayAllocator<T,A>::GetSize () const
{
  return CurrentSize;
}

template<class T, typename A>
void ArrayAllocator<T,A>::Resize (IndexType count)
{
    IndexType OldSize = CurrentSize;

    // count is newsize, CurrentSize is current size
    if ((count < CurrentSize) &&
      (!Obj_Helper<T>::IsFundamental ()))
    {
  for (IndexType I = count; I<CurrentSize; I++)
      Obj_Helper<T>::destruct(PointerToIndex(I));
    }

    CFLogDebugMin( "Resizing ArrayAllocator " << (void*) this
      << " from " << OldSize << " to " << count << " ("
      << count*ElementSize << " bytes)\n");

    //
    // !!! Resizing can move the POINTER !!!
    //
    MemAlloc.Resize (count*ElementSize);
    DataPtr=static_cast<void *> (MemAlloc.GetPtr ());
    cf_assert (!count || DataPtr);

    CurrentSize=count;

    //
    // For object safety
    //
    for (IndexType I = OldSize; I<CurrentSize; I++)
  Obj_Helper<T>::construct( PointerToIndex (I) );
}

/*template<class T, typename ALLOC>
ArrayAllocator<T,ALLOC>::ArrayAllocator (IndexType InitialSize, unsigned int
  ESize)
  : CurrentSize (0), DataPtr(0), ElementSize (ESize)
{
    // Keep it sane
    cf_assert (ElementSize >= sizeof (T));

    Resize(InitialSize);
};*/

template <class T, typename ALLOC>
ArrayAllocator<T,ALLOC>::ArrayAllocator (const T & Init,
           IndexType InitSize,
           unsigned int ESize)
  : CurrentSize (0), DataPtr(0), ElementSize(ESize)
{
  initialize(Init, InitSize, ESize);
}

template <class T, typename ALLOC>
void ArrayAllocator<T,ALLOC>::initialize (const T & Init,
      IndexType InitSize,
      unsigned int ESize)
{
  // reset the value of ElementSize
  ElementSize = ESize;

  // cf_assert (ElementSize >= sizeof (T));
  if (ElementSize >= sizeof (T)) {
    Resize (InitSize);
    cf_assert (GetSize () >= InitSize);

    // Init
    for (IndexType i=0; i < GetSize (); i++)
      Obj_Helper<T>::construct ( PointerToIndex(i), Init );

    CFLogDebugMin( "ArrayAllocator: init: size=" << InitSize
    << ", ElementSize=" << ElementSize << ", type-size="
    << sizeof (T) << "\n");
  }
}

template <class T, typename ALLOC>
void ArrayAllocator<T,ALLOC>::cleanup()
{
  // This becomes an empty (hopefully optimized away)
  // loop in the case of primitive types
  for (IndexType I = 0; I < GetSize (); I++)
    Obj_Helper<T>::destruct ( PointerToIndex(I) );
}

template<class T, typename A>
ArrayAllocator<T,A>::~ArrayAllocator ()
{
  cleanup();
}

} // namespace Common

} // CF
#endif
