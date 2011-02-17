
////// plane rotation: (x,y) <- (\alpha x + \beta y, -\beta x + \alpha y)
__kernel void plane_rotation(
          __global float * vec1,
          __global float * vec2, 
          float alpha,
          float beta,
          unsigned int size) 
{ 
  float tmp1 = 0;
  float tmp2 = 0;

  for (unsigned int i = get_global_id(0); i < size; i += get_global_size(0))
  {
    tmp1 = vec1[i];
    tmp2 = vec2[i];
    
    vec1[i] = alpha * tmp1 + beta * tmp2;
    vec2[i] = alpha * tmp2 - beta * tmp1;
  }

}

