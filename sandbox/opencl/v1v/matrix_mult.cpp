#include <cstdio>
#include <sstream>
#include <sys/stat.h>

#include "matrix_matrix_mult.h"

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
                msg << "detected OpenCL error code [" << error_code << "]\n";
                printf("ERROR CODE %d - file %s line %d\n", error_code, file, line);
                throw std::string( msg.str() );
        }
}

void OpenCL_setup( CLEnv& env)
{
     /*****************************************/
     /* Initialize OpenCL */
     /*****************************************/
     clGetPlatformIDs(1, &env.cpPlatform, NULL);
     clGetDeviceIDs(env.cpPlatform, CL_DEVICE_TYPE_GPU, 2, &env.cdDevice, NULL);

     env.context = clCreateContext(0, 1, &env.cdDevice, NULL, NULL, &env.errcode);
     //env.context = clCreateContextFromType(0,  CL_DEVICE_TYPE_GPU,NULL, NULL, &env.errcode);

     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );


     // get the list of GPU devices associated with context
     env.errcode = clGetContextInfo(env.context, CL_CONTEXT_DEVICES, 0, NULL,&env.device_size);
     env.devices = (cl_device_id *) malloc(env.device_size);

     env.errcode |= clGetContextInfo(env.context, CL_CONTEXT_DEVICES, env.device_size, env.devices, NULL);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     //Create a command-queue
     env.command_queue = clCreateCommandQueue(env.context, env.cdDevice, 0, &env.errcode);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

}

void OpenCL_program_setup(CLEnv& env, const char * filename )
{
     // Load and build OpenCL kernel
     char* kernel_source = load_program_source(filename);
     env.program = clCreateProgramWithSource(env.context, 1, (const char**)&kernel_source, NULL, &env.errcode);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     env.errcode = clBuildProgram(env.program, 0,  NULL, NULL, NULL, NULL);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     env.kernel = clCreateKernel(env.program, "matrix_mul", &env.errcode);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     free(kernel_source);
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
  d_A = clCreateBuffer(env.context,
         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         mem_size_A, h_A, &env.errcode);
  d_B = clCreateBuffer(env.context,
         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         mem_size_B, h_B, &env.errcode);
  d_C = clCreateBuffer(env.context,
         CL_MEM_READ_WRITE,
         mem_size_C, NULL, &env.errcode);

  // 7. Launch OpenCL kernel
  size_t localWorkSize[2], globalWorkSize[2];

  int wA = wa;
  int hA = ha;
  int block = n_blocks;
  env.errcode  = clSetKernelArg(env.kernel, 0, sizeof(cl_mem), (void *)&d_C);
  env.errcode |= clSetKernelArg(env.kernel, 1, sizeof(cl_mem), (void *)&d_A);
  env.errcode |= clSetKernelArg(env.kernel, 2, sizeof(cl_mem), (void *)&d_B);
  env.errcode |= clSetKernelArg(env.kernel, 3, sizeof(int),    (void *)&wA);
  env.errcode |= clSetKernelArg(env.kernel, 4, sizeof(int),    (void *)&hA);
  env.errcode |= clSetKernelArg(env.kernel, 5, sizeof(int),    (void *)&wb);
  env.errcode |= clSetKernelArg(env.kernel, 6, sizeof(int),    (void *)&block);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  localWorkSize [0] = wa;
  localWorkSize [1] = wa;
  globalWorkSize[0] = n_blocks;
  globalWorkSize[1] = wb;

  env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  // 8. Retrieve result from device
  env.errcode = clEnqueueReadBuffer(env.command_queue,d_C, CL_TRUE, 0, mem_size_C, h_C, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

}


void matrix_vector_mult(CLEnv& env, float* h_A, float* h_B, float* h_C, int wa, int ha, int n_blocks )
{
  unsigned int size_A = wa * ha;
  unsigned int size_B = ha*n_blocks;
  unsigned int size_C = ha*n_blocks;

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
  d_B = clCreateBuffer(env.context,
         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
         mem_size_B, h_B, &env.errcode);
  d_C = clCreateBuffer(env.context,
         CL_MEM_READ_WRITE,
         mem_size_C, NULL, &env.errcode);

  // 7. Launch OpenCL kernel
  size_t localWorkSize[2], globalWorkSize[2];

  int wA = wa;
  int hA = ha;
  int block = n_blocks;
  env.errcode  = clSetKernelArg(env.kernel, 0, sizeof(cl_mem), (void *)&d_C);
  env.errcode |= clSetKernelArg(env.kernel, 1, sizeof(cl_mem), (void *)&d_A);
  env.errcode |= clSetKernelArg(env.kernel, 2, sizeof(cl_mem), (void *)&d_B);
  env.errcode |= clSetKernelArg(env.kernel, 3, sizeof(int),    (void *)&wA);
  env.errcode |= clSetKernelArg(env.kernel, 4, sizeof(int),    (void *)&hA);
  env.errcode |= clSetKernelArg(env.kernel, 5, sizeof(int),    (void *)&block);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  localWorkSize [0] = wa;
  localWorkSize [1] = wa;
  globalWorkSize[0] = n_blocks;
  globalWorkSize[1] = 1;

  env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  // 8. Retrieve result from device
  env.errcode = clEnqueueReadBuffer(env.command_queue,d_C, CL_TRUE, 0, mem_size_C, h_C, 0, NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

}



void OpenCL_program_unsetup(CLEnv& env)
{
  clReleaseKernel(env.kernel);
  clReleaseProgram(env.program);
}

void OpenCL_unsetup(CLEnv& env)
{
  free(env.devices);
  clReleaseContext(env.context);
  clReleaseCommandQueue(env.command_queue);
}

