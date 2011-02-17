
__kernel void cpu_mul_add(
          __global const float4 * vec1,
          float factor,
          __global const float4 * vec2,
          __global float4 * result,
          unsigned int size
          ) 
{ 
  for (unsigned int i = get_global_id(0); i < size/4; i += get_global_size(0))
    result[i] = vec1[i] * factor + vec2[i];
}

