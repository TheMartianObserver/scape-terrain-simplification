//
// hfield.H
//
// Defines the HField class.
// HFields are used to store simple grid-sampled height fields.
// These height fields are what the program is going to simplify.
// The idea is to separate the height field and its approximation into
// two separate entities.


class HField : public Model {
    int width,height;

    int render_with_color;
    int render_as_surface;

    DEMdata *data;

    RealTexture *tex;

    void init(ifstream& mntns, char *texfile);
    void free();

    void draw_from_point(int x,int y);
    void emit(Real,Real);

public:
    HField(ifstream& in, char *texfile) { init(in, texfile); }
    ~HField() { free(); }

    Real eval(int x,int y) { 
        if( x>=0 && x<width && y>=0 && y<height )
	    return data->eval(x,y);
	else
	    return data->zmin;
    }
    unsigned short &z_ref(int x,int y) { return data->ref(x,y); }
    Real eval(Real x,Real y) { return eval((int)x,(int)y); }
    Real eval(const Point2d& p) { return eval((int)p.x,(int)p.y); }
    Real eval_interp(Real x,Real y);	// bilinear interpolation
    void color(int x,int y,Real& r,Real& g,Real& b) {
        if( x>=0 && x<width && y>=0 && y<height ) {
	    r=tex->r(x,y);
	    g=tex->g(x,y);
	    b=tex->b(x,y);
	} else {
	    r=g=b=0;
	}
    }
    void color(const Point2d& p,Real& r,Real& g,Real& b) {
	color((int)p.x,(int)p.y,r,g,b);
    }
    Color &color_ref(int i,int j) { return tex->ref(i,j); }
    void color_interp(Real x,Real y,Real &r,Real &g,Real &b);
	// bilinear interpolation

    Real zmax() { return data->zmax; }
    Real zmin() { return data->zmin; }
    int get_width() { return width; }
    int get_height() { return height; }

    int has_texture() { return (tex!=NULL); }

    virtual void process_key(model_key,long value=0);
    virtual long eval_key(model_key);
    virtual void just_render();
};
