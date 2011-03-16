#include <iostream>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <sys/stat.h>

#include "matrix_matrix_mult.h"

char* opencl_errstring(cl_int err)
{
    switch (err)
    {
       /* case CL_SUCCESS:                          return strdup("Success!");
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
        default:                                  return strdup("Unknown");*/
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

void OpenCL_setup( CLEnv& env)
{
     /*****************************************/
     /* Initialize OpenCL */
     /*****************************************/
     clGetPlatformIDs(1, &env.cpPlatform, NULL);
     clGetDeviceIDs(env.cpPlatform, CL_DEVICE_TYPE_GPU, 1, &env.cdDevice, NULL);
     
     env.context = clCreateContext(0, 1, &env.cdDevice, NULL, NULL, &env.errcode);
     //env.context = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, &env.errcode);
     
     
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
     //std::cout<<env.num_compute_units << std::endl;
}

void OpenCL_program_setup(CLEnv& env, cl_kernel& kernel, const char * filename, const char * programename )
{
     // Load and build OpenCL kernel
     char* kernel_source = load_program_source(filename);
     env.program = clCreateProgramWithSource(env.context, 1, (const char**)&kernel_source, NULL, &env.errcode);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     env.errcode = clBuildProgram(env.program, 0,  NULL, "-cl-fast-relaxed-math", NULL, NULL);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     kernel = clCreateKernel(env.program, programename, &env.errcode);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     free(kernel_source);
     clReleaseProgram(env.program);
}

void OpenCL_matrix_vector_basic_setup( CLEnv& env)
{
     OpenCL_program_setup(env, env.kernel_matrix_vector_basic, "matrix_vector_mult.cl", "matrix_vector_mul"  );
}
void OpenCL_matrix_vector_advanced_setup( CLEnv& env)
{
     OpenCL_program_setup(env, env.kernel_matrix_vector_advanced, "matrix_vector_mult_advanced.cl", "matrix_vector_mult_advanced"  );
}
void OpenCL_matrix_matrix_setup( CLEnv& env)
{
    OpenCL_program_setup(env, env.kernel_matrix_matrix, "matrix_matrix_mult.cl", "matrix_matrix_mul"  );
}



void matrix_matrix_mult(CLEnv& env, float* h_A, float* h_B, float* h_C, int wa, int ha, int wb, int n_blocks )
{
  unsigned int size_A = wa * ha;
  unsigned int size_B = wb * wa*n_blocks;
  unsigned int size_C = wb * wa*n_blocks;

  unsigned int mem_size_A = sizeof(float) * size_A;
  unsigned int mem_size_B = sizeof(float) * size_B;
  unsigned int mem_size_C = sizeof(float) * size_C;

  // OpenCL device memory for matrices
  cl_mem d_A;
  cl_mem d_B;
  cl_mem d_C;

  // Setup device memory
  d_A = clCreateBuffer(env.context, CL_MEM_READ_ONLY, mem_size_A, h_A, &env.errcode );
  d_B = clCreateBuffer(env.context, CL_MEM_READ_ONLY, mem_size_B, h_B, &env.errcode );
  d_C = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, mem_size_C, NULL, &env.errcode);
  env.errcode = clEnqueueWriteBuffer(env.command_queue, d_A, CL_FALSE, 0, mem_size_A, h_A, 0, NULL, NULL);
  env.errcode |= clEnqueueWriteBuffer(env.command_queue, d_B, CL_FALSE, 0, mem_size_B, h_B, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  // 7. Launch OpenCL kernel
  

  int wA = wa;
  int hA = ha;
  int block = n_blocks;
  env.errcode  = clSetKernelArg(env.kernel_matrix_matrix, 0, sizeof(cl_mem), (void *)&d_C);
  env.errcode |= clSetKernelArg(env.kernel_matrix_matrix, 1, sizeof(cl_mem), (void *)&d_A);
  env.errcode |= clSetKernelArg(env.kernel_matrix_matrix, 2, sizeof(cl_mem), (void *)&d_B);
  env.errcode |= clSetKernelArg(env.kernel_matrix_matrix, 3, sizeof(int),    (void *)&wA);
  env.errcode |= clSetKernelArg(env.kernel_matrix_matrix, 4, sizeof(int),    (void *)&hA);
  env.errcode |= clSetKernelArg(env.kernel_matrix_matrix, 5, sizeof(int),    (void *)&wb);
  env.errcode |= clSetKernelArg(env.kernel_matrix_matrix, 6, sizeof(int),    (void *)&block);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  size_t localWorkSize[2];
  size_t globalWorkSize[2];
  
  localWorkSize[0] =ha; 
  localWorkSize[1] = ha;
  
  globalWorkSize[0] = 4*env.num_compute_units;
  globalWorkSize[1] = 4*env.num_compute_units;

  env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel_matrix_matrix, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
  clFinish(env.command_queue);

  // 8. Retrieve result from device
  env.errcode = clEnqueueReadBuffer(env.command_queue,d_C, CL_TRUE, 0, mem_size_C, h_C, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
  clReleaseMemObject(d_A);clReleaseMemObject(d_B);clReleaseMemObject(d_C); 

}


void matrix_vector_mult(CLEnv& env, float* h_A, float* h_B, float* h_C, int wa, int ha, int n_blocks )
{
  unsigned int size_A = wa * ha;
  unsigned int size_B = wa*n_blocks;
  unsigned int size_C = ha*n_blocks;

  unsigned int mem_size_A = sizeof(float) * size_A;
  unsigned int mem_size_B = sizeof(float) * size_B;
  unsigned int mem_size_C = sizeof(float) * size_C;

  // OpenCL device memory for matrices
  cl_mem d_A;
  cl_mem d_B;
  cl_mem d_C;

  // Setup device memory
  d_A = clCreateBuffer(env.context, CL_MEM_READ_ONLY, mem_size_A, h_A, &env.errcode);
  d_B = clCreateBuffer(env.context, CL_MEM_READ_ONLY, mem_size_B, h_B, &env.errcode);
  d_C = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY,                       mem_size_C, NULL, &env.errcode);
  env.errcode = clEnqueueWriteBuffer(env.command_queue, d_A, CL_FALSE, 0, mem_size_A, h_A, 0, NULL, NULL);
  env.errcode |= clEnqueueWriteBuffer(env.command_queue, d_B, CL_FALSE, 0, mem_size_B, h_B, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  // 7. Launch OpenCL kernel
  //size_t localWorkSize[2], globalWorkSize[2];
  
  size_t localWorkSize[2];
  size_t globalWorkSize[2];
  
  localWorkSize[0] =ha; 
  localWorkSize[1] = ha;
  
  globalWorkSize[0] = 4*env.num_compute_units;
  globalWorkSize[1] = 4*env.num_compute_units;
   
  int wA = wa;
  int hA = ha;
  int block = n_blocks;
  env.errcode  = clSetKernelArg(env.kernel_matrix_vector_basic, 0, sizeof(cl_mem), (void *)&d_C);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_basic, 1, sizeof(cl_mem), (void *)&d_A);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_basic, 2, sizeof(cl_mem), (void *)&d_B);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_basic, 3, sizeof(int),    (void *)&wa);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_basic, 4, sizeof(int),    (void *)&hA);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_basic, 5, sizeof(int),    (void *)&block);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel_matrix_vector_basic, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
  clFinish(env.command_queue);

  // 8. Retrieve result from device
  env.errcode = clEnqueueReadBuffer(env.command_queue,d_C, CL_TRUE, 0, mem_size_C, h_C, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
  clReleaseMemObject(d_A);clReleaseMemObject(d_B);clReleaseMemObject(d_C); 

}

