#include <cstdlib>

#include "Common/CF.hpp"
#include "Common/Log.hpp"
#include "Common/MemoryAllocatorNormal.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
    namespace Common {

////////////////////////////////////////////////////////////////////////////////

MemoryAllocatorNormal::MemoryAllocatorNormal (MA_Size _initsize)
    : _ptr(0), _size(0)
{
    _ptr=malloc(_initsize);

    if ( _ptr == NULL )
    throw MemoryAllocatorException ( FromHere(), "Memory may have exhausted" );

    _size=_initsize;
}

////////////////////////////////////////////////////////////////////////////////

MemoryAllocatorNormal::~MemoryAllocatorNormal ()
{
    if (_ptr) {
    free(_ptr);
    }
}

////////////////////////////////////////////////////////////////////////////////

MemoryAllocatorNormal::MA_Size MemoryAllocatorNormal::Resize(MA_Size _newsize)
{
/// @todo can resize null pointers (maybe should have smae behavior as MemoryAllocatorMMap)
//  cf_assert ( IsValid() );

  MA_Ptr NewPtr = realloc (_ptr, _newsize);

  if ( _newsize > 0 && NewPtr == NULL )
    throw MemoryAllocatorException ( FromHere(), "Memory may have exhausted" );

  _ptr = NewPtr;
  _size = _newsize;

  return _size;
}

////////////////////////////////////////////////////////////////////////////////

  } // Namespace Common
} // namespace CF
