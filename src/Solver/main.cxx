#include <iostream>

#include "Common/DummyCommon.hh"
#include "Math/DummyMath.hh"

using namespace COOLFluiD::Common;
using namespace COOLFluiD::Math;

int main(int argc, char * argv[])
{
 DummyCommon dc;
 DummyMath dm;
    
 std::cout << "The string is : " << dc.getString() << std::endl;
 std::cout << "The answer is : " << dm.getAnswer() << std::endl;
    
 return 0;
}