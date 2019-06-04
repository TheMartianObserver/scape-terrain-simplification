#include <stdio.h>
#include "stmops.h"


int width = -1;
int height;

double min,max;
char buf[256];
int profiles;

unsigned short *this_profile;



void process_profile()
{
    int num,size,i;
    unsigned short val;

    scanf("%d\n",&num);
    scanf("%d\n",&size);

    if( width < 0 ) {
	width = size;
	stmWriteHeader(stdout, width, height);

	this_profile = (unsigned short *)calloc(sizeof(unsigned short),width);
    } else if( size!=width ) {
	fprintf(stderr,"Error: Data has inconsistently sized profiles.\n");
	exit(1);
    }

    /* Read and ignore the starting coordinates */
    gets(buf);

    for(i=0;i<width;i++) {
	scanf("%hu\n",&val);
	this_profile[i] = val;
    }

    stmWriteData(stdout, this_profile, width);
}

void read_header(FILE *in)
{
    /* Read (x1,y1) coordinates */
    fgets(buf, 255, in);
    fgets(buf, 255, in);

    /* Read (x2,y2) coordinates */
    fgets(buf, 255, in);
    fgets(buf, 255, in);

    /* Read (x3,y3) coordinates */
    fgets(buf, 255, in);
    fgets(buf, 255, in);

    /* Read (x4,y4) coordinates */
    fgets(buf, 255, in);
    fgets(buf, 255, in);

    fscanf(in,"%lf\n",&min);
    fscanf(in,"%lf\n",&max);
    fscanf(in,"%d\n",&profiles);
}


main()
{
    int i;

    read_header(stdin);

    height = profiles;



    for(i=0;i<profiles;i++)
	process_profile();
}
