
__kernel void inplace_add(
          __global float16 * vec1,
          __global const float16 * vec2,
          unsigned int size) 
{ 
  for (unsigned int i = get_global_id(0); i < size/16; i += get_global_size(0))
    vec1[i] += vec2[i];
}

