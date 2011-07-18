// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SchemeLDAGPU_hpp
#define CF_RDM_SchemeLDAGPU_hpp

#include <boost/assign.hpp>
#include <iostream>          // to remove

#include <Eigen/Dense>

#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/ElementData.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

#include "RDM/GPU/CLdeclaration.hpp"
#include "RDM/GPU/LibGPU.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SF, typename QD, typename PHYS >
class RDM_GPU_API SchemeLDAGPU : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeLDAGPU > Ptr;
  typedef boost::shared_ptr< SchemeLDAGPU const> ConstPtr;

  CLEnv env;

public: // functions

  /// Contructor
  /// @param name of the component
  SchemeLDAGPU ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeLDAGPU()
  {
      clReleaseContext(env.context);
      clReleaseKernel(env.kernel);
      clReleaseCommandQueue(env.command_queue);
  };

  /// Get the class name
  static std::string type_name () { return "SchemeLDAGPU<" + SF::type_name() + ">"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void change_elements()
  {
    /// @todo improve this (ugly)

    connectivity_table = elements().as_ptr<Mesh::CElements>()->node_connectivity().as_ptr< Mesh::CTable<Uint> >();
    coordinates = elements().nodes().coordinates().as_ptr< Mesh::CTable<Real> >();

    cf_assert( is_not_null(connectivity_table) );

    /// @todo modify these to option components configured from

    Mesh::CField::Ptr csolution = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( Common::Core::instance().root(), "solution" );
    cf_assert( is_not_null( csolution ) );
    solution = csolution->data_ptr();

    Mesh::CField::Ptr cresidual = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( Common::Core::instance().root(), "residual" );
    cf_assert( is_not_null( cresidual ) );
    residual = cresidual->data_ptr();

    Mesh::CField::Ptr cwave_speed = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( Common::Core::instance().root(), "wave_speed" );
    cf_assert( is_not_null( cwave_speed ) );
    wave_speed = cwave_speed->data_ptr();
  }


private: // data

  /// pointer to connectivity table, may reset when iterating over element types
  Mesh::CTable<Uint>::Ptr connectivity_table;
  /// pointer to nodes coordinates, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr coordinates;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr solution;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr residual;
  /// pointer to solution table, may reset when iterating over element types
  Mesh::CTable<Real>::Ptr wave_speed;

  const QD& m_quadrature;

  /// Values of the solution located in the dof of the element
  Eigen::Matrix<Real, SF::nb_nodes, PHYS::MODEL::_neqs>  m_solution_nd;
  /// Values of the operator L(u) computed in quadrature points.
  Eigen::Matrix<Real, QD::nb_points, PHYS::MODEL::_neqs>    m_Lu_qd;
  /// Nodal residuals
  Eigen::Matrix<Real, SF::nb_nodes, PHYS::MODEL::_neqs>  m_phi;

  /// Integration factor (jacobian multiplied by quadrature weight)
  Eigen::Matrix<Real, QD::nb_points, 1u> m_wj;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SF, typename QD, typename PHYS>
SchemeLDAGPU<SF,QD,PHYS>::SchemeLDAGPU ( const std::string& name ) :
  CLoopOperation(name),
  m_quadrature( QD::instance() )
{
  regist_typeinfo(this);

  m_options["Elements"].attach_trigger ( boost::bind ( &SchemeLDAGPU<SF,QD,PHYS>::change_elements, this ) );

  m_phi.resize(SF::nb_nodes);

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

  #include "RDM/GPU/LDAGPUkernel.hpp"

  // OpenCL kernel compilation

  env.program = clCreateProgramWithSource(env.context, 1, (const char**)&GPUSource, NULL, &env.errcode);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  env.errcode = clBuildProgram(env.program, 0,  NULL, "-cl-fast-relaxed-math", NULL, NULL);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

  env.kernel = clCreateKernel(env.program, "interpolation", &env.errcode);
  opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
  clReleaseProgram(env.program);


}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SF,typename QD, typename PHYS>
void SchemeLDAGPU<SF, QD,PHYS>::execute()
{
   std::cout<<"LDAGPU"<<std::endl;


   //boost::timer ctimer;
   Uint dim     = 2;
   Uint shape   = SF::nb_nodes;
   Uint quad    =  QD::nb_points;
   Uint nodes   = (*coordinates).size();
   Uint elements = (*connectivity_table).size();

   typename SF::MappedGradientT m_sf_grad_ref; //Gradient of the shape functions in reference space
   typename SF::ShapeFunctionsT m_sf_ref;   //Values of shape functions in reference space

   float A_inter[shape*quad], A_ksi[shape*quad], A_eta[shape*quad];
   float weights[quad];

   for( Uint idx = 0; idx < quad; idx++ )
   {
       for( Uint idy = 0; idy<shape;idy++ )
       {
           Uint elem = idx * shape + idy;

           SF::shape_function_gradient( m_quadrature.coords.col(idx), m_sf_grad_ref );
           SF::shape_function_value ( m_quadrature.coords.col(idx), m_sf_ref   );

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


   // ************************************************************************************************
   // data strategy element
   // ************************************************************************************************

   float X_node[nodes*dim], U_node[nodes],phi[nodes], waveSpeed[nodes];
   Uint connectTable[elements*shape];

   for(Uint idx = 0; idx < elements; idx++ )
   {
       for(Uint idy = 0; idy < shape; idy++)
       {
           Uint pos = idx * shape + idy;
           connectTable[pos] = (*connectivity_table)[idx][idy];
       }

   }

   for(Uint idx = 0; idx < nodes; idx++ )
   {
       for(Uint idy = 0; idy < dim; idy++)
       {
           Uint pos = idx * dim + idy;
           X_node[ pos ] = (*coordinates)[idx][idy];
       }
       U_node[idx] = (*solution)[idx][0];
       waveSpeed[idx]=0;
       phi[idx]    = 0;

   }
   //GPGPU mem reservation

   cl_mem X_rGPGPU, U_rGPGPU, phi_rGPGPU, connectTableGPGPU;
   cl_mem waveSpeedGPGPU;

   // copy vector to GPGPU memory

   X_rGPGPU          = clCreateBuffer(env.context, CL_MEM_READ_ONLY , nodes * dim * sizeof(float) , X_node,    &env.errcode);
   U_rGPGPU          = clCreateBuffer(env.context, CL_MEM_READ_ONLY , nodes * sizeof(float),        U_node,    &env.errcode);
   phi_rGPGPU        = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, nodes * sizeof(float),        phi,       &env.errcode);
   waveSpeedGPGPU    = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, nodes * sizeof(float),        waveSpeed, &env.errcode);
   connectTableGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY,  elements * shape * sizeof(Uint),  connectTable, &env.errcode);
   env.errcode |= clEnqueueWriteBuffer(env.command_queue, X_rGPGPU,          CL_FALSE, 0, nodes * dim * sizeof(float), X_node,             0, NULL, NULL);
   env.errcode |= clEnqueueWriteBuffer(env.command_queue, U_rGPGPU,          CL_FALSE, 0, nodes * sizeof(float),       U_node,             0, NULL, NULL);
   env.errcode |= clEnqueueWriteBuffer(env.command_queue, phi_rGPGPU,        CL_FALSE, 0, nodes * sizeof(float),       phi,                0, NULL, NULL);
   env.errcode |= clEnqueueWriteBuffer(env.command_queue, waveSpeedGPGPU,    CL_FALSE, 0, nodes *  sizeof(float),      waveSpeed,          0, NULL, NULL);
   env.errcode |= clEnqueueWriteBuffer(env.command_queue, connectTableGPGPU, CL_FALSE, 0, elements * shape*  sizeof(Uint),  connectTable,  0, NULL, NULL);
   opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );




   // running GPGPU kernel

   /*   __kernel void interpolation(__global float* PHI, __global float* waveSpeed,
                                    __global float* A, __global float* A_ksi, __global float* A_eta,
                                    __global float* weights,
                                    __global float* X_node, __global float* U_node,
                                    __global Uint* connectTable,
                                    int shape, int quad, int dim, int elem,
                                    __local float* X_shape, __local float* U_shape,
                                    __local float* X_quad, __local float* X_ksi, __local float* X_eta )        */

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

   size_t localWorkSize[2], globalWorkSize[2];

   localWorkSize[0] = 8;
   localWorkSize[1] = 8;

   globalWorkSize[0] = 4*env.num_compute_units;
   globalWorkSize[1] = 4*env.num_compute_units;

   env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
   opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
   clFinish(env.command_queue);

   // receive data from GPGPU memory

   env.errcode  = clEnqueueReadBuffer(env.command_queue,phi_rGPGPU,     CL_TRUE, 0, nodes * sizeof(float), phi, 0, NULL, NULL);
   env.errcode |= clEnqueueReadBuffer(env.command_queue,waveSpeedGPGPU, CL_TRUE, 0, nodes * sizeof(float), waveSpeed, 0, NULL, NULL);

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

   for(Uint idx = 0; idx < nodes; idx++ )
   {
       double a= phi[idx];
       if(a< 1e-10 && a >= 0 )
           a = 1e-10;
       if(a> -1e-10 && a < 0 )
           a = -1e-10;
       (*residual)[idx][0]   = a;
        (*wave_speed)[idx][0] = waveSpeed[idx];
   }

  //std::cout<<ctimer.elapsed()<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeLDAGPU_hpp
