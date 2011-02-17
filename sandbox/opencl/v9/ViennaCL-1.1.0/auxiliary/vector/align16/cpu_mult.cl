
__kernel void cpu_mult(
          __global const float16 * vec,
          float factor, 
          __global float16 * result,
          unsigned int size) 
{ 
  for (unsigned int i = get_global_id(0); i < size/16; i += get_global_size(0))
    result[i] = vec[i] * factor;
}

