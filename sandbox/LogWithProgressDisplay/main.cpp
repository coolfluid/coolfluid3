#define BOOST_DISABLE_ASSERTS


#include "Log.hpp"
#include "LogStream.hpp"

int main(int argc, char * argv[])
{
  CFinfo << "This is a test" << CFendl;
  return 0;
}
