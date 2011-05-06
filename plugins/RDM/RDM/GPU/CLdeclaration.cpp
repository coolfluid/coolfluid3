#include <string>
#include <sstream>

#include "RDM/GPU/CLdeclaration.hpp"

using namespace std;

void opencl_check_error(cl_int& error_code, cl_int accept_code, const char * file, int line)
{
        if (error_code != accept_code)
        {
                std::stringstream msg;
                msg << "detected OpenCL error code [" << error_code << "]\n";
                msg << "ERROR CODE " << error_code << " - file " << file<< " line " << line << "\n";
                throw std::string( msg.str() );
        }
}

void GPGPU_setup( CLEnv& env )
{
    // OpenCL Init

    clGetPlatformIDs(1, &env.cpPlatform, NULL);
    clGetDeviceIDs(env.cpPlatform, CL_DEVICE_TYPE_GPU, 1, &env.cdDevice, NULL);
    env.context = clCreateContext(0, 1, &env.cdDevice, NULL, NULL, &env.errcode);


    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    // get the list of GPU devices associated with context
    env.errcode = clGetContextInfo(env.context, CL_CONTEXT_DEVICES, 0, NULL,&env.device_size);
    env.devices = (cl_device_id *) malloc(env.device_size);

    env.errcode |= clGetContextInfo(env.context, CL_CONTEXT_DEVICES, env.device_size, env.devices, NULL);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    //Create a command-queue
    env.command_queue = clCreateCommandQueue(env.context, env.cdDevice, 0, &env.errcode);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    clGetDeviceInfo(env.cdDevice, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(env.num_compute_units), &env.num_compute_units, NULL);
}
