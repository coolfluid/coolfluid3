#include <iostream>
#include <fstream>

#include <cstdio>
#include <cmath>

#include <boost/timer.hpp>

#include "matrix_sizes.h"
#include "matrix_matrix_mult.h"

// Allocates a matrix with random float entries
void random_init( float *data, int size )
{
    for( int i = 0; i < size; i++ )
        data[i] = i;//rand() / (float)RAND_MAX;
}

void printData( float *data, int size )
{
    for( int i = 0; i < size; i++ )
        std::cout<< data[i] << " ";
        std::cout<< std::endl;

}


int main(int argc, char * argv[])
{
  /*std::cout<<"Choose the platform for computation:"<<std::endl;
  std::cout<<"===================================="<<std::endl;
  std::cout<<"      1. Native CPU                 "<<std::endl;
  std::cout<<"      2. openCl GPGPU               "<<std::endl;

  int param;

  std::cin>>param;
  std::cout<<"===================================="<<std::endl;
  */



  // create matrices -----------------------------------------------------

  /* set seed for rand()*/
  srand(2006);

  unsigned int size_A = WA * HA;
  unsigned int size_B = WB * HB*N_BLOCKS;
  unsigned int size_C = WC * HC*N_BLOCKS;

  float A[size_A];
  float B[size_B];
  float C[size_C];

  /* 2. initialize host memory*/
  random_init(A, size_A);
  random_init(B, size_B);

  // run with native code ---------------------------------------------------

  //if (param==1)
/*  {

    boost::timer ntimer;

    for( int idx = 0; idx < N_BLOCKS; idx++ )
    {
          for(unsigned int j=0;j< HC;j++)
          {
            C[idx * HC + j] = 0.0;
            for(unsigned int k=0;k< WA;k++)
            {
              C[idx * HC + j] += + A[j * WA + k] * B[idx * WB +k];
            }
          }
    }

    printf("[native] time: %6.3f seconds\n", ntimer.elapsed() );

  }*/

  // run with opencl code -----------------------------------------------------



  //if (param==2)
  {
    std::cout<<"OpenCL"<<std::endl;

    CLEnv clenv;
    CLEnv clenv1;
    OpenCL_setup(clenv); OpenCL_program_setup(clenv, "matrix_matrix_mult.cl" );
    OpenCL_setup(clenv1); OpenCL_program_setup(clenv1, "matrix_vector_mult.cl" );
    boost::timer ctimer;



    //matrix_matrix_mult(clenv, A, B, C, WA, HA, WB, N_BLOCKS );
    matrix_vector_mult(clenv1, A, B, C, WA, HA, N_BLOCKS );
    OpenCL_program_unsetup(clenv);
    OpenCL_program_unsetup(clenv1);
    printf("[opencl] time: %6.3f seconds\n", ctimer.elapsed() );


    OpenCL_unsetup(clenv);
    OpenCL_unsetup(clenv1);
    printData(A,size_A);
    printData(B,size_B);
    printData(C,size_C);



  }

  // clean up memory -------------------------------------------------------


  return 0;
}
