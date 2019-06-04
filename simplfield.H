//
// simplfield.H
//
// Defines the various classes/structures used in building and
// maintaining approximations to height fields.


struct heap_node {
    Real val;
    Triangle *tri;
};

class Heap;
class SimplField;

struct FitPlane {	// a set of planes for fitting a surface
			    // a temp. data struc for data-dep. triangulation
    Plane z,r,g,b;	// plane equations for z, r, g, b as functions of (x,y)
    Real area;		// area of the triangle
    Real quality;	// area/diameter of triangle: large is good, 0 is bad
    int cx, cy;		// candidate point
    Real cerr;		// absolute error of candidate
    Real err;		// error over triangle,
			// used for data-dependent triangulation
			// = sum of squared error if criterion=SUM2,
			// = maximum error if criterion=SUMINF or MAXINF
    int done;		// is all of the above information final? (1=yes, 0=no)

    FitPlane() {done = 0;};
    FitPlane(SimplField &ter, Triangle *tri);
	// set all FitPlane info by copying from Triangle
    FitPlane(HField *H, const Point2d &p1, const Point2d &p2, const Point2d &p3)
	{init(H, p1, p2, p3);};
    void init
	(HField *H, const Point2d &p1, const Point2d &p2, const Point2d &p3);
	// initialize planes in FitPlane to pass through p,q,r and
	// initialize error sum and candidate for subsequent accumulation
    friend ostream& operator<<(ostream &, const FitPlane &);
};

class SimplField : public Subdivision, public Model  {

    HField *H;          // The height field being approximated
    Heap *heap;         // Heap of candidate points

    // Some variables to hold random rendering options
    int render_with_color;
    int render_with_mesh;
    int render_with_texture;
    int render_as_surface;
    int render_with_dem;
    int dem_step;

    void emit(Real x,Real y,Real z);
    // void emit(Real x,Real y);
    void emit_origin(Edge *e);
    void render_face(Triangle *);
    friend void face_iterator(Triangle *,void *);

    void init(HField *);
    void free();
    void init_cache();
    void select(Triangle *tri, int x, int y, Real cerr);
    void select_datadep(Triangle *tri, FitPlane &fit);
    void update_cache(Edge *e);
    Real compute_choice(int x,int y);
    Real compute_choice_interp(Real x,Real y);
    void check_swap(Edge *e, FitPlane &abd);
    Edge *InsertSite(const Point2d& x, Triangle *tri);

    void scan_triangle_dataindep(Triangle *tri);
    void scan_triangle_datadep_normal
	(const Point2d &p, const Point2d &q, const Point2d &r,
	FitPlane *u, FitPlane *v);
    void scan_triangle_datadep_supersample
	(const Point2d &p, const Point2d &q, const Point2d &r,
	FitPlane *u, FitPlane *v, int ss);
    void scan_triangle_datadep
	(const Point2d &p, const Point2d &q, const Point2d &r,
	FitPlane *u, FitPlane *v);
    Real angle_between_all_normals(const FitPlane&, const FitPlane&);

public:
    array2<char> is_used;

    SimplField(HField *h) { init(h); }
    ~SimplField() { free(); }

    Edge *select_new_point();
    int select_new_points(Real limit);
    int is_used_interp(Real x, Real y);	// for bilinear interpolation

    Real rms_error();
    Real rms_error_supersample(int ss);
    Real rms_error_estimate();
    Real max_error();
    HField *original() { return H; }
    Heap &get_heap() { return *heap; }

    virtual long eval_key(model_key);
    virtual void process_key(model_key,long);
    virtual void just_render();
};



class Heap {
    heap_node *node;
    int size;

    void swap(int i,int j);

    int parent(int i) { return (i-1)/2; }
    int left(int i) { return 2*i+1; }
    int right(int i) { return 2*i+2; }

    void upheap(int i);
    void downheap(int i);

public:
    Heap(int s) { size = 0; node = new heap_node[s]; }
    ~Heap() { delete[] node; }

    heap_node& operator[](int i) { return node[i]; }
    int heap_size() { return size; }

    void insert(Triangle *t,Real v);
    heap_node *extract();
    heap_node *top() { return size<1 ? 0 : &node[0]; };
    heap_node& kill(int i);
    void update(int,Real);
};
