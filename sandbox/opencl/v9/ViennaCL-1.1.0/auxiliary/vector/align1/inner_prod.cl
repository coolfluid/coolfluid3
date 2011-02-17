
//helper:
void helper_inner_prod_parallel_reduction( __local float * tmp_buffer )
{
  for (unsigned int stride = get_local_size(0)/2; stride > 0; stride /= 2)
  {
    barrier(CLK_LOCAL_MEM_FENCE);
    if (get_local_id(0) < stride)
      tmp_buffer[get_local_id(0)] += tmp_buffer[get_local_id(0)+stride];
  }
}

//////// inner products:
float impl_inner_prod(
          __global const float * vec1,
          __global const float * vec2,
          unsigned int start_index,
          unsigned int end_index,
          __local float * tmp_buffer)
{
  float tmp = 0;
  for (unsigned int i = start_index + get_local_id(0); i < end_index; i += get_local_size(0))
    tmp += vec1[i] * vec2[i];
  tmp_buffer[get_local_id(0)] = tmp;
  
  helper_inner_prod_parallel_reduction(tmp_buffer);
  
  return tmp_buffer[0];
}


__kernel void inner_prod(
          __global const float * vec1,
          __global const float * vec2,
          unsigned int size,
          __local float * tmp_buffer,
          global float * group_buffer)
{
  float tmp = impl_inner_prod(vec1,
                              vec2,
                              (      get_group_id(0) * size) / get_num_groups(0),
                              ((get_group_id(0) + 1) * size) / get_num_groups(0),
                              tmp_buffer);
  
  if (get_local_id(0) == 0)
    group_buffer[get_group_id(0)] = tmp;
  
}

