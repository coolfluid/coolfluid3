#include <iostream>
#include <boost/progress.hpp>

#include "LogWithProgressDisplay/Log.hpp"
#include "LogWithProgressDisplay/LogStream.hpp"

using namespace CF::Common;

int main(int argc, char * argv[])
{
  CFinfo << "This is a test" << CFendl;

  boost::progress_display progress(10, CFinfo , "     Testing Progress display\n     ", "prog ", "     " );
//  boost::progress_display progress(10, std::cout, "     Testing Progress display\n     ", "prog ", "     " );
  for(unsigned int i=0; i<10; ++i)
    ++progress;

  return 0;
}
