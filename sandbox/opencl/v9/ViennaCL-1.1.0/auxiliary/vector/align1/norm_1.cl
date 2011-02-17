//helper:
void helper_norm1_parallel_reduction( __local float * tmp_buffer )
{
  for (unsigned int stride = get_global_size(0)/2; stride > 0; stride /= 2)
  {
    barrier(CLK_LOCAL_MEM_FENCE);
    if (get_global_id(0) < stride)
      tmp_buffer[get_global_id(0)] += tmp_buffer[get_global_id(0)+stride];
  }
}

////// norm_1
float impl_norm_1(
          __global const float * vec,
          unsigned int start_index,
          unsigned int end_index,
          __local float * tmp_buffer)
{
  float tmp = 0;
  for (unsigned int i = start_index + get_local_id(0); i < end_index; i += get_local_size(0))
    tmp += fabs(vec[i]);
  
  tmp_buffer[get_local_id(0)] = tmp;
  
  helper_norm1_parallel_reduction(tmp_buffer);
  
  return tmp_buffer[0];
};

__kernel void norm_1(
          __global const float * vec,
          unsigned int size,
          __local float * tmp_buffer,
          global float * group_buffer)
{
  float tmp = impl_norm_1(vec,
                          (      get_group_id(0) * size) / get_num_groups(0),
                          ((get_group_id(0) + 1) * size) / get_num_groups(0),
                          tmp_buffer);
  
  if (get_local_id(0) == 0)
    group_buffer[get_group_id(0)] = tmp;  
}

