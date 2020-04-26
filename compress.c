#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "compress.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "a2methods.h"
#include "array2b.h"
#include "pnm.h"
#include "bitpack.h"
#include "array2.h"
#include "arith.h"


#define DENOMINATOR 255 //0 to 255

static const unsigned A_LSB = 23;
static const unsigned B_LSB = 18;
static const unsigned C_LSB = 13;
static const unsigned D_LSB = 8;
static const unsigned PR_LSB = 4;
static const unsigned PB_LSB = 0;

static const unsigned A_WIDTH = 9;
static const unsigned B_WIDTH= 5;
static const unsigned C_WIDTH = 5;
static const unsigned D_WIDTH= 5;
static const unsigned PR_WIDTH = 4;
static const unsigned PB_WIDTH = 4;

typedef struct cVideo{
    float y;
    float pb;
    float pr;
} cVideo;

typedef struct pixBlock {
    int a;
    int b;
    int c;
    int d;
    int pr;
    int pb;

} pixBlock;

extern unsigned Arith_index_of_chroma(float x);
extern float Arith_chroma_of_index(unsigned x);

typedef A2Methods_Array2 A2;


// -0.3 and 0.3
int quantizeNum(float num)
{
    if (num < -0.3) {
                return -0.3;
        }
    if (num > 0.3) {
                return 0.3;
        }

    return (int)(50 * num);
}

//Converts RGB to Y/Pb/PR
cVideo RGB_to(Pnm_rgb pixel, float denom)
{
    cVideo val;
    float r = pixel->red / denom;
    float g =  pixel->green / denom;
    float b = pixel->blue / denom;

    float y = (0.299 * r) + (0.587 * g) + (0.114 * b);
    float pb = (-0.168736 * r) - (0.331264 * g) + (0.5 * b);
    float pr = (0.5 * r) - (0.418688 * g) - (0.081312 * b);

    val.y=y;
    val.pb=pb;
    val.pr=pr;

    return val;
}

//Applies discreate cosine transofrmation
//returns a struct pixel_block
pixBlock pack(cVideo Y1, cVideo Y2, cVideo Y3, cVideo Y4)
{

    pixBlock pixel_block;

    float PB_average = (Y1.pb + Y2.pb + Y3.pb + Y4.pb) / 4.0;
    float PR_average = (Y1.pr + Y2.pr + Y3.pr + Y4.pr) / 4.0;
    unsigned PBCHROMA = Arith_index_of_chroma(PB_average);
    unsigned PRCHROMA = Arith_index_of_chroma(PR_average);

        float a = (Y1.y + Y2.y + Y3.y + Y4.y) / 4.0;
        float b = (Y4.y + Y3.y - Y2.y - Y1.y) / 4.0;
        float c = (Y4.y - Y3.y + Y2.y - Y1.y) / 4.0;
        float d = (Y4.y - Y3.y - Y2.y + Y1.y) / 4.0;

        int a1= a*511;
        int b1=quantizeNum (b);
        int c1=quantizeNum(c);
        int d1=quantizeNum(d);

        pixel_block.a=a1;
        pixel_block.b=b1;
        pixel_block.c=c1;
        pixel_block.d=d1;
        pixel_block.pb = PBCHROMA;
        pixel_block.pr=PRCHROMA;

        return pixel_block;
}

void create_word(pixBlock pixel_block)
{

        uint64_t word = 0;
        word = Bitpack_newu(word, A_WIDTH, A_LSB, (uint64_t)pixel_block.a);
        word = Bitpack_news(word, B_WIDTH, B_LSB, (int64_t)pixel_block.b);
        word = Bitpack_news(word, C_WIDTH, C_LSB, (int64_t)pixel_block.c);
        word = Bitpack_news(word, D_WIDTH, D_LSB, (int64_t)pixel_block.d);
        word = Bitpack_newu(word, PB_WIDTH, PB_LSB, (uint64_t)pixel_block.pb);
        word = Bitpack_newu(word, PR_WIDTH, PR_LSB, (uint64_t)pixel_block.pr);

        //Writes as 8-bit bytes using putchar
        putchar(Bitpack_getu(word, 8, 0));
        putchar(Bitpack_getu(word, 8, 8));
        putchar(Bitpack_getu(word, 8, 16));
        putchar(Bitpack_getu(word, 8, 24));
}


//Converts RGB to cVideo pixels
//Creates a word and prints
void comp(int i, int j, A2Methods_Array2 arr, void *elem, void *cl)
{
        Pnm_ppm ppm = *(Pnm_ppm *) cl;
        const struct A2Methods_T *methods = ppm->methods;
        float denom = ppm->denominator;
        pixBlock word;
        (void) elem;


        if (i % 2 == 1 && j % 2 == 1) {
                cVideo Y4 = RGB_to((Pnm_rgb) methods->at(arr, i, j),
                                                                         denom);
                cVideo Y3 = RGB_to((Pnm_rgb) methods->at(arr, i - 1, j),
                                                                         denom);
                cVideo Y2 = RGB_to((Pnm_rgb) methods->at(arr, i, j - 1),
                                                                         denom);
                cVideo Y1 = RGB_to((Pnm_rgb) methods->at(arr, i - 1, j - 1),
                                                                         denom);

                word = pack(Y1, Y2, Y3, Y4);
                create_word(word);
        }
}


