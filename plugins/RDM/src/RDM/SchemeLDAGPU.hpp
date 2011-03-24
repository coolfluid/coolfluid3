// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeLDAGPU_hpp
#define CF_Solver_SchemeLDAGPU_hpp

#include <boost/assign.hpp>

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

#include "RDM/LibRDM.hpp"
#include "RDM/FluxOp2D.hpp"

#include "RDM/CLdeclaration.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS >
class RDM_API SchemeLDAGPU : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeLDAGPU > Ptr;
  typedef boost::shared_ptr< SchemeLDAGPU const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SchemeLDAGPU ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeLDAGPU() {};

  /// Get the class name
  static std::string type_name () { return "SchemeLDAGPU<" + SHAPEFUNC::type_name() + ">"; }
	
  /// execute the action
  virtual void execute ();

private: // helper functions

  void change_elements()
  { 
    /// @todo improve this (ugly)

    connectivity_table = elements().as_ptr<Mesh::CElements>()->connectivity_table().as_ptr< Mesh::CTable<Uint> >();
    coordinates = elements().nodes().coordinates().as_ptr< Mesh::CTable<Real> >();

    cf_assert( is_not_null(connectivity_table) );

    /// @todo modify these to option components configured from

    Mesh::CField::Ptr csolution = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( *Common::Core::instance().root(), "solution" );
    cf_assert( is_not_null( csolution ) );
    solution = csolution->data_ptr();

    Mesh::CField::Ptr cresidual = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( *Common::Core::instance().root(), "residual" );
    cf_assert( is_not_null( cresidual ) );
    residual = cresidual->data_ptr();

    Mesh::CField::Ptr cwave_speed = Common::find_component_ptr_recursively_with_tag<Mesh::CField>( *Common::Core::instance().root(), "wave_speed" );
    cf_assert( is_not_null( cwave_speed ) );
    wave_speed = cwave_speed->data_ptr();
  }


