#ifndef CF_Common_BIGALLOCATOR_HH
#define CF_Common_BIGALLOCATOR_HH

#ifdef CF_HAVE_CONFIG_H
#  include "coolfluid_config.h"
#endif // CF_HAVE_CONFIG_H

#ifdef CF_HAVE_ALLOC_MMAP
#  include "Common/MemoryAllocatorMMap.hh"
#else
#  include "Common/MemoryAllocatorNormal.hh"
#endif // CF_HAVE_ALLOC_MMAP

//////////////////////////////////////////////////////////////////////////////

namespace CF {
    namespace Common  {

//////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_ALLOC_MMAP
  typedef MemoryAllocatorMMap   BigAllocator;
#else
  typedef MemoryAllocatorNormal BigAllocator;
#endif

//////////////////////////////////////////////////////////////////////////////

    } // Common
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_BIGALLOCATOR_HH
