// scan.C: Scan conversion of triangles for simplifying height fields.
// This file contains routines for both data-independent triangulation
// such as Delaunay's method (see scan_triangle_dataindep) and for
// data-dependent triangulation (see scan_triangle_datadep).

// There are a number of versions of the scan_line and scan_triangle
// routines, in order to optimize the most common cases of no texture
// (emphasis=0) and cases where supersampling is unnecessary.

// Michael Garland and Paul Heckbert, 1994

#include "scape.H"

int update_cost = 0;

Real area_thresh = 1e30;// Maximum fraction of triangle area that is permitted
			// to be partially covered by samples.
			// Controls supersampling resolution.
			// 0 => infinite supersampling
			// .8 => moderate supersampling
			// 1e30 => no supersampling

int nscan = 0, nsuper = 0; // count of triangles scan converted & supersampled

static Real w1,w2;

static inline Real divide_safe(Real a, Real b) { return b!=0 ? a/b : 0; }

void compute_triangle_zplane(Triangle *tri,HField *H, Plane& z_plane)
{
    const Point2d& p1 = tri->point1();
    const Point2d& p2 = tri->point2();
    const Point2d& p3 = tri->point3();

    Vector3d v1(p1,H->eval(p1)),v2(p2,H->eval(p2)),v3(p3,H->eval(p3));
    z_plane.init(v1,v2,v3);
}

void compute_triangle_planes(Triangle *tri,HField *H,
			     Plane& z_plane,Plane& r_plane,
			     Plane& g_plane, Plane& b_plane)
{
    const Point2d& p1 = tri->point1();
    const Point2d& p2 = tri->point2();
    const Point2d& p3 = tri->point3();

    Vector3d v1(p1,H->eval(p1)),v2(p2,H->eval(p2)),v3(p3,H->eval(p3));
    z_plane.init(v1,v2,v3);

    // possible optimization: don't do the following if emphasis==0
    Real r1,g1,b1,r2,g2,b2,r3,g3,b3;
    H->color(p1,r1,g1,b1);
    H->color(p2,r2,g2,b2);
    H->color(p3,r3,g3,b3);

    v1.z = r1; v2.z = r2; v3.z = r3;
    r_plane.init(v1,v2,v3);

    v1.z = g1; v2.z = g2; v3.z = g3;
    g_plane.init(v1,v2,v3);

    v1.z = b1; v2.z = b2; v3.z = b3;
    b_plane.init(v1,v2,v3);
}

int point2d_y_compar(const void *a,const void *b)
{
    Point2d *p1=(Point2d *)a,
	    *p2=(Point2d *)b;

    return p1->y==p2->y ? 0 : p1->y<p2->y ? -1 : 1;
}

inline void order_triangle_points(Point2d *by_y, const Point2d& p1,
				  const Point2d& p2, const Point2d& p3)
{
    by_y[0] = p1;
    by_y[1] = p2;
    by_y[2] = p3;

    qsort(by_y,3,sizeof(Point2d),point2d_y_compar);
}

//-------------------- scan conversion for data-independent triangulation

void scan_line_dataindep_z(int y, HField *H, SimplField *S,
		       Plane& z_plane, 
		       Real& x1, Real& x2, Real& maxval, int& maxx, int& maxy)
// optimized version of scan_line_dataindep
//
// This version does z only, uses pointer arithmetic for speed.
// These optimizations speed up batch program, which doesn't do graphics,
// by about 7 times, for m/n=1% !  (less if m/n greater)
{
    int x;
    int startx = (int)ceil(MIN(x1,x2));
    int endx   = (int)floor(MAX(x1,x2));
    if (startx > endx) return;

    Real diff, z = z_plane(startx,y), dz = z_plane.a;
    unsigned short *zp = &H->z_ref(startx,y);
    char *usedp = &S->is_used.ref(startx,y);

    for(x=startx;x<=endx;x++) {
	if( !*usedp ) {
	    diff = *zp-z;
	    if (diff<0) diff = -diff;
	    if( diff > maxval ) {	// update candidate for v
		maxx = x;
		maxy = y;
		maxval = diff;
	    }
	    update_cost++;
	}
	z += dz;
	zp++;
	usedp++;
    }
    scancount += endx-startx+1;
}

