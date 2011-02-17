
__kernel void sub(
          __global const float * vec1,
          __global const float * vec2, 
          __global float * result,
          unsigned int size)
{ 
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    result[i] = vec1[i] - vec2[i];
}

