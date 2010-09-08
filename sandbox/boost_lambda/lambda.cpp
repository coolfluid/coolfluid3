#include <iostream>
#include <algorithm>
#include <list>

#include <boost/lambda/lambda.hpp>

#include <boost/assign/list_of.hpp> // for 'list_of()'

using namespace boost::assign;
using namespace boost::lambda;

int main(int argc, char * argv[])
{
  std::list<int> primes = list_of(2)(3)(5)(7)(11);

  // print the primes
  std::for_each(primes.begin(), primes.end(), std::cout <<  _1 << ' ' );
  std::cout << std::endl;

  // print the squares of the primes
  std::for_each(primes.begin(), primes.end(), std::cout << (_1) * (_1)  << ' ' );
  std::cout << std::endl;

  // compute the sum
  double sum = 0;
  std::for_each(primes.begin(), primes.end(), var(sum) += _1);
  std::cout << "sum [" << sum << "]\n" << std::flush;
}


