
__kernel void inplace_divide(
          __global float * vec,
          __global const float * fac,  //note: CPU variant is mapped to prod_scalar
          unsigned int size) 
{ 
  float factor = *fac;
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    vec[i] /= factor;
}

