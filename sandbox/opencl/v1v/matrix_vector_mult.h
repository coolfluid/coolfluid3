#ifndef matrix_vector_mult_h
#define matrix_vector_mult_h

#include <cstdlib>

#if defined __APPLE__ || defined(MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

#include "boost/multi_array.hpp"


struct CLEnv_matrix_vector_mult
{
     cl_context        context;
     cl_command_queue  command_queue;
     cl_program        program;
     cl_kernel         kernel;
     cl_device_id*     devices;
     size_t           device_size;
     cl_int            errcode;
     cl_platform_id    cpPlatform;
     cl_device_id      cdDevice;
};

void opencl_matrix_vector_setup( CLEnv_matrix_vector_mult& env  );
void opencl_matrix_vector_mult(CLEnv_matrix_vector_mult& env, float* h_A, float* h_B, float* h_C );
void opencl_matrix_vector_unsetup( CLEnv_matrix_vector_mult& env);

#endif // matrix_mult_h
