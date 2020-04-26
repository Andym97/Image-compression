#include <bitpack.h>
#include <assert.h>
#include <bitpack.h>

#include <stdint.h>
#include "except.h"

Except_T Bitpack_Overflow= {"Overflowing packing bits"};
//Checks to see if an UNSIGNED int can fit the significant bits  given the width
bool Bitpack_fitsu(uint64_t n, unsigned width){

  assert(width <= 64);
  if(width == 0){
       return false;
     }
  int offset= 64 - width;
  uint64_t temp = n;
  //Left Shift
  temp =temp << offset;
  // Right shift
  temp = temp >> offset;

  if(temp == n) {
      return true;
    }
  else{
     return false;
   }
}
//Checks to see if an SIGNED int can fit the significant bits given the width
bool Bitpack_fitss( int64_t n, unsigned width){

  assert(width <= 64);

  int offset= 64 - width;
  int64_t temp = n;

  temp =temp << offset;
  temp = temp >> offset;
  if(temp== n && width != 0 ){
    return true;
  }
  else {return false;
  }
}

  // if(temp == n) {
  //     return true;
  //   }
  // else{
  //    return false;
  //  }

// Returns an UNSIGNED codeword that represents the field
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb){

  assert(width <= 64);
  assert(width+lsb <= 64);
  if (width == 0 ){
    return 0;
  }
  //Creates the codeword after shfiting left and right
  uint64_t temp = word;
  //if (lsb != 64){
  temp = temp << (64 - (lsb + width));
  temp = temp >> (64 - width);

        return temp;
      //}
      //return temp;
}

// Returns a SIGNED codeword that represents the field
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb){
  assert(width <= 64);
  assert(width+lsb <= 64);
  if (width == 0 ){
    return 0;
  }

  int64_t temp = word;
//  if (lsb != 64){
  temp = temp << (64 - (lsb + width));
  temp = temp >> (64 - width);

        return temp;
  //    }
    //  return temp;
}

//Returns an UNSIGNED codeword with overwritten values starting at the least significant bit
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t value){

  assert(width <= 64);
  assert(width+lsb <= 64);
  if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        //Copies of values without modifiyng original
  uint64_t temp = ~0;
  uint64_t originalWord= word;
  uint64_t copyValue= value;

  //Shifts word left then right then left again given width and lsb

  temp = temp << (64- (width+lsb));
  temp = temp >> (64- width);
  temp = temp << lsb;
  temp = ~temp;

  originalWord= originalWord & temp;
  copyValue = copyValue << lsb;
  originalWord = originalWord | copyValue;

    return originalWord;
}

//Returns a SIGNED codeword with overwritten values  starting at the least significant bit
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, int64_t value){

  assert(width <= 64);
  assert(width+lsb <= 64);
  if (!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }
    uint64_t temp = ~0;
    uint64_t originalWord= word;
    uint64_t copyValue= value;

  //Shifts word left then right then left again given width and lsb
    temp = temp << (64- (width+lsb));
    temp = temp >> (64- width);
    temp = temp << lsb;
    temp = ~temp;

    originalWord= originalWord & temp;
    copyValue= copyValue << ( 64- width);
    copyValue = copyValue >> (64 - (width + lsb ));
    originalWord = originalWord | copyValue;

    return originalWord;
}
