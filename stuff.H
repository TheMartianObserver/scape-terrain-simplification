// ************ I think we ought to conglomerate some of this stuff
// ************ into a more coherent vector library
// (ie.  we have these different Vector3d and Point2d things, both of
// which are quite incomplete and have no sharing of code)
class Vector3d {
public:
    Real x,y,z;

    Vector3d() { x=y=z=0; }
    Vector3d(Real a,Real b,Real c) { x=a;y=b;z=c; }
    Vector3d(const Point2d& p,Real zz) { x=p.x; y=p.y; z=zz; }
    friend ostream& operator<<(ostream&, const Vector3d&);
};

inline ostream& operator<<(ostream& os, const Vector3d& p)
{
    os << '(' << p.x << "," << p.y << ',' << p.z << ')';
    return os;
}

void triarea(const Vector3d& p,const Vector3d& q,const Vector3d& r,
    Real &area, Real &diam);

// A rectangular region defined by min and max points
class Region {
    int xmin,ymin;
    int xmax,ymax;
public:
    Region() { }

    void init(int x0,int y0,int x1,int y1) {
	xmin=x0; ymin=y0;  xmax=x1; ymax=y1;
    }

    void add(int x,int y) {
	if( x<xmin ) xmin = x;
	if( x>xmax ) xmax = x;

	if( y<ymin ) ymin = y;
	if( y>ymax ) ymax = y;
    }


    void extent(int& x0,int& y0,int& x1,int& y1) {
	x0=xmin; y0=ymin;  x1=xmax; y1=ymax;
    }
};

class Plane {
public:
    Real a, b, c;	/* plane is z=ax+by+c */

    Plane() {}
    Plane(const Vector3d& p,const Vector3d& q,const Vector3d& r) {
	init(p,q,r);
    }
    void init(const Vector3d&,const Vector3d&,const Vector3d&);

    Real operator()(Real x,Real y) {
	return a*x+b*y+c;
    }
    Real operator()(int x,int y) {
	return a*x+b*y+c;
    }
};



typedef Real rgb_val;

class Color {
public:
    rgb_val r,g,b;

    Color() { r=g=b=0; }
    Color(rgb_val x,rgb_val y,rgb_val z) { r=x; g=y; b=z; }
};


class tex_array : public array2<Color> {
public:
    tex_array() : array2<Color>() { }
    tex_array(int w,int h) : array2<Color>(w,h) { }

    Color& ref(int i,int j) { 
	i %= width;
	j %= height;

	return data[j*width + i];
    }

    Color& operator()(int i,int j) { return ref(i,j); }
};





class Texture {
protected:
    tex_array data;
public:
    Texture(ifstream& in);
    ~Texture() { data.free(); }

    rgb_val r(int i,int j) { return data.ref(i,j).r; }
    rgb_val g(int i,int j) { return data.ref(i,j).g; }
    rgb_val b(int i,int j) { return data.ref(i,j).b; }

    void color(int i,int j,rgb_val& r,rgb_val& g,rgb_val& b) {
	r = data.ref(i,j).r;
	g = data.ref(i,j).g;
	b = data.ref(i,j).b;
    }
    Color &ref(int i,int j) { return data.ref(i,j); }
};


typedef Texture RealTexture;



class Zdata {
public:
    Real zmax,zmin;
};

class DEMdata : public Zdata {
    array2<unsigned short> z;
public:
    DEMdata(ifstream&);
    ~DEMdata() { z.free(); }

    Real eval(int x,int y) { return (Real)z.ref(x,y); }
    unsigned short &ref(int x,int y) { return z.ref(x,y); }
    int width() { return z.w(); }
    int height() { return z.h(); }
};
