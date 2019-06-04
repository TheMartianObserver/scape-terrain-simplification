#include <stdio.h>
#include <stdlib.h>
#include "stmops.h"

typedef unsigned short ushort;

int exact = 0;
int min,max;

STMdata *stm;

void pgm_raw_header(FILE *out,int width,int height)
{
    fprintf(out,"P5 %d %d 255\n",width,height);
}

void pgm_raw_pixel(FILE *out,unsigned char g)
{
    fputc(g,out);
}

/* ----------------------------------------------------- */

void pgm_write_header(FILE *out,int width,int height)
{
    fprintf(out,"P2 %d %d %d\n",width,height,max);
}

void pgm_write_pixel(FILE *out,unsigned int g)
{
    static int line_count = 0;

    fprintf(out," %u",g);
    line_count++;

    if( line_count >= 10 )
    {
	fprintf(out,"\n");
	line_count = 0;
    }
}

/* ----------------------------------------------------- */


void write_the_file()
{
    int i;
    int v;
    unsigned char c;

    min = max = stm->data[0];
    for(i=0;i<stm->width*stm->height;i++) {
	if( stm->data[i] > max ) max = stm->data[i];
	if( stm->data[i] < min ) min = stm->data[i];
    }

    if( exact )
	pgm_write_header(stdout,stm->width,stm->height);
    else
	pgm_raw_header(stdout,stm->width,stm->height);


    for(i=0;i<stm->width*stm->height;i++) {
	if( exact )
	    pgm_write_pixel(stdout, stm->data[i]);
	else
	{
	    v = 255*(stm->data[i]-min);
	    v /= max;

	    c = (unsigned char)v;
	    pgm_raw_pixel(stdout,c);
	}
    }

    if( exact )
	fprintf(stdout,"\n");
}





main(int argc,char **argv)
{
    FILE *in;
    int i;

    if( argc<2 ) {
	fprintf(stderr,"usage: %s <infile>\n",argv[0]);
	exit(0);
    }

    if( argv[1][0] == '-' ) {
	fprintf(stderr,"Reading from STDIN\n");
	in = stdin;
    } else {
	in = fopen(argv[1],"r");
	if( !in ) {
	    fprintf(stderr,"Error: Unable to open input file.\n");
	    exit(1);
	}
    }

    if( argc>2 && !strcmp(argv[2],"exact") )
    {
	exact = 1;
	fprintf(stderr,"Preserving exact data.\n");
    }

    stm = stmRead(in);
    write_the_file();
}
