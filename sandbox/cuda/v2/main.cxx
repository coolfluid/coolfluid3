#include <iostream>

#include <cstdlib>
#include <cstdio>
#include <cmath>

#include <boost/timer.hpp>
#include <boost/program_options.hpp>

#include "matrix_sizes.h"
#include "matrix_mult.h"

// Allocates a matrix with random float entries
void random_init(float* data, int size)
{
    for (int i = 0; i < size; ++i)
        data[i] = rand() / (float)RAND_MAX;
}


int main(int argc, char * argv[])
{

  // Declare the supported options.
  boost::program_options::options_description desc("allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("cuda", "run with cuda code")
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

  unsigned int mem_size_A = sizeof(float) * size_A;
  unsigned int mem_size_B = sizeof(float) * size_B;
  unsigned int mem_size_C = sizeof(float) * size_C;

  float* h_A = (float*) malloc(mem_size_A);
  float* h_B = (float*) malloc(mem_size_B);
  float* h_C = (float*) malloc(mem_size_C);

  /* 2. initialize host memory*/
  random_init(h_A, size_A);
  random_init(h_B, size_B);

#if 0
    /* 3. print out A and B*/
    printf("\n\nMatrix A\n");
    for( unsigned int i = 0; i < size_A; i++)
    {
       printf("%f ", h_A[i]);
       if(((i + 1) % WA) == 0)
          printf("\n");
    }

    printf("\n\nMatrix B\n");
    for( unsigned int i = 0; i < size_B; i++)
    {
       printf("%f ", h_B[i]);
       if(((i + 1) % WB) == 0)
          printf("\n");
    }
#endif


  // run with native code ---------------------------------------------------

  if (vm.count("native"))
  {

    boost::timer ntimer;

    for(unsigned int i=0;i< HA;i++)
    {
      for(unsigned int j=0;j< WB;j++)
      {
        h_C[i * WC + j] = 0.0;
        for(unsigned int k=0;k< WA;k++)
        {
          h_C[i * WC + j] += + h_A[i * WA + k] * h_B[k * WB +j];
        }
      }
    }

    printf("[native] time: %6.3f seconds\n", ntimer.elapsed() );

  }

  // run with CUDA code -----------------------------------------------------

  if (vm.count("cuda"))
  {

    boost::timer ctimer;

    gpu_mat_mul(h_A, h_B, h_C);

    printf("[cuda] time: %6.3f seconds\n", ctimer.elapsed() );
  }

  // clean up memory -------------------------------------------------------

#if 0
    /* 6. print out the results */
    printf("\n\nMatrix C (Results)\n");
    for( unsigned  int i = 0; i < size_C; i++)
    {
       printf("%f ", h_C[i]);
       if(((i + 1) % WC) == 0)
          printf("\n");
    }
    printf("\n");
#endif


  free(h_A);
  free(h_B);
  free(h_C);

  return 0;
}
