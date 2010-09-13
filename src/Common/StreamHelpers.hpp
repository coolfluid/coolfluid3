#ifndef CF_Common_StreamHelpers_hpp
#define CF_Common_StreamHelpers_hpp

#include <Common/CF.hpp>

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
  
////////////////////////////////////////////////////////////////////////////////

// Some common functions to help outputtung data to streams

/// Print a vector enclosed in prefix and suffix, separated with the given separator
template<typename VectorT, typename StreamT>
void print_vector(StreamT& stream, const VectorT& vector, const std::string& sep=" ", const std::string& prefix = "", const std::string& suffix = "")
{
  stream << prefix;
  const Uint vector_size = vector.size();
  for(Uint i = 0; i != vector_size; ++i)
  {
    stream << (i != 0 ? sep : "") << vector[i];
  }
  stream << suffix;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_StreamHelpers_hpp