void matrix_vector_mult_advanced(CLEnv& env, float* h_A, float* h_B, float* h_C, int wa, int ha, int n_variables, int n_blocks )
{
  unsigned int size_A = wa * ha;
  unsigned int size_B = wa*n_blocks*n_variables;
  unsigned int size_C = ha*n_blocks*n_variables;

  unsigned int mem_size_A = sizeof(float) * size_A;
  unsigned int mem_size_B = sizeof(float) * size_B;
  unsigned int mem_size_C = sizeof(float) * size_C;

  // OpenCL device memory for matrices
  cl_mem d_A;
  cl_mem d_B;
  cl_mem d_C;

  // Setup device memory
  d_A = clCreateBuffer(env.context, CL_MEM_READ_ONLY, mem_size_A, h_A, &env.errcode);
  d_B = clCreateBuffer(env.context, CL_MEM_READ_ONLY, mem_size_B, h_B, &env.errcode);
  d_C = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY,                       mem_size_C, NULL, &env.errcode);
  env.errcode = clEnqueueWriteBuffer(env.command_queue, d_A, CL_FALSE, 0, mem_size_A, h_A, 0, NULL, NULL);
  env.errcode |= clEnqueueWriteBuffer(env.command_queue, d_B, CL_FALSE, 0, mem_size_B, h_B, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  // 7. Launch OpenCL kernel
  //size_t localWorkSize[2], globalWorkSize[2];
  
  int wA = wa;
  int hA = ha;
  int variables = n_variables;
  int block = n_blocks;
  env.errcode  = clSetKernelArg(env.kernel_matrix_vector_advanced, 0, sizeof(cl_mem), (void *)&d_C);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_advanced, 1, sizeof(cl_mem), (void *)&d_A);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_advanced, 2, sizeof(cl_mem), (void *)&d_B);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_advanced, 3, sizeof(int),    (void *)&wa);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_advanced, 4, sizeof(int),    (void *)&hA);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_advanced, 5, sizeof(int),    (void *)&variables);
  env.errcode |= clSetKernelArg(env.kernel_matrix_vector_advanced, 6, sizeof(int),    (void *)&block);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  size_t localWorkSize[2];
  size_t globalWorkSize[2];
  
  localWorkSize[0] = ha; 
  localWorkSize[1] = ha;
  
  globalWorkSize[0] = 4*env.num_compute_units;
  globalWorkSize[1] = 4*env.num_compute_units;
  

  env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel_matrix_vector_advanced, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
  clFinish(env.command_queue);

  // 8. Retrieve result from device
  env.errcode = clEnqueueReadBuffer(env.command_queue,d_C, CL_TRUE, 0, mem_size_C, h_C, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
  clReleaseMemObject(d_A);clReleaseMemObject(d_B);clReleaseMemObject(d_C); 

}



void OpenCL_program_unsetup(CLEnv& env)
{
  
}

void OpenCL_unsetup(CLEnv& env)
{
  clReleaseKernel(env.kernel);
  //clReleaseProgram(env.program);
  clReleaseContext(env.context);
  clReleaseCommandQueue(env.command_queue);
}


