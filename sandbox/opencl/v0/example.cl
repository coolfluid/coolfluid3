
__kernel void 
add(__global float *a,
	__global float *b,
	__global float *answer)
{
	int gid = get_global_id(0);
	answer[gid] = a[gid] + b[gid];
}