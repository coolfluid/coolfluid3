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
#include "Mesh/CField2.hpp"
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

    Mesh::CField2::Ptr csolution = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "solution" );
    cf_assert( is_not_null( csolution ) );
    solution = csolution->data_ptr();

    Mesh::CField2::Ptr cresidual = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "residual" );
    cf_assert( is_not_null( cresidual ) );
    residual = cresidual->data_ptr();

    Mesh::CField2::Ptr cwave_speed = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "wave_speed" );
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
  CLoopOperation(name)
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
 Uint*  adress = connectivity_table->array().origin();
 //float* coordinates_real = coordinates->array().origin();


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
     cl_mem adressGPGPU, atomicGPGPU;

     // copy matrix to GPGPU memory
     Uint nodes, shape,quad, elements, dim;


     /*
     A_iGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY , shape * quad * sizeof(float), A_inter, &env.errcode);
     A_kGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY , shape * quad * sizeof(float), A_ksi,   &env.errcode);
     A_eGPGPU = clCreateBuffer(env.context, CL_MEM_READ_ONLY , shape * quad * sizeof(float), A_eta,   &env.errcode);
     env.errcode  = clEnqueueWriteBuffer(env.command_queue, A_iGPGPU, CL_FALSE, 0, shape * quad * sizeof(float), A_inter,    0, NULL, NULL);
     env.errcode |= clEnqueueWriteBuffer(env.command_queue, A_kGPGPU, CL_FALSE, 0, shape * quad * sizeof(float), A_ksi,      0, NULL, NULL);
     env.errcode |= clEnqueueWriteBuffer(env.command_queue, A_eGPGPU, CL_FALSE, 0, shape * quad * sizeof(float), A_eta,      0, NULL, NULL);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     // copy vector to GPGPU memory

     X_rGPGPU   =    clCreateBuffer(env.context, CL_MEM_READ_ONLY , nodes * dim * sizeof(float) ,    X_real, &env.errcode);
     U_rGPGPU   =    clCreateBuffer(env.context, CL_MEM_READ_ONLY , nodes * sizeof(float)       ,    U_real, &env.errcode);
     phi_rGPGPU =    clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, nodes * sizeof(float)       ,    phi,    &env.errcode);
     adressGPGPU =   clCreateBuffer(env.context, CL_MEM_WRITE_ONLY, elements * shape * sizeof(uint), adress, &env.errcode);
     atomicGPGPU =   clCreateBuffer(env.context, CL_MEM_READ_WRITE, nodes * sizeof(uint),            atomic, &env.errcode);
     env.errcode |= clEnqueueWriteBuffer(env.command_queue, X_rGPGPU,    CL_FALSE, 0, nodes * dim * sizeof(float),     X_real, 0, NULL, NULL);
     env.errcode |= clEnqueueWriteBuffer(env.command_queue, U_rGPGPU,    CL_FALSE, 0, nodes * sizeof(float),           U_real, 0, NULL, NULL);
     env.errcode |= clEnqueueWriteBuffer(env.command_queue, phi_rGPGPU,  CL_FALSE, 0, nodes * sizeof(float),           phi,    0, NULL, NULL);
     env.errcode |= clEnqueueWriteBuffer(env.command_queue, adressGPGPU, CL_FALSE, 0, elements * shape * sizeof(uint), adress, 0, NULL, NULL);
     env.errcode |= clEnqueueWriteBuffer(env.command_queue, atomicGPGPU, CL_FALSE, 0, nodes * sizeof(uint),            atomic, 0, NULL, NULL);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     // running GPGPU kernel

     /*__kernel void interpolation(__global float* PHI,
                             __global float* A, __global float* A_ksi, __global float* A_eta,
                             __global float* X_real, __global float* U_real,
                             __global uint* adress, __global uint* atomic,
                             unsigned int shape, unsigned int quad, unsigned int dim, unsigned int elements,
                             __local float* X_elem, __local float* U_elem,
                             __local float* X_quad, __local float* X_ksi, __local float* X_eta,
                             __local float* jacobi_quad, __local float* sum_phi_quad , __local float* LU_quad )*/



     /*
     int n = 0;
     env.errcode  = clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&phi_rGPGPU);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_iGPGPU);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_kGPGPU);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&A_eGPGPU);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&X_rGPGPU);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&U_rGPGPU);

     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&adressGPGPU);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(cl_mem), (void *)&atomicGPGPU);

     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&shape);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&quad);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&dim);
     env.errcode |= clSetKernelArg(env.kernel, n++, sizeof(int),    (void *)&elements);

     env.errcode |= clSetKernelArg(env.kernel, n++, shape * dim * sizeof(float), 0);
     env.errcode |= clSetKernelArg(env.kernel, n++, shape * sizeof(float), 0);

     env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim * sizeof(float), 0);
     env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim * sizeof(float),       0);
     env.errcode |= clSetKernelArg(env.kernel, n++, quad * dim * sizeof(float),       0);

     env.errcode |= clSetKernelArg(env.kernel, n++, quad * sizeof(float),       0);
     env.errcode |= clSetKernelArg(env.kernel, n++, quad * sizeof(float),       0);
     env.errcode |= clSetKernelArg(env.kernel, n++, quad * sizeof(float),       0);
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

     env.errcode = clEnqueueReadBuffer(env.command_queue,phi_rGPGPU, CL_TRUE, 0, shape * elements * sizeof(float), phi, 0, NULL, NULL);
     opencl_check_error(env.errcode, CL_SUCCESS, __FILE__ , __LINE__ );

     // release GPGPU memory for matrix

     clReleaseMemObject( A_iGPGPU );
     clReleaseMemObject( A_kGPGPU );
     clReleaseMemObject( A_eGPGPU );

     // release GPGPU memory for vectors

     clReleaseMemObject( X_rGPGPU );
     clReleaseMemObject( U_rGPGPU );
     clReleaseMemObject( phi_rGPGPU );
*/
     // free GPU

     clReleaseContext(env.context);
     clReleaseKernel(env.kernel);
     clReleaseCommandQueue(env.command_queue);




