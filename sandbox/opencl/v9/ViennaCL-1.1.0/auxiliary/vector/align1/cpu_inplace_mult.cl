
__kernel void cpu_inplace_mult(
          __global float * vec,
          float factor, 
          unsigned int size) 
{ 
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    vec[i] *= factor;
}