void scan_line_dataindep(int y, HField *H, SimplField *S,
		       Plane& z_plane, Plane& r_plane,
		       Plane& g_plane, Plane& b_plane,
		       Real& x1, Real& x2, Real& maxval, int& maxx, int& maxy)
{
    Real r,g,b,z,diff;
    int x;

    int startx = (int)ceil(MIN(x1,x2));
    int endx   = (int)floor(MAX(x1,x2));
	
    Real z0 = z_plane(startx,y), dz = z_plane.a;
    Real r0 = r_plane(startx,y), dr = r_plane.a;
    Real g0 = g_plane(startx,y), dg = g_plane.a;
    Real b0 = b_plane(startx,y), db = b_plane.a;

    for(x=startx;x<=endx;x++) {
	if( !S->is_used(x,y) ) {
	    z = H->eval(x,y);
	    H->color(x,y,r,g,b);
	    
	    diff = w1*fabs(z-z0) +
		w2*(fabs(r-r0) +
		    fabs(g-g0) +
		    fabs(b-b0));

	    if( diff > maxval ) {
		maxx = x;
		maxy = y;
		maxval = diff;
	    }
		
	    update_cost++;
	}
	z0 += dz;
	r0 += dr;
	g0 += dg;
	b0 += db;
    }
    scancount += endx-startx+1;
}

void SimplField::scan_triangle_dataindep(Triangle *tri)
// Scan convert triangle for data-independent triangulation (e.g. Delaunay)
// assumes that triangle vertices have integer coordinates
{
    Plane z_plane,r_plane,g_plane,b_plane;

    if (debug>1)
	cout << "    scan converting " << tri->point1() << " " << tri->point2()
	    << " " << tri->point3() << endl;

    if( emphasis > 0.0 )
	compute_triangle_planes(tri,H,z_plane,r_plane,g_plane,b_plane);
    else
	compute_triangle_zplane(tri,H,z_plane);

    Point2d by_y[3];
    order_triangle_points(by_y,tri->point1(),tri->point2(),tri->point3());

    Real zrange = H->zmax();	// should probably be zmax-zmin
    if (zrange<=0) zrange = 1;
    w2 = emphasis * zrange/3;
    w1 = 1-emphasis;

    int y;
    Real x1,x2;
    Real dx1,dx2;

    Real maxval = -HUGE;
    int maxx,maxy;


    dx1 = divide_safe((by_y[1].x - by_y[0].x), (by_y[1].y - by_y[0].y));
    dx2 = divide_safe((by_y[2].x - by_y[0].x), (by_y[2].y - by_y[0].y));

    x1 = x2 = by_y[0].x;

    for(y=(int)by_y[0].y;y<(int)by_y[1].y;y++) {
	if (emphasis==0)
	    scan_line_dataindep_z(y,H,this,z_plane,
		x1,x2,maxval,maxx,maxy);
	else
	    scan_line_dataindep(y,H,this,z_plane,r_plane,g_plane,b_plane,
		x1,x2,maxval,maxx,maxy);
	x1 += dx1;
	x2 += dx2;
    }

    dx1 = divide_safe((by_y[2].x - by_y[1].x), (by_y[2].y - by_y[1].y));
    x1 = by_y[1].x;

    for(y=(int)by_y[1].y;y<=(int)by_y[2].y;y++) {
	if (emphasis==0)
	    scan_line_dataindep_z(y,H,this,z_plane,
		x1,x2,maxval,maxx,maxy);
	else
	    scan_line_dataindep(y,H,this,z_plane,r_plane,g_plane,b_plane,
		x1,x2,maxval,maxx,maxy);
	x1 += dx1;
	x2 += dx2;
    }

    select(tri, maxx, maxy, maxval);
}


//-------------------- scan conversion for data-dependent triangulation

void scan_line_datadep_z
    (int y, SimplField *S, FitPlane *u, FitPlane *v, Real& x1, Real& x2)