/*
//  CFinfo << "LDA ELEM [" << idx() << "]" << CFendl;

  const Mesh::CTable<Uint>::ConstRow nodes_idx = connectivity_table->array()[idx()];

//  std::cout << "nodes_idx";
//  for ( Uint i = 0; i < nodes_idx.size(); ++i)
//     std::cout << " " << nodes_idx[i];

  typename SHAPEFUNC::NodeMatrixT nodes;

 Mesh::fill(nodes, *coordinates, nodes_idx );

//  elements().as_ptr<Mesh::CElements>()->put_coordinates( nodes, idx() );

//  std::cout << "field put_coordinates function" <<  std::endl;
//  std::cout << "nodes: " << nodes << std::endl;


//  std::cout << "mesh::fill function" <<  std::endl;
//  std::cout << "nodes: " << nodes << std::endl;


  // copy the solution from the large array to a small
  for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
    m_solution_values[n] = (*solution)[nodes_idx[n]][0];


 m_phi.setZero();

 m_oper.compute(nodes,m_solution_values, m_sf_oper_values, m_flux_oper_values,m_wj);

// std::cout << "solution_values  [" << m_solution_values << "]" << std::endl;
// std::cout << "sf_oper_values   [" << m_sf_oper_values << "]" << std::endl;
// std::cout << std::endl;
// std::cout << "flux_oper_values [" << m_flux_oper_values << "]" << std::endl;
//// if (m_flux_oper_values.norm() > 0.0)
////     std::cin.get();
// std::cout << std::endl;

 for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
 {
   Real sumLplus = 0.0;
   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     sumLplus += std::max(0.0,m_sf_oper_values(q,n));
   }

   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   {
     m_phi[n] += std::max(0.0,m_sf_oper_values(q,n))/sumLplus * m_flux_oper_values[q] * m_wj[q];
   }
 }
  
//   std::cout << "phi [";
//   for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
//      std::cout << m_phi[n] << " ";
//   std::cout << "]" << std::endl;

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    (*residual)[nodes_idx[n]][0] += m_phi[n];

//  std::cout << "residual [";
//  for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
//     std::cout << (*residual)[nodes_idx[n]][0] << " ";
//  std::cout << "]" << std::endl;


  // computing average advection speed on element

	typename SHAPEFUNC::CoordsT centroid;
	
	centroid.setZero();

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
  {
    centroid[XX] += nodes(n, XX);
    centroid[YY] += nodes(n, YY);
  }
  centroid /= SHAPEFUNC::nb_nodes;


  // compute a bounding box of the element:

  Real xmin = nodes(0, XX);
  Real xmax = nodes(0, XX);
  Real ymin = nodes(0, YY);
  Real ymax = nodes(0, YY);

  for(Uint inode = 1; inode < SHAPEFUNC::nb_nodes; ++inode)
  {
    xmin = std::min(xmin,nodes(inode, XX));
    xmax = std::max(xmax,nodes(inode, XX));

    ymin = std::min(ymin,nodes(inode, YY));
    ymax = std::max(ymax,nodes(inode, YY));

  }

  const Real dx = xmax - xmin;
  const Real dy = ymax - ymin;

  // The update coeff is updated by a product of bb radius and norm of advection velocity
  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
  {
    (*wave_speed)[nodes_idx[n]][0] += std::sqrt( dx*dx+dy*dy);
//       * std::sqrt( centroid[XX]*centroid[XX] + centroid[YY]*centroid[YY] );
  }


//  std::cout << "wave_speed [";
//  for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
//     std::cout << (*wave_speed)[nodes_idx[n]][0] << " ";
//  std::cout << "]" << std::endl;

//  std::cout << " --------------------------------------------------------------- " << std::endl;
*/



}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeLDAGPU_hpp
