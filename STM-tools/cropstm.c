#include <stdio.h>
#include "stmops.h"

STMdata *stm;

main(int ac, char **av)
{
    int nx, ny, x0, y0, y;

    if (ac<3) exit(1);
    nx = atoi(av[1]);
    ny = atoi(av[2]);
    if (ac==5) {
	x0 = atoi(av[3]);
	y0 = atoi(av[4]);
    }
    else x0 = y0 = 0;

    stm = stmRead(stdin);

    fprintf(stderr, "old %dx%d, new %dx%d start=(%d,%d)\n",
	stm->width, stm->height, nx, ny, x0, y0);
    if (x0<0 || y0<0 || x0+nx>stm->width || y0+ny>stm->height) {
	fprintf(stderr, "window goes out of bounds\n");
	exit(1);
    }

    stmWriteHeader(stdout, nx, ny);
    for (y=y0; y<y0+ny; y++)
	fwrite(&stm->data[y*stm->width+x0], sizeof(short), nx, stdout);
}
