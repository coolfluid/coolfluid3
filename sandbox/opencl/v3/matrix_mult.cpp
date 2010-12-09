#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <cassert>

#include "matrix_sizes.h"
#include "matrix_mult.h"

char* opencl_errstring(cl_int err)
{
    switch (err)
    {
        case CL_SUCCESS:                          return strdup("Success!");
        case CL_DEVICE_NOT_FOUND:                 return strdup("Device not found.");
        case CL_DEVICE_NOT_AVAILABLE:             return strdup("Device not available");
        case CL_COMPILER_NOT_AVAILABLE:           return strdup("Compiler not available");
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return strdup("Memory object allocation failure");
        case CL_OUT_OF_RESOURCES:                 return strdup("Out of resources");
        case CL_OUT_OF_HOST_MEMORY:               return strdup("Out of host memory");
        case CL_PROFILING_INFO_NOT_AVAILABLE:     return strdup("Profiling information not available");
        case CL_MEM_COPY_OVERLAP:                 return strdup("Memory copy overlap");
        case CL_IMAGE_FORMAT_MISMATCH:            return strdup("Image format mismatch");
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return strdup("Image format not supported");
        case CL_BUILD_PROGRAM_FAILURE:            return strdup("Program build failure");
        case CL_MAP_FAILURE:                      return strdup("Map failure");
        case CL_INVALID_VALUE:                    return strdup("Invalid value");
        case CL_INVALID_DEVICE_TYPE:              return strdup("Invalid device type");
        case CL_INVALID_PLATFORM:                 return strdup("Invalid platform");
        case CL_INVALID_DEVICE:                   return strdup("Invalid device");
        case CL_INVALID_CONTEXT:                  return strdup("Invalid context");
        case CL_INVALID_QUEUE_PROPERTIES:         return strdup("Invalid queue properties");
        case CL_INVALID_COMMAND_QUEUE:            return strdup("Invalid command queue");
        case CL_INVALID_HOST_PTR:                 return strdup("Invalid host pointer");
        case CL_INVALID_MEM_OBJECT:               return strdup("Invalid memory object");
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return strdup("Invalid image format descriptor");
        case CL_INVALID_IMAGE_SIZE:               return strdup("Invalid image size");
        case CL_INVALID_SAMPLER:                  return strdup("Invalid sampler");
        case CL_INVALID_BINARY:                   return strdup("Invalid binary");
        case CL_INVALID_BUILD_OPTIONS:            return strdup("Invalid build options");
        case CL_INVALID_PROGRAM:                  return strdup("Invalid program");
        case CL_INVALID_PROGRAM_EXECUTABLE:       return strdup("Invalid program executable");
        case CL_INVALID_KERNEL_NAME:              return strdup("Invalid kernel name");
        case CL_INVALID_KERNEL_DEFINITION:        return strdup("Invalid kernel definition");
        case CL_INVALID_KERNEL:                   return strdup("Invalid kernel");
        case CL_INVALID_ARG_INDEX:                return strdup("Invalid argument index");
        case CL_INVALID_ARG_VALUE:                return strdup("Invalid argument value");
        case CL_INVALID_ARG_SIZE:                 return strdup("Invalid argument size");
        case CL_INVALID_KERNEL_ARGS:              return strdup("Invalid kernel arguments");
        case CL_INVALID_WORK_DIMENSION:           return strdup("Invalid work dimension");
        case CL_INVALID_WORK_GROUP_SIZE:          return strdup("Invalid work group size");
        case CL_INVALID_WORK_ITEM_SIZE:           return strdup("Invalid work item size");
        case CL_INVALID_GLOBAL_OFFSET:            return strdup("Invalid global offset");
        case CL_INVALID_EVENT_WAIT_LIST:          return strdup("Invalid event wait list");
        case CL_INVALID_EVENT:                    return strdup("Invalid event");
        case CL_INVALID_OPERATION:                return strdup("Invalid operation");
        case CL_INVALID_GL_OBJECT:                return strdup("Invalid OpenGL object");
        case CL_INVALID_BUFFER_SIZE:              return strdup("Invalid buffer size");
        case CL_INVALID_MIP_LEVEL:                return strdup("Invalid mip-map level");
        default:                                  return strdup("Unknown");
    }
}

char * load_program_source(const char *filename)
{
	struct stat statbuf;
	FILE *fh;
	char *source;

	fh = fopen(filename, "r");
	if (fh == 0)
		return 0;

	stat(filename, &statbuf);
	source = (char *) malloc(statbuf.st_size + 1);
	fread(source, statbuf.st_size, 1, fh);
	source[statbuf.st_size] = '\0';

	return source;
}

