

__kernel void sum(
          __global float * vec1,
          __global float * result) 
{ 
  //parallel reduction on global memory:  
  
  for (unsigned int stride = get_global_size(0)/2; stride > 0; stride /= 2)
  {
    if (get_global_id(0) < stride)
      vec1[get_global_id(0)] += vec1[get_global_id(0)+stride];
    barrier(CLK_GLOBAL_MEM_FENCE);
  }
  
  if (get_global_id(0) == 0)
    *result = vec1[0];  
}