private: // data

  Mesh::CTable<Uint>::Ptr connectivity_table;

  Mesh::CTable<Real>::Ptr coordinates;

  Mesh::CTable<Real>::Ptr solution;
  Mesh::CTable<Real>::Ptr residual;
  Mesh::CTable<Real>::Ptr wave_speed;

  typedef FluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS> DiscreteOpType;

  DiscreteOpType m_oper;

  const QUADRATURE& m_quadrature;

  // Values of the solution located in the dof of the element
  // RealVector m_solution_values;
  typename DiscreteOpType::SolutionMatrixT m_solution_values;

  // The operator L in the advection equation Lu = f
  // Matrix m_sf_oper_values stores the value L(N_i) at each quadrature point for each shape function N_i
  typename DiscreteOpType::SFMatrixT m_sf_oper_values;

  // Values of the operator L(u) computed in quadrature points. These operator L returns these values
  // multiplied by Jacobian and quadrature weight
  RealVector m_flux_oper_values;

  // Nodal residuals
  RealVector m_phi;

  //Integration factor (jacobian multiplied by quadrature weight)
  Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> m_wj;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeLDAGPU<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeLDAGPU ( const std::string& name ) :
  CLoopOperation(name),
  m_quadrature( QUADRATURE::instance() )
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeLDAGPU<SHAPEFUNC,QUADRATURE,PHYSICS>::change_elements, this ) );

  m_flux_oper_values.resize(QUADRATURE::nb_points);
  m_phi.resize(SHAPEFUNC::nb_nodes);
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeLDAGPU<SHAPEFUNC, QUADRATURE,PHYSICS>::execute()
{


#include "RDM/LDAGPUkernel.hpp"

 std::cout << "SchemeLDAGPU" << std::endl;

 uint dim     = 2;
 uint shape   = SHAPEFUNC::nb_nodes;
 uint quad    =  QUADRATURE::nb_points;
 uint nodes   = (*coordinates).size() * dim;
 uint elements = (*connectivity_table).size();

 typename SHAPEFUNC::MappedGradientT m_sf_grad_ref; //Gradient of the shape functions in reference space
 typename SHAPEFUNC::ShapeFunctionsT m_sf_ref;   //Values of shape functions in reference space

 float A_inter[shape*quad], A_ksi[shape*quad], A_eta[shape*quad];

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

 }

 float X_real[elements*shape*dim], U_real[elements*shape],phi[elements*shape], waveSpeed[elements];

 for(uint idx = 0; idx < elements; idx++ )
 {
     for(uint idy = 0; idy < shape; idy++)
     {
         uint pos = idx * shape + idy;
         uint adress = (*connectivity_table)[idx][idy];
         if (adress > (*coordinates).size() )
             std::cout<<"error" <<std::endl;

         U_real[pos] = (*solution)[adress][0];
         (*residual)[adress][0] = 0;
         (*wave_speed)[adress][0] = 0;
         phi[pos]    = 0;

         //std::cout<<adress << " ";

         for( uint idz = 0; idz < dim; idz++ )
         {
            X_real[ pos * dim + idz ] = (*coordinates)[adress][idz];
         }
     }

     waveSpeed[idx]=0;

 }

 float weights[quad];

 for( int idx = 0; idx < quad; idx++ )
 {
     weights[idx] = 0.5;//m_wj[idx];
 }

 CLEnv env;
 GPGPU_setup(env);


 // OpenCL kernel compilation

 env.program = clCreateProgramWithSource(env.context, 1, (const char**)&GPUSource, NULL, &env.errcode);
 opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

 env.errcode = clBuildProgram(env.program, 0,  NULL, "-cl-fast-relaxed-math", NULL, NULL);
 opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

 env.kernel = clCreateKernel(env.program, "interpolation", &env.errcode);
 opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

 clReleaseProgram(env.program);

 //GPGPU mem reservation

 cl_mem A_iGPGPU, A_kGPGPU, A_eGPGPU;
 cl_mem X_rGPGPU, U_rGPGPU, phi_rGPGPU;
 cl_mem weightsGPGPU, waveSpeedGPGPU;
