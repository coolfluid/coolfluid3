#include <sstream>

#include "Common/OSystemLayer.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::OSystemLayer()
{
}

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::~OSystemLayer()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string OSystemLayer::memoryUsage () const
{
  const CF::Real bytes = memoryUsageBytes();

  std::ostringstream out;
  if (  bytes/1024 <= 1 ) {
  out << bytes << " B";
  }
  else if (bytes/1024/1024 <= 1 ) {
    out << bytes/1024 << " KB";
  }
  else if (bytes/1024/1024/1024 <= 1 ) {
    out << bytes/1024/1024 << " MB";
  }
  else {
    out << bytes/1024/1024/1024 << " GB";
  }
  return out.str();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

