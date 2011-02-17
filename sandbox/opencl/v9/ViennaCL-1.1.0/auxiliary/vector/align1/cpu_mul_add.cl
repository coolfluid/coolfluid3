
__kernel void cpu_mul_add(
          __global const float * vec1,
          float factor,
          __global const float * vec2,
          __global float * result,
          unsigned int size
          ) 
{ 
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    result[i] = vec1[i] * factor + vec2[i];
}

