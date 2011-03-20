// OpenCL Kernel
const char* GPUSource = std::string(
"    #pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable \n"
"    #pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable\n"
"    #pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable\n"
"    #pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable\n"

"    __kernel void interpolation(__global float* PHI, \n"
"                                __global float* A, __global float* A_ksi, __global float* A_eta, \n"
"                                __global float* X_real, __global float* U_real, \n"
"                                __global uint* adress, __global uint* atomic,\n"
"                                unsigned int shape, unsigned int quad, unsigned int dim, unsigned int elements, \n"
"                                __local float* X_elem, __local float* U_elem,\n"
"                                __local float* X_quad, __local float* X_ksi, __local float* X_eta,\n"
"                                __local float* jacobi_quad, __local float* sum_phi_quad , __local float* LU_quad )\n"
"    {\n"
"    for( unsigned int tx = get_group_id(0); tx <elements; tx += get_num_groups(0) )\n"
"    {\n"
"          // assigment coordinates, solution to nodes\n"
"          for(unsigned int j = get_local_id(0);j< shape;j+= get_local_size(0) )\n"
"          {\n"
"                unsigned int adr = tx*shape+j;\n"
"                \n"
"                for( unsigned int k = 0; k < dim; k++ )\n"
"                {\n"
"                    unsigned int adr1 = adress[adr]*dim + k;\n"
"                    X_elem[j*dim + k] = X_real[adr1];\n"
"                }\n"
"                \n"
"                U_elem[j] = U_real[adress[adr]];\n"
"          }\n"
"          barrier(CLK_LOCAL_MEM_FENCE);\n"

      
      
      // interpolation to quadrature points
      
"          for(unsigned int j = get_local_id(0);j< quad;j+= get_local_size(0) )\n"
"          {\n"
"                for( unsigned int k = 0; k < dim; k++ )\n"
"                {\n"
"                    unsigned int elemC = j * dim + k;\n"
"                    \n"
"                    float value_int = 0;\n"
"                    float value_ksi = 0;\n"
"                    float value_eta = 0;\n"
"                    \n"
"                    for( unsigned int l = 0; l < shape; l++ )\n"
"                    {\n"
"                        unsigned int elemA = j * shape + l;\n"
"                        unsigned int elemB = l * dim + k;\n"

"                        value_int += A[elemA]     * X_real[elemB];\n"
"                        value_ksi += A_ksi[elemA] * X_real[elemB];\n"
"                        value_eta += A_eta[elemA] * X_real[elemB];\n"
"                    }\n"
"                    X_quad[elemC] = value_int;\n"
"                    X_ksi[elemC]  = value_ksi;\n"
"                    X_eta[elemC]  = value_eta;\n"
"                }\n"
"          }\n"
"          barrier(CLK_LOCAL_MEM_FENCE);       \n"

    // jacobi in quad points
      
"          for(unsigned int j = get_local_id(0);j< quad;j+= get_local_size(0) )\n"
"          {\n"
"                unsigned int elemC = j;\n"
"                unsigned int elemA = j * dim;\n"
"                float value = X_ksi[elemA] * X_eta[elemA+1] - X_eta[elemA] * X_eta[elemA+1];\n"

"                jacobi_quad[elemC] = value;\n"
"          }\n"
"          barrier(CLK_LOCAL_MEM_FENCE);\n"
      
      // sum_phi and LU in quad points
      
"          for(unsigned int j = get_local_id(0);j< quad;j+= get_local_size(0) )\n"
"          {\n"
"                unsigned int elemC = j;\n"
"                float value = 0;\n"
"                float value1 = 0;\n"
            
"                for( int k = 0; k < shape; k++ )\n"
"                {\n"
"                    unsigned int elemMatrix = j * shape + k;\n"
"                    unsigned int elemA      = j * dim;\n"
                
                
"                    float phiX =  A_ksi[elemMatrix] * X_eta[elemA+1] - A_eta[elemMatrix] * X_ksi[elemA+1];\n"
"                    float phiY = -A_ksi[elemMatrix] * X_eta[elemA]   + A_eta[elemMatrix] * X_ksi[elemA];\n"
                
                
"                    value += 1.0 * ( X_quad[elemA+1] * phiX  - X_quad[elemA] * phiY );\n"
"                    value1 += 1.0 * ( X_quad[elemA+1] * phiX  - X_quad[elemA] * phiY )*U_real[k];\n"
"                }\n"
"                sum_phi_quad[elemC] = value / jacobi_quad[j];\n"
"                LU_quad[elemC] = value1 / jacobi_quad[j];\n"
"          }\n"
"          barrier(CLK_LOCAL_MEM_FENCE);\n"
      
      // PHI in real points
    
"          for(unsigned int j = get_local_id(0);j< shape;j+= get_local_size(0) )\n"
"          {\n"
"                unsigned int adr1 = adress[tx*shape+j];\n"
"                float value = 0;\n"
            
"                for( int k = 0; k < quad; k++ )\n"
"                {\n"
"                    unsigned int elemMatrix = k * shape + j;\n"
"                    unsigned int elemA      = k * dim;\n"
                
"                    float phiX =  A_ksi[elemMatrix] * X_eta[elemA+1] - A_eta[elemMatrix] * X_ksi[elemA+1];\n"
"                    float phiY = -A_ksi[elemMatrix] * X_eta[elemA]   + A_eta[elemMatrix] * X_ksi[elemA];\n"
                
"                    value += 1.0 / jacobi_quad[k] * ( X_quad[elemA+1] * phiX  - X_quad[elemA] * phiY ) * LU_quad[k] /  sum_phi_quad[k]; \n"
"                }\n"
            
"                while( atomic[adr1] != 0 )\n"
"                {\n"
"                }\n"
            
"                atom_xchg(&atomic[adr1], 1);\n"
"                PHI   [adr1] += value;\n"
"                atom_xchg (&atomic[adr1], 0);\n"
"          }\n"
"    }\n"
"    }\n").c_str();

