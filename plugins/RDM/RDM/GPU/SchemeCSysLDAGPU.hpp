// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SchemeCSysLDAGPU_hpp
#define CF_RDM_SchemeCSysLDAGPU_hpp

#include "RDM/Core/SchemeBase.hpp"
#include "RDM/GPU/CLdeclaration.hpp"
#include "RDM/GPU/LibGPU.hpp"
#include "iostream"


/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_SCHEMES_API CSysLDAGPU::Term : public SchemeBase<SF,QD,PHYS> {

public: // typedefs

  /// base class type
  typedef SchemeBase<SF,QD,PHYS> B;

  /// pointers
  typedef boost::shared_ptr< Term > Ptr;
  typedef boost::shared_ptr< Term const> ConstPtr;

  CLEnv env;

public: // functions

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name ) : SchemeBase<SF,QD,PHYS>(name)
  {
    for(Uint n = 0; n < SF::nb_nodes; ++n)
      DvP[n].setZero();

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

    #include "RDM/GPU/sysLDAGPUkernel.hpp"

    // OpenCL kernel compilation

    env.program = clCreateProgramWithSource(env.context, 1, (const char**)&GPUSource, NULL, &env.errcode);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    env.errcode = clBuildProgram(env.program, 0,  NULL, "-cl-fast-relaxed-math", NULL, NULL);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    env.kernel = clCreateKernel(env.program, "interpolation", &env.errcode);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
    clReleaseProgram(env.program);
  }

  /// Virtual destructor
  virtual ~Term()
  {
      clReleaseContext(env.context);
      clReleaseKernel(env.kernel);
      clReleaseCommandQueue(env.command_queue);
  };

  /// Get the class name
  static std::string type_name () { return "CSysLDAGPU.Scheme<" + SF::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

protected: // data

  /// Matrix KiP_n stores the value L(N_i)+ at each quadrature point for each shape function N_i
  typename B::PhysicsMT  KiP_n [SF::nb_nodes];
  /// Matrix KiM_n stores the value L(N_i)- at each quadrature point for each shape function N_i
  typename B::PhysicsMT  KiM_n [SF::nb_nodes];
  /// sum of Lplus to be inverted
  typename B::PhysicsMT  sumKmin;
  /// inverse Ki+ matix
  typename B::PhysicsMT  InvKi_n;
  /// right eigen vector matrix
  typename B::PhysicsMT  Rv;
  /// left eigen vector matrix
  typename B::PhysicsMT  Lv;
  /// Ki matrix
  typename B::PhysicsMT  Ki;
  /// diagonal matrix with eigen values
  typename B::PhysicsVT  Dv;
  /// diagonal matrix with positive eigen values
  typename B::PhysicsVT  DvP [SF::nb_nodes];
  /// diagonal matrix with negative eigen values
  typename B::PhysicsVT  DvM [SF::nb_nodes];
};

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void CSysLDAGPU::Term<SF,QD,PHYS>::execute()
{
    std::cout<<"LDAGPU"<<std::endl;

    //boost::timer ctimer;
    uint dim     = 2;
    uint nEq     = 4;
    uint shape   = SF::nb_nodes;
    uint quad    =  QD::nb_points;
    uint nodes   = (*B::coordinates).size();
    uint elements = (*B::connectivity_table).size();

    float A_inter[shape*quad], A_ksi[shape*quad], A_eta[shape*quad];
    float weights[quad];

    for( uint idx = 0; idx < quad; idx++ )
    {
       for( uint idy = 0; idy<shape;idy++ )
       {
           uint elem = idx * shape + idy;

           A_inter[elem] = B::Ni(idx,idy);
           A_ksi[elem]   = B::dNdKSI[0](idx,idy);
           A_eta[elem]   = B::dNdKSI[1](idx,idy);
       }
       weights[idx] = B::m_quadrature.weights[idx];
    }

    cl_mem A_iGPGPU, A_kGPGPU, A_eGPGPU, weightsGPGPU;
    A_iGPGPU     = clCreateBuffer(env.context, CL_MEM_READ_ONLY, shape * quad * sizeof(float), A_inter, &env.errcode);
    A_kGPGPU     = clCreateBuffer(env.context, CL_MEM_READ_ONLY, shape * quad * sizeof(float), A_ksi,   &env.errcode);
    A_eGPGPU     = clCreateBuffer(env.context, CL_MEM_READ_ONLY, shape * quad * sizeof(float), A_eta,   &env.errcode);
    weightsGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY, quad * sizeof(float)        , weights, &env.errcode);
    env.errcode  = clEnqueueWriteBuffer(env.command_queue, A_iGPGPU,      CL_FALSE, 0, shape * quad * sizeof(float), A_inter,    0, NULL, NULL);
    env.errcode |= clEnqueueWriteBuffer(env.command_queue, A_kGPGPU,      CL_FALSE, 0, shape * quad * sizeof(float), A_ksi,      0, NULL, NULL);
    env.errcode |= clEnqueueWriteBuffer(env.command_queue, A_eGPGPU,      CL_FALSE, 0, shape * quad * sizeof(float), A_eta,      0, NULL, NULL);
    env.errcode |= clEnqueueWriteBuffer(env.command_queue, weightsGPGPU,  CL_FALSE, 0, quad * sizeof(float),         weights,    0, NULL, NULL);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    float X_node[nodes*dim], U_node[nodes*nEq],phi[nodes*nEq], waveSpeed[nodes];
    uint connectTable[elements*shape];

    for(uint idx = 0; idx < elements; idx++ )
    {
       for(uint idy = 0; idy < shape; idy++)
       {
           uint pos = idx * shape + idy;
           connectTable[pos] = (*B::connectivity_table)[idx][idy];
       }

    }

    for(uint idx = 0; idx < nodes; idx++ )
    {
       for(uint idy = 0; idy < dim; idy++)
       {
           uint pos = idx * dim + idy;
           X_node[pos] = (*B::coordinates)[idx][idy];
       }
       for( uint idy = 0; idy < nEq; idy++ )
       {
           uint pos = idx * nEq + idy;
           U_node[pos] = (*B::solution)[idx][idy];
           phi[pos]    = 0;
       }
       waveSpeed[idx]=0;
    }
    //GPGPU mem reservation

    cl_mem X_rGPGPU, U_rGPGPU, phi_rGPGPU, connectTableGPGPU;
    cl_mem waveSpeedGPGPU;

    // copy vector to GPGPU memory

    X_rGPGPU          = clCreateBuffer(env.context, CL_MEM_READ_ONLY , nodes * dim * sizeof(float), X_node,    &env.errcode);
    U_rGPGPU          = clCreateBuffer(env.context, CL_MEM_READ_ONLY , nodes * nEq * sizeof(float), U_node,    &env.errcode);
    phi_rGPGPU        = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, nodes * nEq * sizeof(float), phi,       &env.errcode);
    waveSpeedGPGPU    = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, nodes * sizeof(float),       waveSpeed, &env.errcode);
    connectTableGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY,  elements * shape * sizeof(uint),  connectTable, &env.errcode);

    env.errcode |= clEnqueueWriteBuffer(env.command_queue, X_rGPGPU,          CL_FALSE, 0, nodes * dim * sizeof(float), X_node,             0, NULL, NULL);
    env.errcode |= clEnqueueWriteBuffer(env.command_queue, U_rGPGPU,          CL_FALSE, 0, nodes * nEq * sizeof(float), U_node,             0, NULL, NULL);
    env.errcode |= clEnqueueWriteBuffer(env.command_queue, phi_rGPGPU,        CL_FALSE, 0, nodes * nEq * sizeof(float), phi,                0, NULL, NULL);
    env.errcode |= clEnqueueWriteBuffer(env.command_queue, waveSpeedGPGPU,    CL_FALSE, 0, nodes *  sizeof(float),      waveSpeed,          0, NULL, NULL);
    env.errcode |= clEnqueueWriteBuffer(env.command_queue, connectTableGPGPU, CL_FALSE, 0, elements * shape*  sizeof(uint),  connectTable,  0, NULL, NULL);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    // running GPGPU kernel

    /*    __kernel void interpolation(__global float* phi, __global float* phiN, __global float* waveSpeed,
                                      __global float* A, __global float* A_ksi, __global float* A_eta,
                                      __global float* weights,
                                      __global float* X_node, __global float* U_node,
                                      __global uint* connectTable,
                                        int shape, int quad, int dim, int elem, int nEq,
                                      __local float* X_shape,  __local float* U_shape,
                                      __local float* X_quad,   __local float* X_ksi, __local float* X_eta,
                                      __local float* U_quad,   __local float* jq,    __local float* Rv,
                                      __local float* Lv,       __local float* Dv,    __local float* Af,
                                      __local float* Bf,       __local float* Ki,    __local float* invKi,
                                      __local float* sumLplus, __local float* dudx,  __local float* dudy,
                                      __local float* LU,       __local float* LUwq,  __local float* help,
                                      __local float* phiH,     __local float* phiHN )*/



    int n = 0;
    env.errcode  = clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&phi_rGPGPU);
    env.errcode  = clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&waveSpeedGPGPU);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_iGPGPU);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_kGPGPU);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_eGPGPU);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&weightsGPGPU);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&X_rGPGPU);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&U_rGPGPU);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&connectTableGPGPU);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&shape);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&quad);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&dim);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&elements);
    env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&nEq);
    env.errcode |= clSetKernelArg(env.kernel, n++, shape * dim *              sizeof(float), 0); // X_shape
    env.errcode |= clSetKernelArg(env.kernel, n++, shape * nEq *              sizeof(float), 0); // U_shape
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim  *              sizeof(float), 0); // X_quad
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim  *              sizeof(float), 0); // X_ksi
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim  *              sizeof(float), 0); // X_eta
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq  *              sizeof(float), 0); // U_quad
    env.errcode |= clSetKernelArg(env.kernel, n++, quad *                     sizeof(float), 0); // jacobi
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq * nEq * shape * sizeof(float), 0); // Rv
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq * nEq * shape * sizeof(float), 0); // Lv
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq *       shape * sizeof(float), 0); // Dv
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq * nEq *         sizeof(float), 0); // Af
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq * nEq *         sizeof(float), 0); // Bf
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq * nEq * shape * sizeof(float), 0); // Ki
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq * nEq *         sizeof(float), 0); // invKi
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq * nEq *         sizeof(float), 0); // sumLplus
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq  *              sizeof(float), 0); // dudx
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq  *              sizeof(float), 0); // dudy
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq  *              sizeof(float), 0); // LU
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq  *              sizeof(float), 0); // LUwq
    env.errcode |= clSetKernelArg(env.kernel, n++, quad * nEq  *              sizeof(float), 0); // help
    env.errcode |= clSetKernelArg(env.kernel, n++, shape * nEq  *             sizeof(float), 0); // PhiH
    env.errcode |= clSetKernelArg(env.kernel, n++, shape * nEq  *             sizeof(float), 0); // phiHN

    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    size_t localWorkSize[2], globalWorkSize[2];

    localWorkSize[0] = 8;
    localWorkSize[1] = 8;

    globalWorkSize[0] = 4*env.num_compute_units;
    globalWorkSize[1] = 4*env.num_compute_units;

    env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
    clFinish(env.command_queue);

    // receive data from GPGPU memory
    env.errcode  = clEnqueueReadBuffer(env.command_queue,phi_rGPGPU,     CL_TRUE, 0, nodes * nEq * sizeof(float), phi,  0, NULL, NULL);
    env.errcode |= clEnqueueReadBuffer(env.command_queue,waveSpeedGPGPU, CL_TRUE, 0, nodes * sizeof(float), waveSpeed,  0, NULL, NULL);

    opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

    // release GPGPU memory for matrix

    clReleaseMemObject( A_iGPGPU );
    clReleaseMemObject( A_kGPGPU );
    clReleaseMemObject( A_eGPGPU );
    clReleaseMemObject( weightsGPGPU );

    // release GPGPU memory for vectors

    clReleaseMemObject( X_rGPGPU );
    clReleaseMemObject( U_rGPGPU );
    clReleaseMemObject( phi_rGPGPU );
    clReleaseMemObject( waveSpeedGPGPU );
    clReleaseMemObject( connectTableGPGPU );

    for(uint idx = 0; idx < nodes; idx++ )
    {
       for( int idy = 0; idy < nEq; idy++ )
       {
           int node = idx*nEq + idy;
           (*B::residual)[idx][idy] = phi[node];
       }
       (*B::wave_speed)[idx][0] = waveSpeed[idx];
    }


}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeCSysLDAGPU_hpp
