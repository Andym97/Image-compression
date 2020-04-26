#include <string.h>

#include <a2blocked.h>
#include "array2b.h"


// define a private version of each function in A2Methods_T that we implement

typedef A2Methods_Array2 A2; // private abbreviation

static A2 new(int width, int height, int size) {
  return Array2b_new_64K_block(width, height, size);
}

static A2 new_with_blocksize(int width, int height, int size, int blocksize) {
  return Array2b_new(width, height, size, blocksize);
}

static void a2free (A2 *array2p) {
  Array2b_free((Array2b_T *)array2p);
}

static int width    (A2 array2) { return Array2b_width    (array2); }
static int height   (A2 array2) { return Array2b_height   (array2); }
static int size     (A2 array2) { return Array2b_size     (array2); }
static int blocksize(A2 array2) { return Array2b_blocksize(array2); }

static A2Methods_Object *at(A2 array2, int i, int j) {
  return Array2b_at(array2, i, j);
}

typedef void applyfun(int i, int j, Array2b_T array2b, void *elem, void *cl);

static void map_block_major (A2 array2, A2Methods_applyfun apply, void *cl) {
  Array2b_map(array2, (applyfun*)apply, cl);
}

static struct A2Methods_T array2_methods_blocked_struct = {
  new,
  new_with_blocksize,
  a2free,
  width,
  height,
  size,
  blocksize,
  at,
  NULL, // map_row_major
  NULL, // map_col_major
  map_block_major,
  map_block_major, // map_default
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T array2_methods_blocked = &array2_methods_blocked_struct;
