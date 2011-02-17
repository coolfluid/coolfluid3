
__kernel void mult(
          __global const float * vec,
          __global const float * fac, 
          __global float * result,
          unsigned int size) 
{ 
  float factor = *fac;
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    result[i] = vec[i] * factor;
}

