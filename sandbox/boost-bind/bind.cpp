#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/assign/list_inserter.hpp> // for 'push_back()'
#include <boost/assign/list_of.hpp>       // for 'list_of()' and 'ref_list_of()'

#include <iostream>

int g( int x ) { return x*x; }

struct A {
  int f ( int x ) { return x*x*x; }
};

int main(int argc, char * argv[])
{
  A a;

  // bind free function
  std::cout << boost::bind( g, 3 )() << std::endl;

  // bind member function
  std::cout << boost::bind( &A::f, a, 2)() << std::endl;

  // bind used with boost::function to create a functor
  boost::function<int()> fix = boost::bind( &A::f, a,  4 );
  std::cout << fix() << std::endl;
}
