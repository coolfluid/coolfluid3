
__kernel void mul_add(
          __global const float4 * vec1,
          __global const float * fac,
          __global const float4 * vec2,
          __global float4 * result,
          unsigned int size
          ) 
{ 
  float factor = *fac;
  for (unsigned int i = get_global_id(0); i < size/4; i += get_global_size(0))
    result[i] = vec1[i] * factor + vec2[i];
}

