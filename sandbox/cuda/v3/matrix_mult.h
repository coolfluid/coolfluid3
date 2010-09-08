#ifndef matrix_mult_h
#define matrix_mult_h

#ifdef __cplusplus
extern "C" {
#endif

void gpu_mat_mul(float* h_A, float* h_B, float* h_C );

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // matrix_mult_h
