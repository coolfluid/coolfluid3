
__kernel void mult(
          __global const float16 * vec,
          __global const float * fac, 
          __global float16 * result,
          unsigned int size) 
{ 
  float factor = *fac;
  for (unsigned int i = get_global_id(0); i < size/16; i += get_global_size(0))
    result[i] = vec[i] * factor;
}

