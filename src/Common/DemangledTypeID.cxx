#include "Common/COOLFluiD.hh"

#ifdef CF_HAVE_CXXABI_H
 #include <cxxabi.h>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

std::string demangle(const char* type)
{
  int status = 0;
  char* r = 0;
  
#ifdef CF_HAVE_CXXABI_H
  r = abi::__cxa_demangle(type, 0, 0, &status);
#else // dont have cxxabi.h
  status = 0;
#endif
  
  std::string ret_value;
  if ( (r == 0) || (status != 0) )
    ret_value = std::string(type);
  else 
    ret_value = std::string(r);
  
  free(r);
  
  return ret_value;
}

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace COOLFluiD
