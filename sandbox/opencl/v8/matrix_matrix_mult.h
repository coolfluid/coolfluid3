#ifndef matrix_mult_h
#define matrix_mult_h

#include <cstdlib>

#if defined __APPLE__ || defined(MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

#include "boost/multi_array.hpp"


struct CLEnv
{
     cl_context        context;
     cl_command_queue  command_queue;
     cl_program        program;
     cl_kernel         kernel_matrix_vector_basic;
     cl_kernel         kernel_matrix_vector_advanced;
     cl_kernel         kernel_matrix_matrix;
     cl_kernel         kernel;
     cl_device_id*     devices;
     size_t           device_size;
     cl_int            errcode;
     cl_platform_id    cpPlatform;
     cl_device_id      cdDevice;
     cl_event 	       ceEvent;
     cl_uint           num_compute_units;
};
void OpenCL_setup( CLEnv& env );
void OpenCL_program_setup( CLEnv& env, cl_kernel& kernel, const char * filename, const char * programename );
void OpenCL_matrix_vector_basic_setup( CLEnv& env);
void OpenCL_matrix_vector_advanced_setup( CLEnv& env);
void OpenCL_matrix_matrix_setup( CLEnv& env);


void matrix_matrix_mult(CLEnv& env, float* h_A, float* h_B, float* h_C, int wa, int ha, int wb, int n_blocks );
void matrix_vector_mult(CLEnv& env, float* h_A, float* h_B, float* h_C , int wa, int ha, int n_blocks);
void matrix_vector_mult_advanced(CLEnv& env, float* h_A, float* h_B, float* h_C , int wa, int ha, int n_variables, int n_blocks);


void OpenCL_unsetup(CLEnv& env);
void OpenCL_program_unsetup(CLEnv& env);



#endif // matrix_mult_h
