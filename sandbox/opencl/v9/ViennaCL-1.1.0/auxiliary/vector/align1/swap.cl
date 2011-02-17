

////// swap:
__kernel void swap(
          __global float * vec1,
          __global float * vec2,
          unsigned int size
          ) 
{ 
  float tmp;
  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
  {
    tmp = vec2[i];
    vec2[i] = vec1[i];
    vec1[i] = tmp;
  }
}
 
