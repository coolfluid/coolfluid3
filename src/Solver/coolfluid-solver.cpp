#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"

using namespace CF::Common;

int main(int argc, char * argv[])
{
  CFinfo << "Welcome to the COOLFLUID K3 solver!\n" << CFflush;

  CoreEnv::getInstance().initiate(argc, argv);

  CoreEnv::getInstance().terminate();

  return 0;
}
