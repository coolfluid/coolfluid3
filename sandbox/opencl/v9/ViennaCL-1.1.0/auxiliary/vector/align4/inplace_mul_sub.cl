
__kernel void inplace_mul_sub(
          __global float4 * vec1,
          __global const float4 * vec2,
          __global const float * fac,   //CPU variant is mapped to mult_add
          unsigned int size
          ) 
{ 
  float factor = *fac;
  for (unsigned int i = get_global_id(0); i < size/4; i += get_global_size(0))
    vec1[i] -= vec2[i] * factor;
}