// copy matrix to GPGPU memory


 A_iGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY , shape * quad * sizeof(float), A_inter, &env.errcode);
 A_kGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY , shape * quad * sizeof(float), A_ksi,   &env.errcode);
 A_eGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY , shape * quad * sizeof(float), A_eta,   &env.errcode);
 env.errcode  = clEnqueueWriteBuffer(env.command_queue, A_iGPGPU, CL_FALSE, 0, shape * quad * sizeof(float), A_inter,    0, NULL, NULL);
 env.errcode |= clEnqueueWriteBuffer(env.command_queue, A_kGPGPU, CL_FALSE, 0, shape * quad * sizeof(float), A_ksi,      0, NULL, NULL);
 env.errcode |= clEnqueueWriteBuffer(env.command_queue, A_eGPGPU, CL_FALSE, 0, shape * quad * sizeof(float), A_eta,      0, NULL, NULL);
 opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

 // copy vector to GPGPU memory

 X_rGPGPU         =   clCreateBuffer(env.context, CL_MEM_READ_ONLY , elements * shape * dim * sizeof(float) ,    X_real, &env.errcode);
 U_rGPGPU         =   clCreateBuffer(env.context, CL_MEM_READ_ONLY , elements * shape * sizeof(float)       ,    U_real, &env.errcode);
 weightsGPGPU     =   clCreateBuffer(env.context, CL_MEM_READ_ONLY,  quad * sizeof(float)                   ,    weights,    &env.errcode);
 phi_rGPGPU       =   clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, elements * shape * sizeof(float)       ,    phi,    &env.errcode);
 waveSpeedGPGPU   =   clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, elements * sizeof(float)               ,    waveSpeed,    &env.errcode);


 env.errcode |= clEnqueueWriteBuffer(env.command_queue, X_rGPGPU,    CL_FALSE, 0, elements * shape * dim * sizeof(float),     X_real,     0, NULL, NULL);
 env.errcode |= clEnqueueWriteBuffer(env.command_queue, U_rGPGPU,    CL_FALSE, 0, elements * shape * sizeof(float),           U_real,     0, NULL, NULL);
 env.errcode |= clEnqueueWriteBuffer(env.command_queue, phi_rGPGPU,  CL_FALSE, 0, elements * shape * sizeof(float),           phi,        0, NULL, NULL);
 env.errcode |= clEnqueueWriteBuffer(env.command_queue, weightsGPGPU,  CL_FALSE, 0, quad * sizeof(float),                     weights,    0, NULL, NULL);
 env.errcode |= clEnqueueWriteBuffer(env.command_queue, waveSpeedGPGPU,  CL_FALSE, 0, elements *  sizeof(float),              waveSpeed,  0, NULL, NULL);
 opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

 // running GPGPU kernel

 /* __kernel void interpolation(__global float* X_quad, __global float* X_ksi, __global float* X_eta,
                                __global float* A, __global float* A_ksi, __global float* A_eta,
                                __global float* X_shape,
                                 int shape, int quad, int dim, int elem )                             */

 int n = 0;
 env.errcode  = clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&phi_rGPGPU);
 env.errcode  = clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&waveSpeedGPGPU);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_iGPGPU);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_kGPGPU);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_eGPGPU);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&X_rGPGPU);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&U_rGPGPU);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&weightsGPGPU);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&shape);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&quad);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&dim);
 env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&elements);
 env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim * sizeof(float), 0);
 env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim * sizeof(float), 0);
 env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim * sizeof(float), 0);
 env.errcode |= clSetKernelArg(env.kernel, n++, quad * sizeof(float), 0);
 env.errcode |= clSetKernelArg(env.kernel, n++, quad * sizeof(float), 0);
 env.errcode |= clSetKernelArg(env.kernel, n++, quad * sizeof(float), 0);

 opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

 size_t localWorkSize[2], globalWorkSize[2];

 localWorkSize[0] = 1;
 localWorkSize[1] = 1;

 globalWorkSize[0] = env.num_compute_units;
 globalWorkSize[1] = env.num_compute_units;

 env.errcode = clEnqueueNDRangeKernel(env.command_queue, env.kernel, 2, globalWorkSize, localWorkSize, NULL, 0, NULL, NULL);
 opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );
 clFinish(env.command_queue);

 // receive data from GPGPU memory

 env.errcode  = clEnqueueReadBuffer(env.command_queue,phi_rGPGPU,     CL_TRUE, 0, elements*shape * sizeof(float), phi, 0, NULL, NULL);
 env.errcode |= clEnqueueReadBuffer(env.command_queue,waveSpeedGPGPU, CL_TRUE, 0, elements * sizeof(float)      , waveSpeed, 0, NULL, NULL);

 opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

 // release GPGPU memory for matrix

 clReleaseMemObject( A_iGPGPU );
 clReleaseMemObject( A_kGPGPU );
 clReleaseMemObject( A_eGPGPU );

 // release GPGPU memory for vectors

 clReleaseMemObject( X_rGPGPU );
 clReleaseMemObject( U_rGPGPU );
 clReleaseMemObject( phi_rGPGPU );
 clReleaseMemObject( weightsGPGPU );
 clReleaseMemObject( waveSpeedGPGPU );

 // free GPU

 clReleaseContext(env.context);
 clReleaseKernel(env.kernel);
 clReleaseCommandQueue(env.command_queue);



 for(uint idx = 0; idx < elements; idx++ )
 {
     for(uint idy = 0; idy < shape; idy++)
     {
         uint pos = idx * shape + idy;
         uint adress = (*connectivity_table)[idx][idy];

         (*residual)[adress][0] += phi[pos];
         (*wave_speed)[adress][0] += waveSpeed[idx];
     }
 }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeLDAGPU_hpp
