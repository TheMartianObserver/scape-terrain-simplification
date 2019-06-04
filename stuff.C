#include <ctype.h>
#include "scape.H"
#include <stdio.h>

extern "C" {
#include "STM-tools/stmops.h"
}

void triarea(const Vector3d& p,const Vector3d& q,const Vector3d& r,
    Real &area, Real &diam)
// finds the area of the oriented 2-D triangle (p, q, r), i.e., the
// area is positive if the triangle is oriented counterclockwise.
// also finds the approximate diameter
{
    Real ux = q.x-p.x, uy = q.y-p.y;
    Real vx = r.x-p.x, vy = r.y-p.y;
    area = (ux*vy-uy*vx)/2.;
    assert(area>=0);//??

    Real xmin, xmax, ymin, ymax;
    xmin = p.x; ymin = p.y;
    xmax = p.x; ymax = p.y;
    if (q.x<xmin) xmin = q.x;  if (q.y<ymin) ymin = q.y;
    if (q.x>xmax) xmax = q.x;  if (q.y>ymax) ymax = q.y;
    if (r.x<xmin) xmin = r.x;  if (r.y<ymin) ymin = r.y;
    if (r.x>xmax) xmax = r.x;  if (r.y>ymax) ymax = r.y;
    diam = MAX(xmax-xmin, ymax-ymin);
    assert(diam>0);//??
}

void Plane::init(const Vector3d& p,const Vector3d& q,const Vector3d& r)
// find the plane z=ax+by+c passing through three points p,q,r
{
    // We explicitly declare these (rather than putting them in a
    // Vector) so that they can be allocated into registers.
    Real ux = q.x-p.x, uy = q.y-p.y, uz = q.z-p.z;
    Real vx = r.x-p.x, vy = r.y-p.y, vz = r.z-p.z;
    Real den = ux*vy-uy*vx;
    if (den==0)//??
	cout << "Plane::init p=" << p << " q=" << q << " r=" << r << endl;
    assert(den!=0);
    a = (uz*vy-uy*vz)/den;
    b = (ux*vz-uz*vx)/den;
    c = p.z-a*p.x-b*p.y;
}


Texture::Texture(ifstream& in)
{
    char tmp[16];
    int width,height,cmax,x,y,r,g,b;
    int is_raw = 0;
    unsigned char byte;

    if( !in.good() ) {
	cerr << "ERROR:  Unable to read texture file." << endl;
	exit(1);
    }


    in >> tmp >> width >> height >> cmax;

    assert( cmax==255 );

    data.init(width,height);


    if( tmp[0]=='P' && tmp[1]=='3' )
	;
    else if( tmp[0]=='P' && tmp[1]=='6' ) {
	is_raw = 1;
	// There may be a single whitespace character in our way
	if( isspace(in.peek()) )
	    in.get(byte);
    } else {
	cerr << "BOGUS PPM TEXTURE FILE" << endl;
	exit(1);
    }

    // for(y=0;y<height;y++)
    for(y=height-1;y>=0;y--) 
	for(x=0;x<width;x++) {
	    Color& c = data.ref(x,y);

	    if( is_raw ) {
		in.get(byte);
		c.r = (rgb_val)byte / (rgb_val)cmax;

		in.get(byte);
		c.g = (rgb_val)byte / (rgb_val)cmax;

		in.get(byte);
		c.b = (rgb_val)byte / (rgb_val)cmax;;
	    } else {
		in >> r >> g >> b;
		c.r = (rgb_val)r / (rgb_val)cmax;;
		c.g = (rgb_val)g / (rgb_val)cmax;;
		c.b = (rgb_val)b / (rgb_val)cmax;;
	    }
	}
}




DEMdata::DEMdata(ifstream& in)
{
    int width, height;
    int x,y;
    char c;
    char orderBytes[4];

    in >> "STM" >> width >> height;
    in >> orderBytes[0] >> orderBytes[1] >> orderBytes[2] >> orderBytes[3];

    in.get(c); // Read the EOL byte

    if( debug ) {
	cout << "# Width: " << width << "   Height: " << height;
    }
    
    z.init(width, height);
    
    z.bitread(in);

    if( !stmMatchOrder(orderBytes) )
        for(x=0;x<width;x++)
	    for(y=0;y<height;y++) {
		unsigned short v = z.ref(x,y);
		z.ref(x,y) = ((v<<8)&0xff00) | ((v>>8)&0xff);
	    }

    //
    // The data is stored with (0,0) in the upper left ala image
    // coordinates.  However, GL will display things with (0,0) in the
    // lower left corner.  So we'll just flip the data.
    // This has no affect on the TIN that is generated.
    // It only changes the display.
    //
    for(y=0;y<height/2;y++)
	for(x=0;x<width;x++)
	{
	    unsigned short tmp = z.ref(x,y);
	    z.ref(x,y) = z.ref(x,height-1-y);
	    z.ref(x,height-1-y) = tmp;
	}


    zmax = -HUGE;
    zmin = HUGE;

    for(x=0;x<width;x++)
	for(y=0;y<height;y++) {
	    Real val = (Real)z.ref(x,y);
	    
	    if (val!=DEM_BAD) {
		if( val > zmax ) zmax = val;
		if( val < zmin ) zmin = val;
	    }
	}
    if (debug)
	cerr << "# zmin=" << zmin << ", zmax=" << zmax << endl;
}
