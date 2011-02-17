
__kernel void inplace_mult(
          __global float16 * vec,
          __global const float * fac, 
          unsigned int size) 
{ 
  float factor = *fac;
  for (unsigned int i = get_global_id(0); i < size/16; i += get_global_size(0))
    vec[i] *= factor;
}

