//
// hfield.C
//
// Implements the HField class.
// HFields are used to store simple grid-sampled height fields.
// These height fields are what the program is going to simplify.
// The idea is to separate the height field and its approximation into
// two separate entities.

#include "scape.H"

#define LERP(t, a, b)	((a)+(t)*((b)-(a)))	/* linear interpolation */

// HField::init --
//
// Takes a stream to read in a height field from and the filename of a
// texture file.  It then does the obvious -- it reads in the height field,
// reads in the texture, and initializes the various internal data arrays.
//
void HField::init(ifstream& mntns, char *texfile)
{
    if( !mntns.good() )
    {
	cerr << "ERROR: Input terrain data does not seem to exist." << endl;
	exit(1);
    }

    data = new DEMdata(mntns);
    width = data->width();
    height = data->height();

    //?? optimization: if emphasis==0 || texfile==0 then don't read texture
    if( texfile ) {
	ifstream tin(texfile);
	cout << "# Opening texture file: " << texfile << endl;
	tex = new RealTexture(tin);
    } else {
	emphasis = 0.0;
	tex = NULL;
    }

    render_with_color = 0;
    render_as_surface = 0;

    model_center.x = width/2;
    model_center.y = height/2;
    model_center.z = zmin() + (zmax()-zmin())/2;

    bound_volume.min.x = 0;
    bound_volume.min.y = 0;
    bound_volume.min.z = zmin();

    bound_volume.max.x = width-1;
    bound_volume.max.y = height-1;
    bound_volume.max.z = zmax();
}


// HField::free --
//
// Like the name says, free the storage that we're currently using.
//
void HField::free()
{
    delete data;
    delete tex;
}

Real HField::eval_interp(Real x,Real y)
// bilinear interpolation
// Note: this code could access off edge of array, but such bogus samples
// should be weighted by zero (fx=0 and/or fy=0).
{
    int ix = (int)x; Real fx = x-ix;
    int iy = (int)y; Real fy = y-iy;
    Real zx0 = LERP(fx, eval(ix,iy),   eval(ix+1,iy));
    Real zx1 = LERP(fx, eval(ix,iy+1), eval(ix+1,iy+1));
    return LERP(fy, zx0, zx1);
}

void HField::color_interp(Real x,Real y,Real &r,Real &g,Real &b)
// bilinear interpolation
// Note: this code could access off edge of array, but such bogus samples
// should be weighted by zero (fx=0 and/or fy=0).
{
    int ix = (int)x; Real fx = x-ix;
    int iy = (int)y; Real fy = y-iy;
    Color cx0, cx1, *color;

    color = &color_ref(ix, iy);
    cx0.r = LERP(fx, color[0].r, color[1].r);
    cx0.g = LERP(fx, color[0].g, color[1].g);
    cx0.b = LERP(fx, color[0].b, color[1].b);

    color = &color_ref(ix, iy+1);
    cx1.r = LERP(fx, color[0].r, color[1].r);
    cx1.g = LERP(fx, color[0].g, color[1].g);
    cx1.b = LERP(fx, color[0].b, color[1].b);

    r = LERP(fy, cx0.r, cx1.r);
    g = LERP(fy, cx0.g, cx1.g);
    b = LERP(fy, cx0.b, cx1.b);

}
