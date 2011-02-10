#include <iostream>
#include <fstream>
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
  d_A = clCreateBuffer(env.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size_A, h_A, &env.errcode );
  d_B = clCreateBuffer(env.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size_B, h_B, &env.errcode );
  d_C = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, mem_size_C, NULL, &env.errcode);

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
  d_A = clCreateBuffer(env.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size_A, h_A, &env.errcode);
  d_B = clCreateBuffer(env.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size_B, h_B, &env.errcode);
  d_C = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY,                       mem_size_C, NULL, &env.errcode);

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
  d_A = clCreateBuffer(env.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size_A, h_A, &env.errcode);
  d_B = clCreateBuffer(env.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size_B, h_B, &env.errcode);
  d_C = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY,                       mem_size_C, NULL, &env.errcode);

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


