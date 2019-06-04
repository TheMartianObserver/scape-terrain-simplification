#include <stdio.h>
#include <assert.h>

#include "stmops.h"

#define MAGIC_VALUE 0x04030201

typedef union {
    unsigned int val;
    unsigned char bytes[4];
} bitTest;

int stmMatchOrder(char orderBytes[4])
{
    int match = 1;
    int i;
    bitTest test;

    test.val = MAGIC_VALUE;

    for(i=0;i<3;i++)
	match = match & (test.bytes[i] == orderBytes[i]);

    return match;
}

void stmSwapOrder(unsigned short *data, int length)
{
    int i;

    for(i=0;i<length;i++)
	data[i] = (data[i]<<8)&0xff00 | (data[i]>>8)&0xff;
}

void stmReadHeader(FILE *in, int *width, int *height, int *shouldSwap)
{
    char orderBytes[4];

    fscanf(in,"STM %d %d %c%c%c%c\n",
	   width, height,
	   &orderBytes[0],
	   &orderBytes[1],
	   &orderBytes[2],
	   &orderBytes[3]);

    *shouldSwap = !stmMatchOrder(orderBytes);
}

void stmWriteHeader(FILE *out,int width, int height)
{
    bitTest test;

    test.val = MAGIC_VALUE;
    fprintf(out,"STM %d %d %c%c%c%c\n",
	    width, height,
	    test.bytes[0],
	    test.bytes[1],
	    test.bytes[2],
	    test.bytes[3]);
}

void stmWriteData(FILE *out, unsigned short *data, int length)
{
    fwrite(data, sizeof(unsigned short), length, out);
}


/*******************************************************************/

STMdata *stmRead(FILE *in)
{
    STMdata *data;
    int shouldSwap;
    int length;

    data = (STMdata *)malloc( sizeof(STMdata) );
    if( !data ) {
	fprintf(stderr,"stmRead: Unable to allocate memory.\n");
	exit(1);
    }

    stmReadHeader(in, &data->width, &data->height, &shouldSwap);

    length = data->width * data->height;

    data->data = (unsigned short *)malloc( sizeof(unsigned short)*length );
    if( !data->data ) {
	fprintf(stderr,"stmRead: Unable to allocate memory.\n");
	exit(1);
    }
    fread(data->data, sizeof(unsigned short), length, in);

    if( shouldSwap )
	stmSwapOrder(data->data, length);

    return data;
}


void stmWrite(FILE *out, STMdata *stm)
{
    stmWriteHeader(out, stm->width, stm->height);
    stmWriteData(out, stm->data, stm->width * stm->height);
}
