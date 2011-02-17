
__kernel void cpu_inplace_mul_add(
          __global float * vec1,
          __global const float * vec2,
          float factor,
          unsigned int size
          ) 
{ 
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    vec1[i] += vec2[i] * factor;
}

