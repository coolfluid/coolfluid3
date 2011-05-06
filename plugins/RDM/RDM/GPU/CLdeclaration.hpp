#ifndef RDM_GPU_CLEnv
#define RDM_GPU_CLEnv

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
     cl_device_id      cdDevice;
     cl_event 	       ceEvent;
     cl_uint           num_compute_units;
};

void opencl_check_error(cl_int& error_code, cl_int accept_code, const char * file, int line);

void GPGPU_setup( CLEnv& env );

#endif // !RDM_GPU_CLEnv
