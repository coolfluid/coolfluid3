#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <cassert>

#include "matrix_sizes.h"
#include "matrix_mult.h"

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
};

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
int largest_divider( int a, int b )
{
    int u = a;
    int v = b;

    do
    {
        ldiv_t result = ldiv(a,b);
        a = b;
        b = result.rem;
    }
    while ( b !=0 );

    return a;
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
     clGetDeviceIDs(env.cpPlatform, CL_DEVICE_TYPE_GPU, 1, &env.cdDevice, NULL);

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


void opencl_unsetup(CLEnv& env)
{
  free(env.devices);
  clReleaseContext(env.context);
  clReleaseKernel(env.kernel);
  clReleaseProgram(env.program);
  clReleaseCommandQueue(env.command_queue);
}

void mat_mul(array_2D matrix_A, array_2D vector_field_b, array_2D &vector_field_C, int n_blocks )
{
    CLEnv clenv;
    opencl_setup(clenv);

    unsigned int size_A = matrix_A[0].size();
    unsigned int size_B = vector_field_b[0].size();
    unsigned int size_C = vector_field_C[0].size();

    unsigned int mem_size_A = sizeof(double) * size_A * n_blocks;
    unsigned int mem_size_B = sizeof(double) * size_B * n_blocks;
    unsigned int mem_size_C = sizeof(double) * size_C * n_blocks;

    unsigned int w_A = size_B * n_blocks;
    unsigned int h_A = size_C * n_blocks;
    unsigned int h_C = size_C * n_blocks;
    unsigned int w_B = 1;
    unsigned int w_C = 1;
    unsigned int h_B = size_B * n_blocks;

    float a[h_A][ n_blocks * w_A];
    float b[ n_blocks * w_A];
    float c[ n_blocks * w_A];

    for( int i = 0; i < h_A; i++ )
    {
        for( int idx = 0; idx<n_blocks; idx++ )
            for ( int j = 0; j < w_A; j++ )
                a[i][ idx * w_A + j ]=matrix_A[idx][i*w_A+j];
    }
    for( int idx = 0; idx < n_blocks; idx++ )
    {
        for(int i = 0; i < w_A; i++ )
            b[idx*w_A + i]=vector_field_b[idx][i];
    }


    // OpenCL device memory for matrices
    cl_mem d_A;
    cl_mem d_B;
    cl_mem d_C;

  // Setup device memory
    d_A = clCreateBuffer(clenv.context, CL_MEM_READ_ONLY, mem_size_A, NULL, &clenv.errcode);
    assert(clenv.errcode == CL_SUCCESS);

    d_B = clCreateBuffer(clenv.context, CL_MEM_READ_ONLY, mem_size_B, NULL, &clenv.errcode);
    assert(clenv.errcode == CL_SUCCESS);

    d_C = clCreateBuffer(clenv.context, CL_MEM_WRITE_ONLY, mem_size_C, NULL, &clenv.errcode);
    assert(clenv.errcode == CL_SUCCESS);

    // 7. Launch OpenCL kernel
    size_t localWorkSize[2], globalWorkSize[2];

    int wA = w_A;//WA;
    int wC = w_C;//WC;

    localWorkSize [0] = 16;
    localWorkSize [1] = 16;
    globalWorkSize[0] = n_blocks;
    globalWorkSize[1] = h_B;


  // 8. Retrieve result from device


    for( int idx = 0; idx < n_blocks; idx++ )
    {
         std::cout<<idx<<std::endl;
         /*double h_aI[matrix_A[idx].size()]; for( int i = 0; i < matrix_A[idx].size(); i++ ) h_aI[i] = matrix_A[idx][i];
         double h_bI[matrix_B[idx].size()]; for( int i = 0; i < vector_field_b_B[idx].size(); i++ ) h_bI[i] = matrix_B[idx][i];
         double h_cI[matrix_C[idx].size()];

         clenv.errcode  = clSetKernelArg(clenv.kernel, 0, sizeof(cl_mem), (void *)&d_C);
         clenv.errcode |= clSetKernelArg(clenv.kernel, 1, sizeof(cl_mem), (void *)&d_A);
         clenv.errcode |= clSetKernelArg(clenv.kernel, 2, sizeof(cl_mem), (void *)&d_B);
         clenv.errcode |= clSetKernelArg(clenv.kernel, 3, sizeof(int),    (void *)&wA);
         clenv.errcode |= clSetKernelArg(clenv.kernel, 4, sizeof(int),    (void *)&wC);
         opencl_check_error(clenv.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

         clenv.errcode = clEnqueueWriteBuffer(clenv.command_queue, d_A, CL_FALSE, 0, sizeof(cl_double) * matrix_A[idx].size(), h_aI, 0, NULL, NULL);
         clenv.errcode = clEnqueueWriteBuffer(clenv.command_queue, d_B, CL_FALSE, 0, sizeof(cl_double) * matrix_B[idx].size(), h_bI, 0, NULL, NULL);

         //ciErr1 = clEnqueueNDRangeKernel(cqCommandQueue, ckKernel, 1, NULL, &szGlobalWorkSize, &szLocalWorkSize, 0, NULL, NULL);

         clenv.errcode = clEnqueueNDRangeKernel(clenv.command_queue, clenv.kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
         opencl_check_error(clenv.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

         clenv.errcode = clEnqueueReadBuffer(clenv.command_queue, d_C, CL_TRUE, 0, mem_size_C, h_cI, 0, NULL, NULL);
         opencl_check_error(clenv.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

         for( int i = 0; i < matrix_C[idx].size(); i++ ) matrix_C[idx][i] = h_cI[i];*/
    }

    opencl_unsetup(clenv);
}

void cpu_multiplication( array_2D matrix_A, array_2D vector_field_b, array_2D &vector_field_C, int n_blocks  )
{
        unsigned int size_A = matrix_A[0].size();
        unsigned int size_B = vector_field_b[0].size();
        unsigned int size_C =  vector_field_b[0].size();

        unsigned int w_A = size_B;
        unsigned int h_A = size_C;

        std::cout << "2" <<std::endl;

        float a[h_A][ n_blocks * w_A];
        float b[ n_blocks * w_A];
        float c[ n_blocks * w_A];

        for( int i = 0; i < h_A; i++ )
        {
            for( int idx = 0; idx<n_blocks; idx++ )
                for ( int j = 0; j < w_A; j++ )
                    a[i][ idx * w_A + j ]=matrix_A[idx][i*w_A+j];
        }
        for( int idx = 0; idx < n_blocks; idx++ )
        {
            for(int i = 0; i < w_A; i++ )
                b[idx*w_A + i]=vector_field_b[idx][i];
        }

        for( int i=0; i<h_A;i++)
        {
            for( int j=0;j<n_blocks;j++ )
            {
                float value = 0;
                for( int k = 0; k < w_A;k++ )
                    value += a[i][j*w_A+k]*b[j*w_A+k];
                c[j*h_A+i] = value;

            }
        }

        for( int i=0; i< n_blocks; i++ )
            for( int j = 0; j < h_A; j++)
                vector_field_C[i][j]=c[i*h_A+j];



}