void opencl_check_error(cl_int& error_code, cl_int accept_code, const char * file, int line)
{
	if (error_code != accept_code)
	{
		std::stringstream msg;
		msg << "OPENCL ERROR CODE [" << error_code << "] : " << opencl_errstring(error_code) << "\n";
		msg << "at file " << file << " +" << line << "\n";
		std::cerr << msg.str() << std::endl;
		throw std::string( msg.str() );
	}
}

void opencl_setup(CLEnv& env)
{
    /*****************************************/
    /* Initialize OpenCL */
    /*****************************************/
    clGetPlatformIDs(1, &env.cpPlatform, NULL);
    clGetDeviceIDs(env.cpPlatform, CL_DEVICE_TYPE_GPU, 1, &env.devices[0], NULL);

    env.context = clCreateContext(0, 1, &env.devices[0], NULL, NULL, &env.errcode);


    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    // get the list of GPU devices associated with context
    env.errcode = clGetContextInfo(env.context, CL_CONTEXT_DEVICES, 0, NULL,&env.device_size);
    env.devices = (cl_device_id *) malloc(env.device_size);

    env.errcode |= clGetContextInfo(env.context, CL_CONTEXT_DEVICES, env.device_size, env.devices, NULL);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    //Create a command-queue
    env.command_queue = clCreateCommandQueue(env.context, env.devices[0], 0, &env.errcode);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    // Load and build OpenCL kernel
    const char * filename = "kernel.cl";
    char* kernel_source = load_program_source(filename);
    env.program = clCreateProgramWithSource(env.context, 1, (const char**)&kernel_source, NULL, &env.errcode);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    env.errcode = clBuildProgram(env.program, 0,  NULL, NULL, NULL, NULL);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    env.kernel = clCreateKernel(env.program, "matrix_mul", &env.errcode);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    free(kernel_source);

}

void opencl_mat_mul(CLEnv& env, float* h_A, float* h_B, float* h_C )
{
  unsigned int size_A = WA * HA;
  unsigned int size_B = WB * HB;
  unsigned int size_C = WC * HC;

  unsigned int mem_size_A = sizeof(float) * size_A;
  unsigned int mem_size_B = sizeof(float) * size_B;
  unsigned int mem_size_C = sizeof(float) * size_C;

  // OpenCL device memory for matrices
  cl_mem d_A;
  cl_mem d_B;
  cl_mem d_C;

  // Setup device memory
  d_A = clCreateBuffer(env.context,
         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         mem_size_A, h_A, &env.errcode);
  assert(env.errcode == CL_SUCCESS);

  d_B = clCreateBuffer(env.context,
         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         mem_size_B, h_B, &env.errcode);
  assert(env.errcode == CL_SUCCESS);

  d_C = clCreateBuffer(env.context,
         CL_MEM_READ_WRITE,
         mem_size_C, NULL, &env.errcode);
  assert(env.errcode == CL_SUCCESS);

  // 7. Launch OpenCL kernel
  size_t localWorkSize[2], globalWorkSize[2];

  int wA = WA;
  int wC = WC;
  env.errcode  = clSetKernelArg(env.kernel, 0, sizeof(cl_mem), (void *)&d_C);
  env.errcode |= clSetKernelArg(env.kernel, 1, sizeof(cl_mem), (void *)&d_A);
  env.errcode |= clSetKernelArg(env.kernel, 2, sizeof(cl_mem), (void *)&d_B);
  env.errcode |= clSetKernelArg(env.kernel, 3, sizeof(int),    (void *)&wA);
  env.errcode |= clSetKernelArg(env.kernel, 4, sizeof(int),    (void *)&wC);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  localWorkSize [0] = 16;
  localWorkSize [1] = 16;
  globalWorkSize[0] = HA;
  globalWorkSize[1] = WB;

  env.errcode = clEnqueueNDRangeKernel(env.command_queue,
                                   env.kernel, 2, NULL, globalWorkSize,
                                   localWorkSize, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  // 8. Retrieve result from device
  env.errcode = clEnqueueReadBuffer(env.command_queue,
                                d_C, CL_TRUE, 0, mem_size_C,
                                h_C, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

}

void opencl_unsetup(CLEnv& env)
{
  free(env.devices);
  clReleaseContext(env.context);
  clReleaseKernel(env.kernel);
  clReleaseProgram(env.program);
  clReleaseCommandQueue(env.command_queue);
}
