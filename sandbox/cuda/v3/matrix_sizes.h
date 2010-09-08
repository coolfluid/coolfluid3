#ifndef MATRIX_SIZES_H
#define MATRIX_SIZES_H

// Thread block size
#define BLOCK_SIZE 16
#define TILE_SIZE  16

#define WA 2048   // Matrix A width
#define HA 2048   // Matrix A height
#define WB 2048   // Matrix B width
#define HB WA     // Matrix B height
#define WC WB     // Matrix C width
#define HC HA     // Matrix C height

#endif // MATRIX_SIZES_H
