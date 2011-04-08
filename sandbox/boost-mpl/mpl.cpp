#include <iostream>
#include <algorithm>
#include <vector>

#include <cassert>

#include <boost/bind.hpp>
#include <boost/ref.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#include "boost/mpl/vector_c.hpp"


struct LOLO { std::string name() { return "LOLO"; } };
struct POLO { std::string name() { return "POLO"; } };
struct POPO { std::string name() { return "POPO"; } };

typedef boost::mpl::vector< LOLO, POPO, POLO > Types;

struct OP
{
  OP() { std::cout << "constructor OP()" << std::endl; };
  OP( const OP& ) { std::cout << "copy contructor OP()" << std::endl; };
  ~OP()  { std::cout << "destructop ~OP()" << std::endl; };

  template <typename T>
  void operator() ( T& pt )
  {
    std::cout << pt.name() << std::endl;
  }
};



int main(int argc, char * argv[])
{
   OP mop;

   boost::mpl::for_each< Types >( boost::ref(mop) );

   // -------------------------------------------------

   typedef boost::mpl::vector_c<int,1,2,3> height_t;

   std::vector<int> height;
   boost::mpl::for_each<height_t>(boost::bind(&std::vector<int>::push_back, &height, _1));

   assert(height[0] == 1);
   assert(height[1] == 2);
   assert(height[2] == 3);
}


