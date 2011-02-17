
__kernel void inplace_mul_add(
          __global float4 * vec1,
          __global const float4 * vec2,
          __global const float * fac,
          unsigned int size
          ) 
{ 
  float factor = *fac;
  unsigned int size_div_4 = size >> 2;
  for (unsigned int i = get_global_id(0); i < size_div_4; i += get_global_size(0))
    vec1[i] += vec2[i] * factor;
}