// Scan a horizonal line between (x1,y) and (x2,y) computing error between
// data in height field H and the planes u and v, updating for each
// plane the sum of squared errors and the candidate point with highest error.
//
// This version does z only, uses pointer arithmetic for speed.
// These optimizations speed up scape program, which spends much of its
//	time drawing, about 12 to 15%,
// and it speeds up batch program, which doesn't do graphics, by about 5 times,
// for m/n=1% !  (less if m/n greater)
//
// plane u's error already computed iff u==0,
// plane v always needs to be computed
{
    int x;
    int startx = (int)ceil(MIN(x1,x2));
    int endx   = (int)floor(MAX(x1,x2));
    if (startx > endx) return;

    Real diff, uz;
    if (u) uz = u->z(startx,y);
    Real vz = v->z(startx,y);
    unsigned short *zp = &S->original()->z_ref(startx,y);
    char *usedp = &S->is_used.ref(startx,y);

    for(x=startx;x<=endx;x++) {
	if( !*usedp ) {
	    if (u) {
		// test against plane u
		diff = *zp-uz;
		if (diff<0) diff = -diff;
		if( diff > u->cerr ) {	// update candidate for u
		    u->cx = x;
		    u->cy = y;
		    u->cerr = diff;
		}
		if (criterion==SUM2)
		    u->err += diff*diff;// update squared error for u
		else if (diff>u->err)	// update max error for u
		    u->err = diff;
		if (debug>2)//??
		    cout << "(" << x << "," << y << ")" << diff << "  ";
	    }
	    else if (debug>2) cout << "       ";//??

	    // test against plane v
	    diff = *zp-vz;
	    if (diff<0) diff = -diff;
	    if( diff > v->cerr ) {	// update candidate for v
		v->cx = x;
		v->cy = y;
		v->cerr = diff;
	    }
	    if (criterion==SUM2)
		v->err += diff*diff;	// update squared error for v
	    else if (diff>v->err)	// update max error for v
		v->err = diff;
	    if (debug>2)//??
		cout << "(" << x << "," << y << ")" << diff << "\n";

	    update_cost++;
	}
	if (u) uz += u->z.a;
	vz += v->z.a;
	zp++;
	usedp++;
    }
    if (debug>2) cout << endl;//??
    scancount += endx-startx+1;
}

void scan_line_datadep_zrgb
    (int y, SimplField *S, FitPlane *u, FitPlane *v, Real& x1, Real& x2)
// Scan a horizonal line between (x1,y) and (x2,y) computing error between
// data in height field H and the planes u and v, updating for each
// plane the sum of squared errors and the candidate point with highest error.
// This version does z,r,g,b.
// plane u's error already computed iff u==0,
// plane v always needs to be computed
{
    int x;
    int startx = (int)ceil(MIN(x1,x2));
    int endx   = (int)floor(MAX(x1,x2));
    if (startx > endx) return;

    Real diff, z, r, g, b, uz, ur, ug, ub;
    if (u) {
	uz = u->z(startx,y);
	ur = u->r(startx,y);
	ug = u->g(startx,y);
	ub = u->b(startx,y);
    }
    Real vz = v->z(startx,y);
    Real vr = v->r(startx,y);
    Real vg = v->g(startx,y);
    Real vb = v->b(startx,y);

    for(x=startx;x<=endx;x++) {
	if( !S->is_used(x,y) ) {
	    z = S->original()->eval(x,y);
	    S->original()->color(x,y,r,g,b);

	    if (u) {
		// test against plane u
		diff = w1*ABS(z-uz) + w2*(ABS(r-ur) + ABS(g-ug) + ABS(b-ub));
		if( diff > u->cerr ) {	// update candidate for u
		    u->cx = x;
		    u->cy = y;
		    u->cerr = diff;
		}
		if (criterion==SUM2)
		    u->err += diff*diff;// update squared error for u
		else if (diff>u->err)	// update max error for u
		    u->err = diff;
	    }

	    // test against plane v
	    diff = w1*ABS(z-vz) + w2*(ABS(r-vr) + ABS(g-vg) + ABS(b-vb));
	    if( diff > v->cerr ) {	// update candidate for v
		v->cx = x;
		v->cy = y;
		v->cerr = diff;
	    }
	    if (criterion==SUM2)
		v->err += diff*diff;	// update squared error for v
	    else if (diff>v->err)	// update max error for v
		v->err = diff;

	    update_cost++;
	}
	if (u) {
	    uz += u->z.a;
	    ur += u->r.a;
	    ug += u->g.a;
	    ub += u->b.a;
	}
	vz += v->z.a;
	vr += v->r.a;
	vg += v->g.a;
	vb += v->b.a;
    }
    scancount += endx-startx+1;
}


