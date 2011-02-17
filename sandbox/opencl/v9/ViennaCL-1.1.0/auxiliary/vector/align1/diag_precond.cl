
__kernel void diag_precond(
          __global const float * diag_A_inv, 
          __global float * x, 
          unsigned int size) 
{ 
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    x[i] *= diag_A_inv[i];
}
