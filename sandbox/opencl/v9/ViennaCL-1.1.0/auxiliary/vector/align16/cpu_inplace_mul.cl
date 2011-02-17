
__kernel void cpu_inplace_mult(
          __global float16 * vec,
          float factor, 
          unsigned int size) 
{ 
  for (unsigned int i = get_global_id(0); i < size/16; i += get_global_size(0))
    vec[i] *= factor;
}