void SimplField::scan_triangle_datadep_normal
    (const Point2d &p, const Point2d &q, const Point2d &r,
    FitPlane *u, FitPlane *v)
// scan convert the triangle with vertices p,q,r to find the error and best
// candidates for the two planes u and v
// plane u's error needs to be computed iff u!=0 && u->done==0,
// plane v's error always needs to be computed
// doesn't assume that vertices have integer coordinates
// This version does normal scan conversion (no supersampling).
{
    if (debug>1)
	cout << "    scan converting " << p << " " << q << " " << r;

    if (u && u->done) u = 0;

    // triangles u and v each share one side and one angle with triangle pqr,
    // so if either triangle u or triangle v has zero area, that implies
    // that triangle pqr has zero area and no interior pixels, so we can return

    if (u && u->area==0 || v->area==0) {
	assert(!u || u->area!=0);
	if (debug>1) {
	    cout << endl << "      empty triangle, not scan converting,";
	    if (u) cout << " u->area=" << u->area;
	    else cout << " no-u";
	    if (v) cout << " v->area=" << v->area;
	    else cout << " no-v";
	    cout << endl;
	}
	return;
    }

    Point2d by_y[3];
    order_triangle_points(by_y, p, q, r);

    // if we went ahead and divided in all cases instead of using divide_safe,
    // this would occasionally cause startx or endx to contain 0x7fffffff
    // and cause valid scan lines at bottom or top to be missed
    Real dx1 = divide_safe(by_y[1].x - by_y[0].x, by_y[1].y - by_y[0].y);
    Real dx2 = divide_safe(by_y[2].x - by_y[0].x, by_y[2].y - by_y[0].y);
    int y = (int)ceil(by_y[0].y);
    Real frac = y - by_y[0].y;
    Real x1 = by_y[0].x + dx1*frac;
    Real x2 = by_y[0].x + dx2*frac;
    int scancount0 = scancount;

    for(;y<by_y[1].y;y++) {
	if (emphasis==0)
	    scan_line_datadep_z(y, this, u, v, x1, x2);
	else
	    scan_line_datadep_zrgb(y, this, u, v, x1, x2);
	x1 += dx1;
	x2 += dx2;
    }

    dx1 = divide_safe(by_y[2].x - by_y[1].x, by_y[2].y - by_y[1].y);
    frac = y - by_y[1].y;
    x1 = by_y[1].x + dx1*frac;

    for(;y<=(int)by_y[2].y;y++) {
	if (emphasis==0)
	    scan_line_datadep_z(y, this, u, v, x1, x2);
	else
	    scan_line_datadep_zrgb(y, this, u, v, x1, x2);
	x1 += dx1;
	x2 += dx2;
    }
    if (debug>1)
	cout << ", " << scancount-scancount0 << " pixels" << endl;
}


//------- scan conversion for data-dependent triangulation, with supersampling

void scan_line_datadep_supersample
    (int y, SimplField *S, FitPlane *u, FitPlane *v, Real& x1, Real& x2, int ss)
