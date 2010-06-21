#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"

using namespace CF::Common;

int main(int argc, char * argv[])
{
  CFinfo << "Welcome to the COOLFLUID K3 solver!\n" << CFflush;

  CoreEnv::instance().initiate(argc, argv);

  CoreEnv::instance().terminate();

  return 0;
}
