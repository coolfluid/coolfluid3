#ifndef CF_Common_BIGALLOCATOR_hpp
#define CF_Common_BIGALLOCATOR_hpp

#ifdef CF_HAVE_CONFIG_H
#  include "coolfluid_config.h"
#endif // CF_HAVE_CONFIG_H

#ifdef CF_HAVE_ALLOC_MMAP
#  include "Common/MemoryAllocatorMMap.hpp"
#else
#  include "Common/MemoryAllocatorNormal.hpp"
#endif // CF_HAVE_ALLOC_MMAP

////////////////////////////////////////////////////////////////////////////////

namespace CF {
    namespace Common  {

////////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_ALLOC_MMAP
  typedef MemoryAllocatorMMap   BigAllocator;
#else
  typedef MemoryAllocatorNormal BigAllocator;
#endif

////////////////////////////////////////////////////////////////////////////////

    } // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_BIGALLOCATOR_hpp
