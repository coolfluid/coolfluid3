#ifndef matrix_mult_h
#define matrix_mult_h

#include <cstdlib>

#if defined __APPLE__ || defined(MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

struct CLEnv
{
     cl_context        context;
     cl_command_queue  command_queue;
     cl_program        program;
     cl_kernel         kernel;
     cl_device_id*     devices;
     size_t            device_size;
     cl_int            errcode;
     cl_platform_id    cpPlatform;
};

void opencl_setup( CLEnv& env);
void opencl_mat_mul(CLEnv& env, float* h_A, float* h_B, float* h_C );
void opencl_unsetup( CLEnv& env);

#endif // matrix_mult_h
