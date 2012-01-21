#include <iostream>
#include <fstream>

#include <cstdio>
#include <cmath>

#include <boost/timer.hpp>
#include <boost/program_options.hpp>


#include "matrix_sizes.h"
#include "matrix_mult.h"



// Allocates a matrix with random float entries
void random_init( array_2D &data, int size )
{
    for( int i = 0; i < 5; i++ )
        for (int j = 0; j < size; j++)
      //  data[i] = 10*i;
        data[i][j] = rand() / (float)RAND_MAX;
}


int main(int argc, char * argv[])
{

  // Declare the supported options.
  boost::program_options::options_description desc("allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("file", boost::program_options::value<std::string>() , "naem of the file to create" )
      ("opencl", "run with opencl code")
      ("native", "run with native code");

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count("help")) {
      std::cout << desc << "\n";
      return 1;
  }

  // create matrices -----------------------------------------------------

  /* set seed for rand()*/
  srand(2006);

  unsigned int size_A = WA * HA;
  unsigned int size_B = WB * HB;
  unsigned int size_C = WC * HC;

  array_2D h_A( boost::extents[5][size_A] );
  array_2D h_B( boost::extents[5][size_B] );
  array_2D h_C( boost::extents[5][size_C] );

  /* 2. initialize host memory*/
  random_init(h_A, size_A);
  random_init(h_B, size_B);

  // run with native code ---------------------------------------------------

  if (vm.count("native"))
  {

    boost::timer ntimer;

    cpu_multiplication( h_A, h_B, h_C, 5 );

    printf("[native] time: %6.3f seconds\n", ntimer.elapsed() );

  }

  // run with opencl code -----------------------------------------------------

  if (vm.count("opencl"))
  {
    boost::timer Timer;

    mat_mul(h_A, h_B, h_C, 5 );

    printf("[opencl] time: %6.3f seconds\n", Timer.elapsed() );
  }


  // write result  -------------------------------------------------------

  if (vm.count("file"))
  {
    std::string filename = vm["file"].as<std::string>();
    std::cout << "writing to " << filename << std::endl;
    std::ofstream fout ( filename.c_str() );
    for( int idx = 0; idx < 5; idx++ )
    {
        for( unsigned  int i = 0; i < size_C; i++)
        {
           fout << h_C[idx][i] << " ";
           if(((i + 1) % WC) == 0 ) fout << "\n";
        }
        fout << std::endl;
    }
    fout.close();
  }

  // clean up memory -------------------------------------------------------

  return 0;
}
