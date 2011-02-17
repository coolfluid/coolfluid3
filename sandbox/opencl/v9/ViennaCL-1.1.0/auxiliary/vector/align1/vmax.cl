

__kernel void vmax(
          __global float * vec1,
          __global float * result,
          unsigned int size) 
{ 
  //parallel reduction on global memory:
  for (unsigned int stride = get_global_size(0)/2; stride > 0; stride /= 2)
  {
    if (get_global_id(0) < stride)
      vec1[get_global_id(0)] = fmax(vec1[get_global_id(0)+stride], vec1[get_global_id(0)]);
    barrier(CLK_GLOBAL_MEM_FENCE);
  }
  
  if (get_global_id(0) == 0)
    *result = vec1[0];  
}

