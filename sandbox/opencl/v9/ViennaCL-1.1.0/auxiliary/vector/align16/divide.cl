
//Note: 'div' cannot be used because of complaints by the jit-compiler
__kernel void divide(
          __global const float16 * vec,
          __global const float * fac,  //note: CPU variant is mapped to prod_scalar
          __global float16 * result,
          unsigned int size)  
{ 
  float factor = *fac;
  for (unsigned int i = get_global_id(0); i < size/16; i += get_global_size(0))
    result[i] = vec[i] / factor;
}


