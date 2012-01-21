#include <iostream>
#include <fstream>
#include <vector>

#include <cstdio>
#include <cmath>

#include "matrix_sizes.h"
#include <sys/time.h>

#include "viennacl/ocl/backend.hpp"
//#include "viennacl/scalar.hpp"
#include "viennacl/vector.hpp"

// Allocates a matrix with random float entries
void random_init( std::vector<float>& vec )
{
    for( int i = 0; i < vec.size(); i++ )
        vec[i] = i;
}

void printData( std::vector<float>& vec )
{
    for( int i = 0; i < vec.size(); i++ )
        std::cout<< vec[i] << " ";
        std::cout<< std::endl;
}



class Timer
{
public:

	Timer() : ts(0)
	{}

	void start()
	{
		struct timeval tval;
		gettimeofday(&tval, NULL);
		ts = tval.tv_sec * 1000000 + tval.tv_usec;
	}

	double get() const
	{
		struct timeval tval;
		gettimeofday(&tval, NULL);
		int64_t end_time = tval.tv_sec * 1000000 + tval.tv_usec;

		return static_cast<double>(end_time-ts) / 1000000.0;
	}

private:
	int64_t ts;
};

#include"openCL_functions.h"

int main(int argc, char * argv[])
{
  unsigned int size_A = WA * HA;
  unsigned int size_B = WB * HB * N_BLOCKS * N_VARIABLES;
  unsigned int size_C = WC * HC * N_BLOCKS * N_VARIABLES;

  std::vector<float> A(size_A);
  std::vector<float> B(size_B);
  std::vector<float> C(size_C);
  std::vector<float> Ac(WA);
  int computeUnits  = viennacl::ocl::current_device().compute_units();
  random_init(A);
  random_init(B);
  


  viennacl::vector<float> vcl_A(size_A);
  viennacl::vector<float> vcl_B(size_B);
  viennacl::vector<float> vcl_C(size_C);
  viennacl::vector<float> vcl_rA(HA); viennacl::vector<float> vcl_rB(HB);
  viennacl::vector<float> vcl_cA(WA); viennacl::vector<float> vcl_cB(WB);

  unsigned int wA = WA;
  unsigned int wB = WB;
  unsigned int hA = HA;
  unsigned int nBlocks = N_BLOCKS;
  unsigned int nVariables = N_VARIABLES;

  
  
  viennacl::ocl::program & prog_matrix_vector_mult = viennacl::ocl::current_context().add_program(own_matrix_vector_mult, "own_matrix_vector_mult");
  viennacl::ocl::kernel & kernel_matrix_vector_mult = prog_matrix_vector_mult.add_kernel("matrix_vector_mult");
  kernel_matrix_vector_mult.local_work_size(0, HA);
  kernel_matrix_vector_mult.global_work_size(0,4 * computeUnits * HA );
  Timer Timer;
  Timer.start();
  fast_copy(A, vcl_A);
  fast_copy(B, vcl_B);
  viennacl::ocl::enqueue( kernel_matrix_vector_mult( vcl_A, vcl_B, vcl_C, wA, hA, nBlocks ) );
  
    
  fast_copy(vcl_C,C);
  printf("[native] time: %6.4f seconds\n", Timer.get() ); 
 // printData(C);

  viennacl::ocl::program & prog_matrix_matrix_mult = viennacl::ocl::current_context().add_program(own_matrix_matrix_mult, "own_matrix_matrix_mult");
  viennacl::ocl::kernel & kernel_matrix_matrix_mult = prog_matrix_matrix_mult.add_kernel("matrix_matrix_mult");
  kernel_matrix_matrix_mult.local_work_size(0, 1);
  kernel_matrix_matrix_mult.global_work_size(0, 2);
  viennacl::ocl::enqueue( kernel_matrix_matrix_mult( vcl_A, vcl_B, vcl_C, wA, hA, wB, nBlocks ) );
  
  
  fast_copy(vcl_C,C);
  //printData(A);
  //printData(B);
  //printData(C);

  viennacl::ocl::program & prog_matrix_vector_global = viennacl::ocl::current_context().add_program(own_matrix_vector_global, "own_matrix_vector_global");
  viennacl::ocl::kernel & kernel_matrix_vector_global = prog_matrix_vector_global.add_kernel("matrix_vector_global");
  kernel_matrix_vector_global.local_work_size(0, 1);
  kernel_matrix_vector_global.global_work_size(0, 2);

  viennacl::ocl::enqueue( kernel_matrix_vector_global( vcl_A, vcl_B, vcl_C, wA, hA, nBlocks, nVariables ) );
  fast_copy(vcl_C,C);
  //printData(C);

  return 0;
}
