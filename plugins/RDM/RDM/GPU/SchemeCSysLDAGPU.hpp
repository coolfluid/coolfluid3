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
      DvPlus[n].setZero();

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

 virtual ~Term()
  {
    clReleaseContext(env.context);
    clReleaseKernel(env.kernel);
    clReleaseCommandQueue(env.command_queue);
  };

  /// Get the class name
  static std::string type_name () { return "CSysLDA.Scheme<" + SF::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

protected: // data

  /// The operator L in the advection equation Lu = f
  /// Matrix Ki_n stores the value L(N_i) at each quadrature point for each shape function N_i
  typename B::PhysicsMT  Ki_n [SF::nb_nodes];
  /// sum of Lplus to be inverted
  typename B::PhysicsMT  sumLplus;
  /// inverse Ki+ matix
  typename B::PhysicsMT  InvKi_n;
  /// right eigen vector matrix
  typename B::PhysicsMT  Rv;
  /// left eigen vector matrix
  typename B::PhysicsMT  Lv;
  /// diagonal matrix with eigen values
  typename B::PhysicsVT  Dv;
  /// temporary hold of Values of the operator L(u) computed in quadrature points.
  typename B::PhysicsVT  LUwq;
  /// diagonal matrix with positive eigen values
  typename B::PhysicsVT  DvPlus [SF::nb_nodes];

};

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void CSysLDAGPU::Term<SF,QD,PHYS>::execute()
{
    /*std::cout<<"sysLDAGPU"<<std::endl;


       //boost::timer ctimer;
       uint nEq     = 4; // number of equation
       uint dim     = 2; // dimension
       uint shape   = SHAPEFUNC::nb_nodes;
       uint quad    =  QUADRATURE::nb_points;
       uint nodes   = (*coordinates).size();
       uint elements = (*connectivity_table).size();

       typename SHAPEFUNC::MappedGradientT m_sf_grad_ref; //Gradient of the shape functions in reference space
       typename SHAPEFUNC::ShapeFunctionsT m_sf_ref;   //Values of shape functions in reference space

       float A_inter[shape*quad], A_ksi[shape*quad], A_eta[shape*quad];
       float weights[quad];

       for( uint idx = 0; idx < quad; idx++ )
       {
           for( uint idy = 0; idy<shape;idy++ )
           {
               uint elem = idx * shape + idy;

               SHAPEFUNC::mapped_gradient( m_quadrature.coords.col(idx), m_sf_grad_ref );
               SHAPEFUNC::shape_function ( m_quadrature.coords.col(idx), m_sf_ref   );

               A_inter[elem] = m_sf_ref[idy];
               A_ksi[elem]   = m_sf_grad_ref(KSI,idy);
               A_eta[elem]   = m_sf_grad_ref(ETA,idy);
           }
           weights[idx] = m_quadrature.weights[idx];//m_wj[idx];
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
               connectTable[pos] = (*connectivity_table)[idx][idy];
           }

       }

       for(uint idx = 0; idx < nodes; idx++ )
       {
           for(uint idy = 0; idy < dim; idy++)
           {
               uint pos = idx * dim + idy;
               X_node[pos] = (*coordinates)[idx][idy];
           }
           for( uint idy = 0; idy < nEq; idy++ )
           {
               uint pos = idx * nEq + idy;
               U_node[pos] = (*solution)[idx][idy];
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

       /*   __kernel void interpolation(__global float* PHI, __global float* waveSpeed,
                                        __global float* A, __global float* A_ksi, __global float* A_eta,
                                        __global float* weights,
                                        __global float* X_node, __global float* U_node,
                                        __global uint* connectTable,
                                        int shape, int quad, int dim, int elem,
                                        __local float* X_shape, __local float* U_shape,
                                        __local float* X_quad, __local float* X_ksi, __local float* X_eta )        */
       /*
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
       env.errcode |= clSetKernelArg(env.kernel, n++, shape * dim * sizeof(float), 0);
       env.errcode |= clSetKernelArg(env.kernel, n++, shape       * sizeof(float), 0);
       env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim  * sizeof(float), 0);
       env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim  * sizeof(float), 0);
       env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim  * sizeof(float), 0);
       env.errcode |= clSetKernelArg(env.kernel, n++, quad *  sizeof(float), 0);
       env.errcode |= clSetKernelArg(env.kernel, n++, quad *  sizeof(float), 0);
       env.errcode |= clSetKernelArg(env.kernel, n++, quad *  sizeof(float), 0);

       opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
       */


       // code similar to GPU code for one core on CPU
    /*
       for( uint tx = 0; tx < elements; tx++ )
       {
           float X_shape[shape*dim];
           float U_shape[shape*nEq];

           // connection data from the table to the real element

           for(unsigned int j = 0;j< shape;j++ )
           {
                          uint adress = connectTable[ tx * shape + j ];
                          for( unsigned int k = 0; k < dim; k++)
                          {
                              X_shape[ j*dim + k] = X_node[ adress*dim + k ];
                          }
                          for( unsigned int k = 0; k < nEq; k++)
                          {
                              X_shape[ j*nEq + k] = X_node[ adress*nEq + k ];
                          }
           }

           //  interpolation data from shape to quadreture points

           float X_quad[quad*dim],X_ksi[quad*dim],X_eta[quad*dim], U_quad[quad*nEq];
           float JMinv[quad*dim*dim],jq[quad];
           float Rv[nEq*nEq*quad], Lv[nEq*nEq*quad], Dv[nEq*quad], Af[nEq*nEq*quad], Bf[nEq*nEq*quad];
           float Ki[nEq*nEq*quad*shape], invKi[nEq*nEq*quad*shape];
           float sumLplus[nEq*nEq*quad];

           float gamma = 1.4;
           float R = 287.058;

           for(unsigned int j = 0;j< quad;j++ )
           {
              for( unsigned int k = 0; k < dim; k++ )
              {
                   unsigned int elemC =  j * dim + k;

                   float value = 0;
                   float value1 = 0;
                   float value2 = 0;

                   for( unsigned int l = 0; l < shape; l++ )
                   {
                       unsigned int elemA = j * shape + l;
                       unsigned int elemB = l * dim + k;

                       value += A_inter[elemA] * X_shape[elemB];
                       value1 += A_ksi[elemA] * X_shape[elemB];
                       value2 += A_eta[elemA] * X_shape[elemB];
                   }
                   X_quad[elemC] = value;
                   X_ksi[elemC] =  value1;
                   X_eta[elemC] =  value2;
              }

              jq[j] = X_ksi[j * dim] * X_eta[j*dim+1] - X_ksi[j * dim+1] * X_eta[j*dim];

              JMinv[j*dim*dim]   = X_eta[j*dim+1] / jq[j]; JMinv[j*dim*dim+1] =-X_eta[j*dim] / jq[j];
              JMinv[j*dim*dim+2] = X_ksi[j*dim+1] / jq[j]; JMinv[j*dim*dim+3] = X_ksi[j*dim] / jq[j];


              for( unsigned int k = 0; k < nEq; k++ )
              {
                   unsigned int elemC =  j * nEq + k;

                   float value = 0;

                   for( unsigned int l = 0; l < shape; l++ )
                   {
                       unsigned int elemA = j * shape + l;
                       unsigned int elemB = l * nEq + k;

                       value += A_inter[elemA] * U_shape[elemB];
                   }
                   U_quad[elemC] = value;
              }

              // set matrix Ki

              // compute properties

              float rho = U_quad[j*nEq];
              float rhoU = U_quad[j*nEq + 1];
              float rhoV = U_quad[j*nEq + 2];
              float rhoE = U_quad[j*nEq + 3];
              float u = rhoU / rho;
              float v = rhoV / rho;
              float uuvv = u*u + v*v;
              float H = gamma * rhoE / rho - 0.5 * ( gamma - 1 ) * uuvv;
              float a = sqrt((gamma-1)*(H-0.5*uuvv));
              float T = a*a/(gamma*R);
              float p = rho * R * T;
              float E  = H-p / rho;
              float half_gm1_v2 = 0.5 * (gamma - 1 ) * uuvv;

              int adressQ = j*nEq*nEq;
              for( int k = 0; k < 16; k++ )
                  sumLplus[adressQ+k] = 0;

              for( int k = 0; k < shape; k++ )
              {
                  float nx = 1; // correct
                  float ny = 1; // correct

                  float um = u * nx + v * ny;
                  float ra = 0.5 * rho / a;
                  float coeffM2 = half_gm1_v2 / (a*a);
                  float uDivA = (gamma -1) * u / a;
                  float vDivA = (gamma -1) * v / a;
                  float gm1_ov_rhoa = (gamma -1) / a;

                  Rv[adressQ]    = 1;        Rv[adressQ+1]  = 0.0;             Rv[adressQ+2]  = ra;            Rv[adressQ+3]  = ra;
                  Rv[adressQ+4]  = u;        Rv[adressQ+5]  = rho*ny;          Rv[adressQ+6]  = ra * (u+a*nx); Rv[adressQ+7]  = ra * (u-a*nx);
                  Rv[adressQ+8]  = v;        Rv[adressQ+9]  = -rho*nx;         Rv[adressQ+10] = ra * (v+a*ny); Rv[adressQ+11] = ra * (v-a*ny);
                  Rv[adressQ+12] = 0.5*uuvv; Rv[adressQ+13] = rho*(u*ny-v*nx); Rv[adressQ+14] = ra * (H+a*um); Rv[adressQ+15] = ra * (H-a*um);

                  Lv[adressQ]    = 1-coeffM2;                    Lv[adressQ+1]  = uDivA / a;        Lv[adressQ+2]  = vDivA / a;        Lv[adressQ+3] = (gamma-1)/(a*a);
                  Lv[adressQ+4]  = ( v*nx-u*ny ) / rho;          Lv[adressQ+5]  = ny/rho;           Lv[adressQ+6]  = -nx / rho;        Lv[adressQ+7] = 0.0;
                  Lv[adressQ+8]  = a / rho * (coeffM2 - um / a); Lv[adressQ+9]  = (nx-uDivA) / rho; Lv[adressQ+10] = (ny-vDivA) / rho; Lv[adressQ+11] = gm1_ov_rhoa;
                  Lv[adressQ+12] = a / rho * (coeffM2 + um / a); Lv[adressQ+13] = (nx+uDivA) / rho; Lv[adressQ+14] = (ny+vDivA) / rho; Lv[adressQ+15] = gm1_ov_rhoa;

                  //Dv[j*nEq] = max(um,0); Dv[j*nEq+1] = max(um,0); Dv[j*nEq+2] = max(um+a,0); Dv[j*nEq+3] =max(um-a,0);
                  if (um<0) { Dv[j*nEq] = 0; Dv[j*nEq+1] = 0; } else { Dv[j*nEq] = um; Dv[j*nEq+1] = um; }
                  if (um+a<0) Dv[j*nEq+2]=0; else Dv[j*nEq+2]=um+a;
                  if (um-a<0) Dv[j*nEq+3]=0; else Dv[j*nEq+3]=um-a;

                  int adressQS = (j*shape+k)*nEq*nEq;

                  for( int l = 0; l < nEq*nEq; l++ )
                      Ki[adressQS+l] = 0;
                  for( int l = 0; l < nEq; l++ )
                     for( int m = 0; m < nEq; m++ )
                        for( int n = 0; n < nEq; n++ )
                              Ki[adressQS+l*nEq+m] += Rv[adressQ+l*nEq+n ] * Dv[j*nEq+n] * Lv[adressQ+m+n*nEq ];
                  for( int l = 0; l < nEq*nEq; l++ )
                      sumLplus[adressQ+l] += Ki[adressQS+l];


               }

              float detKi = sumLplus[adressQ]  *(sumLplus[adressQ+5]*sumLplus[adressQ+10]*sumLplus[adressQ+15] + sumLplus[adressQ+6]*sumLplus[adressQ+11]*sumLplus[adressQ+13] + sumLplus[adressQ+7]*sumLplus[adressQ+9] *sumLplus[adressQ+14] - sumLplus[adressQ+5]*sumLplus[adressQ+11]*sumLplus[adressQ+14] - sumLplus[adressQ+6]*sumLplus[adressQ+9] *sumLplus[adressQ+15] - sumLplus[adressQ+7]*sumLplus[adressQ+10]*sumLplus[adressQ+13] )
                          + sumLplus[adressQ+1]*(sumLplus[adressQ+4]*sumLplus[adressQ+11]*sumLplus[adressQ+14] + sumLplus[adressQ+6]*sumLplus[adressQ+8] *sumLplus[adressQ+15] + sumLplus[adressQ+7]*sumLplus[adressQ+10]*sumLplus[adressQ+12] - sumLplus[adressQ+4]*sumLplus[adressQ+10]*sumLplus[adressQ+15] - sumLplus[adressQ+6]*sumLplus[adressQ+11]*sumLplus[adressQ+12] - sumLplus[adressQ+7]*sumLplus[adressQ+8] *sumLplus[adressQ+14] )
                          + sumLplus[adressQ+2]*(sumLplus[adressQ+4]*sumLplus[adressQ+9] *sumLplus[adressQ+15] + sumLplus[adressQ+5]*sumLplus[adressQ+11]*sumLplus[adressQ+12] + sumLplus[adressQ+7]*sumLplus[adressQ+8] *sumLplus[adressQ+13] - sumLplus[adressQ+4]*sumLplus[adressQ+11]*sumLplus[adressQ+13] - sumLplus[adressQ+5]*sumLplus[adressQ+8] *sumLplus[adressQ+15] - sumLplus[adressQ+7]*sumLplus[adressQ+9] *sumLplus[adressQ+12] )
                          + sumLplus[adressQ+3]*(sumLplus[adressQ+4]*sumLplus[adressQ+10]*sumLplus[adressQ+13] + sumLplus[adressQ+5]*sumLplus[adressQ+8] *sumLplus[adressQ+14] + sumLplus[adressQ+6]*sumLplus[adressQ+9] *sumLplus[adressQ+12] - sumLplus[adressQ+4]*sumLplus[adressQ+9] *sumLplus[adressQ+14] - sumLplus[adressQ+5]*sumLplus[adressQ+10]*sumLplus[adressQ+12] - sumLplus[adressQ+6]*sumLplus[adressQ+8] *sumLplus[adressQ+13] );

              invKi[adressQ]    = 1 / detKi * ( sumLplus[adressQ+5]*(sumLplus[adressQ+10]*sumLplus[adressQ+15] - sumLplus[adressQ+11]*sumLplus[adressQ+14]) + sumLplus[adressQ+6]*(sumLplus[adressQ+11]*sumLplus[adressQ+13] - sumLplus[adressQ+9] *sumLplus[adressQ+15]) + sumLplus[adressQ+7]*(sumLplus[adressQ+9] *sumLplus[adressQ+14] - sumLplus[adressQ+10]*sumLplus[adressQ+13]) ); // 0
              invKi[adressQ+1]  = 1 / detKi * ( sumLplus[adressQ+1]*(sumLplus[adressQ+11]*sumLplus[adressQ+14] - sumLplus[adressQ+10]*sumLplus[adressQ+15]) + sumLplus[adressQ+2]*(sumLplus[adressQ+9] *sumLplus[adressQ+15] - sumLplus[adressQ+11]*sumLplus[adressQ+13]) + sumLplus[adressQ+3]*(sumLplus[adressQ+10]*sumLplus[adressQ+13] - sumLplus[adressQ+9] *sumLplus[adressQ+14]) ); // 1
              invKi[adressQ+2]  = 1 / detKi * ( sumLplus[adressQ+1]*(sumLplus[adressQ+6] *sumLplus[adressQ+15] - sumLplus[adressQ+7] *sumLplus[adressQ+14]) + sumLplus[adressQ+2]*(sumLplus[adressQ+7] *sumLplus[adressQ+13] - sumLplus[adressQ+5] *sumLplus[adressQ+15]) + sumLplus[adressQ+3]*(sumLplus[adressQ+5] *sumLplus[adressQ+14] - sumLplus[adressQ+6] *sumLplus[adressQ+13]) ); // 2
              invKi[adressQ+3]  = 1 / detKi * ( sumLplus[adressQ+1]*(sumLplus[adressQ+7] *sumLplus[adressQ+10] - sumLplus[adressQ+6] *sumLplus[adressQ+11]) + sumLplus[adressQ+2]*(sumLplus[adressQ+5] *sumLplus[adressQ+11] - sumLplus[adressQ+7] *sumLplus[adressQ+9])  + sumLplus[adressQ+3]*(sumLplus[adressQ+6] *sumLplus[adressQ+9]  - sumLplus[adressQ+5] *sumLplus[adressQ+10]) ); // 3
              invKi[adressQ+4]  = 1 / detKi * ( sumLplus[adressQ+4]*(sumLplus[adressQ+11]*sumLplus[adressQ+14] - sumLplus[adressQ+10]*sumLplus[adressQ+15]) + sumLplus[adressQ+6]*(sumLplus[adressQ+8] *sumLplus[adressQ+15] - sumLplus[adressQ+11]*sumLplus[adressQ+12]) + sumLplus[adressQ+7]*(sumLplus[adressQ+10]*sumLplus[adressQ+12] - sumLplus[adressQ+8] *sumLplus[adressQ+14]) ); // 4
              invKi[adressQ+5]  = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+10]*sumLplus[adressQ+15] - sumLplus[adressQ+11]*sumLplus[adressQ+14]) + sumLplus[adressQ+2]*(sumLplus[adressQ+11]*sumLplus[adressQ+12] - sumLplus[adressQ+8] *sumLplus[adressQ+15]) + sumLplus[adressQ+3]*(sumLplus[adressQ+8] *sumLplus[adressQ+14] - sumLplus[adressQ+10]*sumLplus[adressQ+12]) ); // 5
              invKi[adressQ+6]  = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+7] *sumLplus[adressQ+14] - sumLplus[adressQ+6] *sumLplus[adressQ+15]) + sumLplus[adressQ+2]*(sumLplus[adressQ+4] *sumLplus[adressQ+15] - sumLplus[adressQ+7] *sumLplus[adressQ+12]) + sumLplus[adressQ+3]*(sumLplus[adressQ+6] *sumLplus[adressQ+12] - sumLplus[adressQ+4] *sumLplus[adressQ+14]) ); // 6
              invKi[adressQ+7]  = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+6] *sumLplus[adressQ+11] - sumLplus[adressQ+7] *sumLplus[adressQ+10]) + sumLplus[adressQ+2]*(sumLplus[adressQ+7] *sumLplus[adressQ+8]  - sumLplus[adressQ+4] *sumLplus[adressQ+11]) + sumLplus[adressQ+3]*(sumLplus[adressQ+4] *sumLplus[adressQ+10] - sumLplus[adressQ+6] *sumLplus[adressQ+8]) ); // 7
              invKi[adressQ+8]  = 1 / detKi * ( sumLplus[adressQ+4]*(sumLplus[adressQ+9] *sumLplus[adressQ+15] - sumLplus[adressQ+11]*sumLplus[adressQ+13]) + sumLplus[adressQ+5]*(sumLplus[adressQ+11]*sumLplus[adressQ+12] - sumLplus[adressQ+8] *sumLplus[adressQ+15]) + sumLplus[adressQ+7]*(sumLplus[adressQ+8] *sumLplus[adressQ+13] - sumLplus[adressQ+9] *sumLplus[adressQ+12]) ); // 8
              invKi[adressQ+9]  = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+11]*sumLplus[adressQ+13] - sumLplus[adressQ+9] *sumLplus[adressQ+15]) + sumLplus[adressQ+1]*(sumLplus[adressQ+8] *sumLplus[adressQ+15] - sumLplus[adressQ+11]*sumLplus[adressQ+12]) + sumLplus[adressQ+3]*(sumLplus[adressQ+9] *sumLplus[adressQ+12] - sumLplus[adressQ+8] *sumLplus[adressQ+13]) ); // 9
              invKi[adressQ+10] = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+5] *sumLplus[adressQ+15] - sumLplus[adressQ+7] *sumLplus[adressQ+13]) + sumLplus[adressQ+1]*(sumLplus[adressQ+7] *sumLplus[adressQ+12] - sumLplus[adressQ+4] *sumLplus[adressQ+15]) + sumLplus[adressQ+3]*(sumLplus[adressQ+4] *sumLplus[adressQ+13] - sumLplus[adressQ+5] *sumLplus[adressQ+12]) ); // 10
              invKi[adressQ+11] = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+7] *sumLplus[adressQ+9]  - sumLplus[adressQ+5] *sumLplus[adressQ+11]) + sumLplus[adressQ+1]*(sumLplus[adressQ+4] *sumLplus[adressQ+11] - sumLplus[adressQ+7] *sumLplus[adressQ+8])  + sumLplus[adressQ+3]*(sumLplus[adressQ+5] *sumLplus[adressQ+8]  - sumLplus[adressQ+4] *sumLplus[adressQ+9]) ); // 11
              invKi[adressQ+12] = 1 / detKi * ( sumLplus[adressQ+4]*(sumLplus[adressQ+10]*sumLplus[adressQ+13] - sumLplus[adressQ+9] *sumLplus[adressQ+14]) + sumLplus[adressQ+5]*(sumLplus[adressQ+8] *sumLplus[adressQ+14] - sumLplus[adressQ+10]*sumLplus[adressQ+12]) + sumLplus[adressQ+6]*(sumLplus[adressQ+9] *sumLplus[adressQ+12] - sumLplus[adressQ+8] *sumLplus[adressQ+13]) ); // 12
              invKi[adressQ+13] = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+9] *sumLplus[adressQ+14] - sumLplus[adressQ+10]*sumLplus[adressQ+13]) + sumLplus[adressQ+1]*(sumLplus[adressQ+10]*sumLplus[adressQ+12] - sumLplus[adressQ+8] *sumLplus[adressQ+14]) + sumLplus[adressQ+2]*(sumLplus[adressQ+8] *sumLplus[adressQ+13] - sumLplus[adressQ+9] *sumLplus[adressQ+12]) ); // 13
              invKi[adressQ+14] = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+6] *sumLplus[adressQ+13] - sumLplus[adressQ+5] *sumLplus[adressQ+14]) + sumLplus[adressQ+1]*(sumLplus[adressQ+4] *sumLplus[adressQ+14] - sumLplus[adressQ+6] *sumLplus[adressQ+12]) + sumLplus[adressQ+2]*(sumLplus[adressQ+5] *sumLplus[adressQ+12] - sumLplus[adressQ+4] *sumLplus[adressQ+13]) ); // 14
              invKi[adressQ+15] = 1 / detKi * ( sumLplus[adressQ]  *(sumLplus[adressQ+5] *sumLplus[adressQ+10] - sumLplus[adressQ+6] *sumLplus[adressQ+9])  + sumLplus[adressQ+1]*(sumLplus[adressQ+6] *sumLplus[adressQ+8]  - sumLplus[adressQ+4] *sumLplus[adressQ+10]) + sumLplus[adressQ+2]*(sumLplus[adressQ+4] *sumLplus[adressQ+9]  - sumLplus[adressQ+5] *sumLplus[adressQ+8]) ); // 15


              Af[adressQ]    = 0;                  Af[adressQ+1]  = 1.0;               Af[adressQ+2]  = 0.0;            Af[adressQ+3] = 0.0;
              Af[adressQ+4]  = half_gm1_v2 -u*u;   Af[adressQ+5]  = -(gamma - 3)*u;    Af[adressQ+6]  = -(gamma-1)*v;   Af[adressQ+7] =( gamma - 1 );
              Af[adressQ+8]  = -u*v;               Af[adressQ+9]  = v;                 Af[adressQ+10] = u;              Af[adressQ+11] = 0.0;
              Af[adressQ+12] = ( half_gm1_v2-H)*u; Af[adressQ+13] = -(gamma-1)*u*u +H; Af[adressQ+14] = -(gamma-1)*u*v; Af[adressQ+15] = gamma * u;

              Bf[adressQ]    = 0;                  Bf[adressQ+1]  = 0.0;            Bf[adressQ+2]  = 1.0;               Bf[adressQ+3] = 0.0;
              Bf[adressQ+4]  = -u*v;               Bf[adressQ+5]  = v;              Bf[adressQ+6]  = u;                 Bf[adressQ+7] = 0.0;
              Bf[adressQ+8]  = half_gm1_v2 -v*v;   Bf[adressQ+9]  = -(gamma-1)*u;   Bf[adressQ+10] = -(gamma - 3)*v;    Bf[adressQ+11] =( gamma - 1 );
              Bf[adressQ+12] = ( half_gm1_v2-H)*v; Bf[adressQ+13] = -(gamma-1)*u*v; Bf[adressQ+14] = -(gamma-1)*v*v +H; Bf[adressQ+15] = gamma *v;


           }


       }*/



      /* size_t localWorkSize[2], globalWorkSize[2];

       localWorkSize[0] = 8;
       localWorkSize[1] = 8;

       globalWorkSize[0] = 4*env.num_compute_units;
       globalWorkSize[1] = 4*env.num_compute_units;

       env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
       opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
       clFinish(env.command_queue);

       // receive data from GPGPU memory

       env.errcode  = clEnqueueReadBuffer(env.command_queue,phi_rGPGPU,     CL_TRUE, 0, nodes * nEq * sizeof(float), phi, 0, NULL, NULL);
       env.errcode |= clEnqueueReadBuffer(env.command_queue,waveSpeedGPGPU, CL_TRUE, 0, nodes * sizeof(float), waveSpeed, 0, NULL, NULL);

       opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
    */
       // release GPGPU memory for matrix
    /*
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
            (*residual)[idx][0]   = phi[idx];
            (*wave_speed)[idx][0] = waveSpeed[idx];
       }*/

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeCSysLDAGPU_hpp
