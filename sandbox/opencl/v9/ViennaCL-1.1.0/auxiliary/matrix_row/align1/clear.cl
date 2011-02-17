
__kernel void clear(
          __global float * vec,
          unsigned int size) 
{ 
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
    vec[i] = 0;
}