// With supersample factor ss,
// scan a horizonal line between (x1,y) and (x2,y) computing error between
// data in height field H and the planes u and v, updating for each
// plane the sum of squared errors and the candidate point with highest error.
// This version does z,r,g,b.
// plane u's error already computed iff u==0,
// plane v always needs to be computed
{
    int x;
    int startx = (int)ceil(MIN(x1,x2));
    int endx   = (int)floor(MAX(x1,x2));
    if (startx > endx) return;

    Real diff, z, r, g, b, uz, ur, ug, ub, vr, vg, vb;
    if (u) {
	uz = u->z(startx,y);
	if (emphasis!=0) {
	    ur = u->r(startx,y);
	    ug = u->g(startx,y);
	    ub = u->b(startx,y);
	}
    }
    Real vz = v->z(startx,y);
    if (emphasis!=0) {
	vr = v->r(startx,y);
	vg = v->g(startx,y);
	vb = v->b(startx,y);
    }

    Real rx, ry = (Real)y/ss;
    for(x=startx;x<=endx;x++) {
	rx = (Real)x/ss;
	if( !S->is_used_interp(rx,ry) ) {
	    z = S->original()->eval_interp(rx,ry);
	    if (emphasis!=0)
		S->original()->color_interp(rx,ry,r,g,b);

	    if (u) {
		// test against plane u
		if (emphasis==0)
		    diff = ABS(z-uz);
		else
		    diff = w1*ABS(z-uz)
			+ w2*(ABS(r-ur) + ABS(g-ug) + ABS(b-ub));
		if( x%ss==0 && y%ss==0 && diff > u->cerr ) {
		    // update candidate for u
		    // (only when x/ss and y/ss are integers)
		    u->cx = (int)rx;
		    u->cy = (int)ry;
		    u->cerr = diff;
		}
		if (criterion==SUM2)
		    u->err += diff*diff;// update squared error for u
		else if (diff>u->err)	// update max error for u
		    u->err = diff;
		if (debug>2)//??
		    cout << "(" << x << "," << y << ")" << diff << "  ";
	    }
	    else if (debug>2) cout << "       ";//??

	    // test against plane v
	    if (emphasis==0)
		diff = ABS(z-vz);
	    else
		diff = w1*ABS(z-vz)
		    + w2*(ABS(r-vr) + ABS(g-vg) + ABS(b-vb));
	    if( x%ss==0 && y%ss==0 && diff > v->cerr ) {
		// update candidate for v
		// (only when x/ss and y/ss are integers)
		v->cx = x/ss;
		v->cy = y/ss;
		v->cerr = diff;
	    }
	    if (criterion==SUM2)
		v->err += diff*diff;	// update squared error for v
	    else if (diff>v->err)	// update max error for v
		v->err = diff;
	    if (debug>2)//??
		cout << "(" << x << "," << y << ")" << diff << endl;

	    update_cost++;
	}
	if (u) {
	    uz += u->z.a;
	    if (emphasis!=0) {
		ur += u->r.a;
		ug += u->g.a;
		ub += u->b.a;
	    }
	}
	vz += v->z.a;
	if (emphasis!=0) {
	    vr += v->r.a;
	    vg += v->g.a;
	    vb += v->b.a;
	}
    }
    if (debug>2) cout << endl;
    scancount += endx-startx+1;
}

void SimplField::scan_triangle_datadep_supersample
    (const Point2d &p, const Point2d &q, const Point2d &r,
    FitPlane *u, FitPlane *v, int ss)
