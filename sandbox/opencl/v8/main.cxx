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
        data[i] = 1;//rand() / (float)RAND_MAX;
}

void printData( float *data, int size )
{
    for( int i = 0; i < size; i++ )
        std::cout<< data[i] << " ";
        std::cout<< std::endl;

}


int main(int argc, char * argv[])
{
  // create matrices -----------------------------------------------------

  /* set seed for rand()*/
  srand(2006);

  unsigned int size_A = WA * HA;
  unsigned int size_B = WB * HB*N_BLOCKS*VARIABLES;
  unsigned int size_C = WC * HC*N_BLOCKS*VARIABLES;

  float A[size_A];
  float B[size_B];
  float C[size_C];
  
  /* 2. initialize host memory*/
  random_init(A, size_A);
  random_init(B, size_B);

  // run with native code ---------------------------------------------------

  //if (param==1)
  {

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

  }

  // run with opencl code -----------------------------------------------------

  
 
  //if (param==2)
  {
        std::cout<<"OpenCL"<<std::endl;

    CLEnv clenv;
    
    OpenCL_setup(clenv);
//    OpenCL_matrix_vector_basic_setup( clenv );
    OpenCL_matrix_vector_advanced_setup( clenv );
//    OpenCL_matrix_matrix_setup( clenv );
    boost::timer Timer;
//    matrix_matrix_mult(clenv, A, B, C, WA, HA, WB, N_BLOCKS );
    //for( int i = 0; i < 5; i++ )
//    matrix_vector_mult(clenv, A, B, C, WA, HA, N_BLOCKS ); // multiplication of single of variables
    matrix_vector_mult_advanced(clenv, A, B, C, WA, HA, VARIABLES,N_BLOCKS); // multiplication of blocks of variables
    //printData(C,size_C);
    printf("[native] time: %6.4f seconds\n", Timer.elapsed() );
    //printData(C,size_C);
    //printData(C,size_C);
    //OpenCL_program_unsetup(clenv1);
    //printData(A,size_A);
    //printData(B,size_B);
    
    //OpenCL_unsetup(clenv);


  }

 
  // clean up memory -------------------------------------------------------


  return 0;
}
