#include <stdlib.h>
#include <a2plain.h>
#include "array2.h"


typedef A2Methods_Array2 A2;

// define a private version of each function in A2Methods_T that we implement
static A2Methods_Array2 new(int width, int height, int size) {
  return Array2_new(width,height,size);
}

static A2Methods_Array2 new_with_blocksize(int width, int height, int size,
int blocksize)
{
  (void) blocksize;
  return Array2_new(width,height,size);
}



static void a2free(A2 *array2p){
  Array2_free((Array2_T*) array2p);
}
// frees *array2p and overwrites the pointer with NULL
// observe properties of the array

int (width) (A2 array2) { return Array2_width(array2); }

int (height) (A2 array2){  return Array2_height(array2); }

int (size) (A2 array2){  return Array2_size(array2); }
 // for an unblocked array, returns 1
int (blocksize)(A2 array2){
  (void) array2;
  return 1;


}

static A2Methods_Object *at(A2 array2, int i, int j){
  return Array2_at(array2,i,j);
}

 typedef void Array2_apply(int row, int col, Array2_T array2, void *elem, void *cl);
 // static void Array2_map_row_major(Array2_T a2, Array2_apply apply, void *cl){
 //   Array2_map_row_major(array2,(Array2_apply *) apply,cl);
 // }
//  Array2_map_col_major(Array2_T a2, Array2_apply apply, void *cl);


struct a2fun_closure {
  A2Methods_applyfun *apply; // apply function as known to A2Methods
  void *cl;  // closure that goes with that apply function
  A2Methods_Array2 array2; // array being mapped over
};

void apply_a2methods_using_array2_prototype(int row, int col, void *elem, void *cl)
{
  struct a2fun_closure *f = cl; // this is the function/closure originally passed
  f->apply(col, row, f->array2, elem, f->cl);
}

static void map_row_major(A2Methods_Array2 array2, A2Methods_applyfun apply, void *cl) {
  struct a2fun_closure mycl = { apply, cl, array2 };
  Array2_map_row_major(array2, (Array2_apply*) apply_a2methods_using_array2_prototype, &mycl);
}

static void map_col_major(A2Methods_Array2 array2, A2Methods_applyfun apply, void *cl) {
  struct a2fun_closure mycl = { apply, cl, array2 };
  Array2_map_col_major(array2, (Array2_apply*)apply_a2methods_using_array2_prototype, &mycl);
}

// now create the private struct containing pointers to the functions
static struct A2Methods_T array2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,
        map_col_major,
        NULL, //Map Block Major
        map_row_major, //Default
};

// finally the payoff: here is the exported pointer to the struct
A2Methods_T array2_methods_plain = &array2_methods_plain_struct;
