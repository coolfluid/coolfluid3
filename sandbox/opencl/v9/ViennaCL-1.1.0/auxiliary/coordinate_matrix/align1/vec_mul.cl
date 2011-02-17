
//segmented parallel reduction. At present restricted to up to 256 threads
void segmented_parallel_reduction(unsigned int row, 
                                  float val, 
                                  __local unsigned int * shared_rows, 
                                  __local float * inter_results) 
{ 
  barrier(CLK_LOCAL_MEM_FENCE); 
  shared_rows[get_local_id(0)] = row; 
  inter_results[get_local_id(0)] = val; 
 
  barrier(CLK_LOCAL_MEM_FENCE); 
  if( get_local_id(0) >=  1 && row == shared_rows[get_local_id(0) -  1] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) -  1]; }  
  barrier(CLK_LOCAL_MEM_FENCE); 
  if( get_local_id(0) >=  2 && row == shared_rows[get_local_id(0) -  2] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) -  2]; } 
  barrier(CLK_LOCAL_MEM_FENCE); 
  if( get_local_id(0) >=  4 && row == shared_rows[get_local_id(0) -  4] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) -  4]; } 
  barrier(CLK_LOCAL_MEM_FENCE); 
  if( get_local_id(0) >=  8 && row == shared_rows[get_local_id(0) -  8] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) -  8]; } 
  barrier(CLK_LOCAL_MEM_FENCE); 
  if( get_local_id(0) >= 16 && row == shared_rows[get_local_id(0) - 16] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) - 16]; } 
  barrier(CLK_LOCAL_MEM_FENCE); 
  if( get_local_id(0) >= 32 && row == shared_rows[get_local_id(0) - 32] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) - 32]; } 
  barrier(CLK_LOCAL_MEM_FENCE); 
  if( get_local_id(0) >= 64 && row == shared_rows[get_local_id(0) - 64] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) - 64]; } 
  barrier(CLK_LOCAL_MEM_FENCE); 
  if( get_local_id(0) >= 128 && row == shared_rows[get_local_id(0) - 128] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) - 128]; } 
  barrier(CLK_LOCAL_MEM_FENCE); 
  //if( get_local_id(0) >= 256 && row == shared_rows[get_local_id(0) - 256] ) { inter_results[get_local_id(0)] += inter_results[get_local_id(0) - 256]; } 
  //barrier(CLK_LOCAL_MEM_FENCE); 
 
}


void impl_vec_mul( 
          __global const uint2 * coords, //(row_index, column_index) 
          __global const float * elements, 
          __global const float * vector,  
          __global float * result, 
          unsigned int size, 
          __local unsigned int * shared_rows, 
          __local float * inter_results) 
{
  uint2 tmp; 
  float val; 
  const uint last_index = get_local_size(0) - 1; 
  shared_rows[get_local_id(0)] = 0; 
  inter_results[get_local_id(0)] = 0.0f; 
  unsigned int for_size = (size - 1) / get_local_size(0); 
  
  for (unsigned int k = 0; k <= for_size; ++k)
  { 
    unsigned int index = k * get_local_size(0) + get_local_id(0); 
    barrier(CLK_GLOBAL_MEM_FENCE);
  
    if (index < size)
    {
      tmp = coords[index]; 
      val = elements[index] * vector[tmp.y]; 
      
      if (get_local_id(0) == 0) 
      { 
        //check for carry from previous loop run: 
        if (tmp.x == shared_rows[last_index]) 
          val += inter_results[last_index]; 
        else 
          result[shared_rows[last_index]] += inter_results[last_index]; 
      } 
      segmented_parallel_reduction(tmp.x, val, shared_rows, inter_results); 
    }
    else
      segmented_parallel_reduction(0, 0, shared_rows, inter_results); //all threads have to enter this function
     
    barrier(CLK_GLOBAL_MEM_FENCE);
  
    if (get_local_id(0) != last_index &&
         shared_rows[get_local_id(0)] != shared_rows[get_local_id(0) + 1] &&
         inter_results[get_local_id(0)] != 0) 
    { 
      result[tmp.x] += inter_results[get_local_id(0)]; 
    }
  } //for 
   
  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE); 
   
  if (get_local_id(0) == last_index && inter_results[get_local_id(0)] != 0) 
    result[shared_rows[get_local_id(0)]] += inter_results[get_local_id(0)]; 
}


__kernel void vec_mul( 
          __global const uint2 * coords, //(row_index, column_index) 
          __global const float * elements, 
          __global const float * vector,  
          __global float * result, 
          unsigned int size, 
          __local unsigned int * shared_rows, 
          __local float * inter_results) 
{ 
  
  __global const uint2 * group_coords_start   = coords   + (size * get_group_id(0))       / get_num_groups(0);
  __global const uint2 * group_coords_end     = coords   + (size * (get_group_id(0) + 1)) / get_num_groups(0);
  __global const float * group_elements_start = elements + (size * get_group_id(0))       / get_num_groups(0);
  unsigned int group_size = (size * (get_group_id(0) + 1)) / get_num_groups(0) - (size * get_group_id(0)) / get_num_groups(0);
  
  //find suitable start:
  uint2 tmp;
  if (group_coords_start != coords) //first pointer stays at start, others search for start of next row
  {
    tmp = *group_coords_start;
    while ( (*group_coords_start).x == tmp.x && 
            (group_coords_start != group_coords_end) )
    {
      ++group_coords_start;
      ++group_elements_start;
      --group_size;
    }
  }
  //if the row index does not change within the group, then group is inactive.
  
  
  //find suitable end for remaining active groups:
  if (group_coords_start != group_coords_end)
  {
    if (group_coords_end != coords + size)   //do not increment or dereference beyond available array length
    {
      tmp = *group_coords_end;
      while ( (*group_coords_end).x == tmp.x && 
              (group_coords_end != coords + size) )
      {
        ++group_coords_end;
        ++group_size;
      }
    }

    //run workers:
    impl_vec_mul( group_coords_start,    //coords pointer
                  group_elements_start,  //elements pointer
                  vector,    //vector pointer
                  result,    //result pointer
                  group_size,  //size of group (unsigned int)
                  shared_rows, //local row buffer
                  inter_results); //local result buffer
  }
}