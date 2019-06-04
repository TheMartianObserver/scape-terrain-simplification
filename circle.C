/*
circle - draw a filled circle of fixed screen space radius

circf is ridiculously slow, about 100 times slower than this.
Whoever wrote circf should be spanked!

Paul Heckbert	6 July 1994
*/

#include <assert.h>
#include <malloc.h>
#include <gl.h>		/* iris graphics library subroutine prototypes */

#define FONT_INDEX 1


void circle_init(float radius) {
    /* create a font bitmap for a circle */
    /* (this technique for fast dot-drawing suggested by Thad Beier) */

    static float curradius = -1;
    if (radius==curradius) return;
    curradius = radius;

    int x, y, r = (int)radius, d = 2*r+1, nw = (d+15)/16;
    unsigned short *bitmap;
    Fontchar chars[2];
	/* \000 is unused (because it ends string) */
	/* \001 is circle */
    
    bitmap = (unsigned short *)calloc(nw*d, sizeof bitmap[0]);
	/* calloc zeros the array */
    assert(bitmap);
#   define SETBIT(x, y) bitmap[(y)*nw+(x)/16] |= 1<<(~(x)&15)
    for (y= -r; y<=r; y++)
	for (x= -r; x<=r; x++)
	    if (x*x+y*y<=radius*radius) SETBIT(x+r, y+r);
    chars[1].offset = 0;
    chars[1].w = d;
    chars[1].h = d;
    chars[1].xoff = -r;
    chars[1].yoff = -r;
    chars[1].width = d+1;
    defrasterfont(FONT_INDEX, /*height*/ d,
	sizeof chars/sizeof chars[0], /*character info*/ chars,
	/*length of bitmap array*/ nw*d, bitmap);
    free(bitmap);
    font(FONT_INDEX);
}

void circle(float x, float y, float z) {
    cmov(x, y, z);
    charstr("\001");
}
