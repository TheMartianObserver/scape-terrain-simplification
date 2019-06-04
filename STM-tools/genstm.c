#include <stdio.h>
#include <math.h>
#include "stmops.h"
#define ABS(a)          ((a)>=0 ? (a) : -(a))

main(int ac, char **av) {
    int nx, ny, x, y, style;
    short z;
    float sx, sy;

    if (ac<3) exit(1);
    nx = atoi(av[1]);
    ny = atoi(av[2]);
    style = ac>3 ? av[3][0] : 'p';

    stmWriteHeader(stdout, nx, ny);
    for (y=0; y<ny; y++)
        for (x=0; x<nx; x++) {
            switch (style) {
                case 'h': /* step in x (Heaviside) */
                    z = x>nx/2 ? 1000 : 0;
                    break;
                case 'x': /* rounded step in x */
                    sx = (x-nx/2)/(nx/8.);
                    z = 1000*(1+tanh(sx));
                    break;
                case 'X': /* sharply rounded step in x */
                    sx = (x-nx/2)/(nx/64.);
                    z = 1000*(1+tanh(sx));
                    break;
                case 'd': /* rounded step diagonally */
                    sx = (x+y-nx)/(nx/8.);
                    z = 1000*(1+tanh(sx));
                    break;
                case 'D': /* sharply rounded step diagonally */
                    sx = (x+y-nx)/(nx/64.);
                    z = 1000*(1+tanh(sx));
                    break;
                case 'q': /* quadratic in x */
                    sx = (double)x/nx;
                    z = 1000*sx*sx;
                    break;
                case 'o': /* orthogonal pyramid */
                    sx = x-nx/3;
                    sy = y-ny/4;
                    z = ABS(sx)+ABS(sy);
                    break;
                case 'r': /* rotated pyramid */
                    sx = x+y-nx/3-ny/4;
                    sy = x-y-nx/3+ny/4;
                    z = ABS(sx)+ABS(sy);
                    break;
                case 'p': /* paraboloid */
                    sx = x-nx/2;
                    sy = y-ny/2;
                    z = sx*sx+sy*sy;
                    break;
                case 'P': /* scaled paraboloid */
                    sx = x-nx/2;
                    sy = y-ny/2;
                    z = (sx*sx+sy*sy)*32767*4/(nx*nx+ny*ny);
                    break;
            }
            fwrite(&z, sizeof z, 1, stdout);
        }
}