// With supersample factor ss,
// scan convert the triangle with vertices p,q,r to find the error and best
// candidates for the two planes u and v
// plane u's error needs to be computed iff u!=0 && u->done==0,
// plane v's error always needs to be computed
// doesn't assume that vertices have integer coordinates
//
// To scan convert with supersampling, we multiply triangle's coordinates
// by ss, do scan conversion as usual, but inside inner loop we
// access the is_used, z, and color arrays using coordinates that have
// been divided by ss, and do bilinear interpolation of z and color.
//
// Side effect: this routine will modify the planes in u->z, u->r, etc if ss!=1
{
    if (debug>1)
	cout << "    scan converting " << p << " " << q << " " << r;

    if (u && u->done) u = 0;

    // triangles u and v each share one side and one angle with triangle pqr,
    // so if either triangle u or triangle v has zero area, that implies
    // that triangle pqr has zero area and no interior pixels, so we can return

    if (u && u->area==0 || v->area==0) {
	assert(!u || u->area!=0);
	if (debug>1) {
	    cout << endl << "      empty triangle, not scan converting,";
	    if (u) cout << " u->area=" << u->area;
	    else cout << " no-u";
	    if (v) cout << " v->area=" << v->area;
	    else cout << " no-v";
	    cout << endl;
	}
	return;
    }

    Point2d by_y[3];
    order_triangle_points(by_y, p, q, r);

    // multiply vertex coordinates by ss
    int i;
    for (i=0; i<3; i++) {
	by_y[i].x *= ss;
	by_y[i].y *= ss;
    }

    // save old plane equations before modifying them
    Plane uz, ur, ug, ub, vz, vr, vg, vb;
    if (u) {
	uz = u->z;
	if (emphasis!=0) {
	    ur = u->r;
	    ug = u->g;
	    ub = u->b;
	}
    }
    vz = v->z;
    if (emphasis!=0) {
	vr = v->r;
	vg = v->g;
	vb = v->b;
    }

    // adjust plane equations to compensate for multiplied coordinates
    if (u) {
	u->z.a /= ss; u->z.b /= ss;
	if (emphasis!=0) {
	    u->r.a /= ss; u->r.b /= ss;
	    u->g.a /= ss; u->g.b /= ss;
	    u->b.a /= ss; u->b.b /= ss;
	}
    }
    v->z.a /= ss; v->z.b /= ss;
    if (emphasis!=0) {
	v->r.a /= ss; v->r.b /= ss;
	v->g.a /= ss; v->g.b /= ss;
	v->b.a /= ss; v->b.b /= ss;
    }

    // if we went ahead and divided in all cases instead of using divide_safe,
    // this would occasionally cause startx or endx to contain 0x7fffffff
    // and cause valid scan lines at bottom or top to be missed
    Real dx1 = divide_safe(by_y[1].x - by_y[0].x, by_y[1].y - by_y[0].y);
    Real dx2 = divide_safe(by_y[2].x - by_y[0].x, by_y[2].y - by_y[0].y);
    int y = (int)ceil(by_y[0].y);
    Real frac = y - by_y[0].y;
    Real x1 = by_y[0].x + dx1*frac;
    Real x2 = by_y[0].x + dx2*frac;
    int scancount0 = scancount;

    for(;y<by_y[1].y;y++) {
	scan_line_datadep_supersample(y, this, u, v, x1, x2, ss);
	x1 += dx1;
	x2 += dx2;
    }

    dx1 = divide_safe(by_y[2].x - by_y[1].x, by_y[2].y - by_y[1].y);
    frac = y - by_y[1].y;
    x1 = by_y[1].x + dx1*frac;

    for(;y<=(int)by_y[2].y;y++) {
	scan_line_datadep_supersample(y, this, u, v, x1, x2, ss);
	x1 += dx1;
	x2 += dx2;
    }
    if (debug>1)
	cout << ", " << scancount-scancount0 << " pixels" << endl;
    if (criterion==SUM2) {
	// multiply sum of squared errors by the
	// area of each supersample, 1/(ss*ss)
	if (u) u->err /= ss*ss;
	v->err /= ss*ss;
    }

    // restore plane equations
    if (u) {
	u->z = uz;
	if (emphasis!=0) {
	    u->r = ur;
	    u->g = ug;
	    u->b = ub;
	}
    }
    v->z = vz;
    if (emphasis!=0) {
	v->r = vr;
	v->g = vg;
	v->b = vb;
    }
}


//------- scan conversion for data-dependent triangulation, high level routine

void bbox(const Point2d &p, const Point2d &q, const Point2d &r,
    Real &dx, Real &dy)
{
    Real xmin, xmax, ymin, ymax;
    xmin = MIN(p.x, q.x);	xmax = MAX(p.x, q.x);
    if (r.x<xmin) xmin = r.x;	if (r.x>xmax) xmax = r.x;
    ymin = MIN(p.y, q.y);	ymax = MAX(p.y, q.y);
    if (r.y<ymin) ymin = r.y;	if (r.y>ymax) ymax = r.y;
    dx = xmax-xmin;	// width
    dy = ymax-ymin;	// height
}

void SimplField::scan_triangle_datadep
    (const Point2d &p, const Point2d &q, const Point2d &r,
    FitPlane *u, FitPlane *v)
// Decide whether supersampling is necessary and call the appropriate routine
// to scan convert triangle pqr
// Side effect: this routine will modify the planes in u->z, u->r, etc if ss!=1
{
    Real zrange = H->zmax();	// should probably be zmax-zmin
    if (zrange<=0) zrange = 1;
    w2 = emphasis * zrange/3;
    w1 = 1-emphasis;

    // decide if supersampling is necessary to accurately measure the error
    // between the input data and the linear approximation
    Real area = TriArea(p, q, r)/2;
    if (area<1e-5) return;
	// sometimes, the triangle's area is zero, but the area
	// variable contains numbers like 1e-13 or -1e-13 because of
	// roundoff error, hence the check above
    Real dx, dy;
    bbox(p, q, r, dx, dy);
    int ss = (int)ceil((dx+dy)/(2*area*area_thresh));
    if (debug)
	cout << "  area=" << area << ", dx=" << dx << " dy=" << dy
	    << " ss=" << ss << endl;

    if (ss==1) scan_triangle_datadep_normal(p, q, r, u, v);
    else scan_triangle_datadep_supersample(p, q, r, u, v, ss);
    if (ss>1) nsuper++;
    nscan++;
}
