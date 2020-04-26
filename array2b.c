#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include "array2b.h"
#include "array2.h"
#include "assert.h"
#include "math.h"


#define T Array2b_T

struct T{
  int width;
  int height;
  int size;
  int blocksize;
  Array2_T blockedArray;
};


//NOT DONE
T Array2b_new(int width, int height, int size, int blocksize){

  assert ( (width>0) && (height >0) && (size >0) && (blocksize >0) ) ;
  //T array2b=malloc(sizeof(*array2b));
  //assert(array2b);
  T array2b;
  NEW (array2b);

  int array2b_width= blocksize *  blocksize;
  int array2b_height= height / blocksize;
  array2b -> blockedArray = Array2_new(array2b_width, array2b_height,sizeof(Array2_T)) ;
  array2b -> width = width;
  array2b -> height = height;
  array2b -> size = size;
  array2b -> blocksize = blocksize;


return array2b;
}

T Array2b_new_64K_block(int width, int height, int size){
  //64KB * 1024
  int blocksize= sqrt(65536/size);
  return Array2b_new(width,height,size,blocksize);
}

//Frees 1D array then frees 2D array
void Array2b_free (T*array2b){

  assert(array2b!= NULL);
  Array2_free(& ((*array2b) -> blockedArray));
  FREE (*array2b);
}

int Array2b_width (T array2b){

  assert(array2b);
  return array2b->  width;
 }

int Array2b_height (T array2b){

  assert(array2b);
  return array2b-> height;
}

int Array2b_size (T array2b){

  assert(array2b);
  return array2b-> size;
}

int Array2b_blocksize (T array2b){

  assert(array2b);
  return array2b-> blocksize;
}

void *Array2b_at(T array2b, int i, int j){

  assert (array2b);
  int blocksize= array2b->blocksize;
  int width= array2b -> width;
  int column= (i/ blocksize);
  int row= (j/blocksize);

  // fidn row/column
  int blockNum= (width % blocksize);
  int columnIndex= (blocksize * (i%blocksize)) + (j%blocksize);
  int rowIndex= ((row * blockNum) + column);

  return Array2_at(array2b->blockedArray,columnIndex,rowIndex);
}

void Array2b_map(T array2b,
  void apply (int i, int j, T array2b, void *elem, void *cl), void *cl)
  {
    assert (array2b!= NULL);
    int height= array2b->height;
    int width = array2b->width;
    int blocksize= array2b->blocksize;
    int blockedRow;
    int blockedColumn;

    for (int row=0; row<height; row++ )
    {
      for ( int col=0; col<width; col++)
      {
        blockedRow = row+blocksize;
          for( int i  = row; i < blockedRow && i < height; i++)
          {
            blockedColumn= col + blocksize;
            for (int j= col; j<blockedColumn && j < width; j++)
            {
              apply(j,i,array2b,Array2b_at(array2b,j,i),cl);
            }
          }


      }
    }

  }
