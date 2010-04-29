#ifndef MEM_ALLOC_MMAP_hpp
#define MEM_ALLOC_MMAP_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/MemoryAllocator.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

    namespace Common {

////////////////////////////////////////////////////////////////////////////////

class  Common_API  MemoryAllocatorMMap : public MemoryAllocator {
private:

  MA_Ptr DataPtr;
  MA_Size CurrentSize;
  int FileDesc;


  void Alloc (MA_Size size);
  void Free ();

public:
  /// Constructor:
  /// InitialSize: size in bytes
  /// set InitialSize to -1 to let the implementation choose
  ///  (mostly Granularity)
  MemoryAllocatorMMap (MA_Size InitialSize = 0);

  /// virtual destructor
  virtual ~MemoryAllocatorMMap();

  /// Returns the size of the allocated memory
  virtual MA_Size GetSize () const
  {
    return CurrentSize;
  }

  /// Returns a pointer to the allocated memory
  virtual MA_Ptr GetPtr () const
  {
    return DataPtr;
  }

  /// Resize the allocated memory
  virtual MA_Size Resize (MA_Size NewSize);

  /// Returns true if the object points to valid memory
  bool IsValid () const;

  /// Determines the overhead of the resize operation
  bool IsZeroCopy () const
  {
    return true;
  }

  /// Returns the granularity
  virtual MA_Size GetGranularity () const;

  /// Prevent copy
  MemoryAllocatorMMap (const MemoryAllocator & M);
  MemoryAllocatorMMap& operator =(const MemoryAllocator & M);

  /// easy access
  operator bool () const
  {
    return IsValid();
  }

  operator MA_Ptr () const
  {
    return GetPtr ();
  }

}; // end class MemoryAllocatorMMap

////////////////////////////////////////////////////////////////////////////////

  } //namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // MEM_ALLOC_MMAP_hpp
