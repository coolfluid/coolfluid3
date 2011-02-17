
__kernel void cpu_mult(
          __global const float * vec,
          float factor, 
          __global float * result,
          unsigned int size) 
{ 
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    result[i] = vec[i] * factor;
}


