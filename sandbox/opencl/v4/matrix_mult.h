#ifndef matrix_mult_h
#define matrix_mult_h

#include <cstdlib>
#include <boost/multi_array.hpp>
typedef boost::multi_array<float, 2> array_2D; // list of matricies

void mat_mul(array_2D matrix_A, array_2D matrix_B, array_2D &matrix_C, int n_blocks );
void cpu_multiplication( array_2D matrix_A, array_2D matrix_B, array_2D &matrix_C, int n_block );

#endif // matrix_mult_h
