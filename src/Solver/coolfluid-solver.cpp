#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "Common/Log.hpp"

int g( int x ) { return x*x; }

struct A {
  int f ( int x ) { return x*x*x; }
};

int main(int argc, char * argv[])
{

  CFinfo << "Welcome to the COOLFLUID K3 solver!\n" << CFendl;

  A a;

  // bind free function
  std::cout << boost::bind( g, 3 )() << std::endl;

  // bind member function
  std::cout << boost::bind( &A::f, a, 2)() << std::endl;

  // bind used with boost::function to create a functor
  boost::function<int()> fix = boost::bind( &A::f, a,  4 );
  std::cout << fix() << std::endl;
}