void compress (FILE *input)
{
        assert(input != NULL);
        int width, height;
        A2Methods_T methods =array2_methods_plain;
        Pnm_ppm image = Pnm_ppmread(input, methods);
        width = image->width;
        height = image->height;
        image->width = width - (width % 2);
        image->height = height - (height % 2);

        printf(" Compressed image format 2\\n%u %u\n",
                                        image->width, image->height);

        methods->map_row_major(image->pixels, comp, &image);

        Pnm_ppmfree(&image);
}



/*

DECOMPRESS

*/

//Gets next char in compressed image and unpacks
//Returned in a pixel_block struct
pixBlock unpack(FILE *fp)
{
    assert(fp != NULL);
    pixBlock pixel_block;
    uint64_t word = 0;

    unsigned char c = getc(fp);
    word = Bitpack_newu(word, 8, 0, (uint64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 8, (uint64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 16, (uint64_t)c);
    c = getc(fp);
    word = Bitpack_newu(word, 8, 24, (uint64_t)c);


    pixel_block.a = Bitpack_getu(word, A_WIDTH, A_LSB);
    pixel_block.b = Bitpack_gets(word, B_WIDTH, B_LSB);
    pixel_block.c = Bitpack_gets(word, C_WIDTH, C_LSB);
    pixel_block.d = Bitpack_gets(word, D_WIDTH, D_LSB);
    pixel_block.pb = Bitpack_getu(word, PB_WIDTH, PB_LSB);
    pixel_block.pr = Bitpack_getu(word, PR_WIDTH, PR_LSB);




    return pixel_block;

}

float reverse_quantizeNum_index(int index)
{
        return (float)index / 100.0;
}

struct Pnm_rgb to_RGB(cVideo pix)
{
        struct Pnm_rgb temp;

        float y = pix.y;
        float pb = pix.pb;
        float pr = pix.pr;

        int r = DENOMINATOR * ((1.0 * y) + (0 * pb) + (1.402 * pr));
          r = fmax(fmin(r, DENOMINATOR), 0);
        int g = DENOMINATOR * ((1.0 * y) - (0.344136 * pb) - (0.714136 * pr));
          g = fmax(fmin(g, DENOMINATOR), 0);
        int b = DENOMINATOR * ((1.0 * y) + (1.772 * pb) + (0 * pr));
          b =  fmax(fmin(b, DENOMINATOR), 0);

        temp.red = r;
        temp.green = g;
        temp.blue = b;
        return temp;
}

//Converts cVideo pixels to RGB
//Stored in a 2x2 block array
void decomp(int i, int j, A2Methods_Array2 arr, void *elem, void *cl)
{
        (void) elem;

         if (i % 2 == 0 && j % 2 == 0) {
      pixBlock pixel_block = unpack((FILE *) cl);
        A2Methods_T methods =array2_methods_plain;
        cVideo Y1, Y2, Y3, Y4;

        float a = (float) pixel_block.a /  511.0;
        float b = reverse_quantizeNum_index(pixel_block.b);
        float c = reverse_quantizeNum_index(pixel_block.c);
        float d = reverse_quantizeNum_index(pixel_block.d);

        Y1.y = fmin(fmax(a - b - c + d, 0), 1);
        Y2.y = fminf(fmax(a - b + c - d, 0), 1);
        Y3.y = fmin(fmax(a + b - c - d, 0), 1);
        Y4.y = fmin(fmax(a + b + c + d, 0), 1);

      float pb = Arith_chroma_of_index(pixel_block.pb);
      float pr = Arith_chroma_of_index(pixel_block.pr);
        Y1.pb = pb;
        Y1.pr = pr;

        Y2.pb = pb;
        Y2.pr = pr;

        Y3.pb = pb;
        Y3.pr = pr;

        Y4.pb = pb;
        Y4.pr = pr;

      *((struct Pnm_rgb *) methods->at(arr, i, j)) = to_RGB(Y1);
      *((struct Pnm_rgb *) methods->at(arr, i + 1, j)) = to_RGB(Y2);
      *((struct Pnm_rgb *) methods->at(arr,i, j + 1)) = to_RGB(Y3);
      *((struct Pnm_rgb *) methods->at(arr,i + 1, j + 1)) = to_RGB(Y4);
      }

}

void decompress(FILE *input)
{
        assert(input != NULL);
        int size;
        A2Methods_T methods =array2_methods_plain;
        unsigned width, height;
        int read = fscanf(input, "Compressed image format 2\\n%u %u",
                                                         &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        size = sizeof(struct Pnm_rgb);
        A2Methods_Array2 array = methods->new(width, height, size);
        struct Pnm_ppm pixmap = {
                .width = width,
                .height = height,
                .denominator = DENOMINATOR,
                .pixels = array,
                .methods = methods
        };

        methods->map_row_major(pixmap.pixels, decomp, input);
        Pnm_ppmwrite(stdout, &pixmap);

        methods->free(&array);
}
