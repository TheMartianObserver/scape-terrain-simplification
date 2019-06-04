////////////////////////////////////////////////////////////////////////
// This code is a modified version of the Delaunay triangulator
// written by Dani Lischinski
// in Graphics Gems IV
////////////////////////////////////////////////////////////////////////


// in a triangulation, the topology looks like this:
//
//                    Dest
//                     @
//                   * * *
//         Lnext | *   *   * | Rprev
//               *--   *   --*
//             *       *       *
//   Dprev --*        /*\        *-- Dnext
//         * |     Sym *         | *
//       *             *             *
//     @               *               @
//       *             *             *
//         * |         * e       | *
//   Lprev --*        \*/        *-- Rnext
//             *       *       *
//               *--   *   --*
//         Onext | *   *   * | Oprev
//                   * * *
//                     @
//                    Org

#ifndef QUADEDGE_H
#define QUADEDGE_H

#include "geom2d.H"

class QuadEdge;
class Triangle;
class Edge;
typedef void (*edge_callback)(Edge *,void *);
typedef void (*face_callback)(Triangle *,void *);


#define NOT_IN_HEAP   -47
    // value for Triangle->heap_index indicating no candidate in heap
#define UNSCANNED -1
    // value for Triangle->err indicating triangle hasn't been scan converted


class Edge {
    friend QuadEdge;
    friend void Splice(Edge*, Edge*);
private:
    int num;
    Edge *next;
    Point2d *data;
    Triangle *lface_data;
public:
    Edge()			{ data = 0; lface_data = NULL; }
    Edge* Rot();
    Edge* invRot();
    Edge* Sym();
    Edge* Onext();
    Edge* Oprev();
    Edge* Dnext();
    Edge* Dprev();
    Edge* Lnext();	
    Edge* Lprev();
    Edge* Rnext();	
    Edge* Rprev();
    Point2d* Org();
    Point2d* Dest();
    const Point2d& Org2d() const;
    const Point2d& Dest2d() const;
    void  EndPoints(Point2d*, Point2d*);
    QuadEdge* Qedge()           { return (QuadEdge *)(this - num); }

    int CcwPerim();
	// returns 1 if a counterclockwise perimeter edge,
	// 0 if a clockwise perimeter edge or internal edge
    Triangle *Lface() { return lface_data; }
    void set_Lface(Triangle *f) { lface_data = f; }
    void OverEdges(unsigned int,edge_callback,void *closure);

    friend ostream& operator<<(ostream&,Edge *);
};

class QuadEdge {
	friend Edge *MakeEdge();
  private:
	Edge e[4];
	unsigned int ts;
  public:
	QuadEdge();
	int TimeStamp(unsigned int);
};


class Subdivision {
private:
    Edge *startingEdge;
    Triangle *first_face;

    Triangle *make_face(Edge *);
    void rebuild_face(Edge *);
protected:
    void init(const Point2d&,const Point2d&,const Point2d&,const Point2d&);
    Subdivision() { }
public:
    Edge *Locate(const Point2d& x, Edge *hintedge);
    Subdivision(const Point2d& a,const Point2d& b,
		const Point2d& c ,const Point2d& d)
	{ init(a,b,c,d); }
    Edge *Spoke(const Point2d& x, Triangle *tri);
    Edge *InsertSite(const Point2d&, Triangle *tri);

    int is_interior(Edge *);

    void OverEdges(edge_callback,void *closure);
    void OverFaces(face_callback,void *closure);
    void vef(int &nv, int &ne, int &nf);
};

inline QuadEdge::QuadEdge()
{
	e[0].num = 0, e[1].num = 1, e[2].num = 2, e[3].num = 3;
	e[0].next = &(e[0]); e[1].next = &(e[3]);
	e[2].next = &(e[2]); e[3].next = &(e[1]);
	assert(!e[0].lface_data);
	assert(!e[1].lface_data);
	assert(!e[2].lface_data);
	assert(!e[3].lface_data);
	ts = 0;
}

inline int QuadEdge::TimeStamp(unsigned int stamp)
{
	if (ts != stamp) {
		ts = stamp;
		return TRUE;
	} else 
		return FALSE;
}

/************************* Edge Algebra *************************************/

inline Edge* Edge::Rot()
// Return the dual of the current edge, directed from its right to its left. 
{
	return (num < 3) ? this + 1 : this - 3;
}

inline Edge* Edge::invRot()
// Return the dual of the current edge, directed from its left to its right. 
{
	return (num > 0) ? this - 1 : this + 3;
}

inline Edge* Edge::Sym()
// Return the edge from the destination to the origin of the current edge.
{
	return (num < 2) ? this + 2 : this - 2;
}
	
inline Edge* Edge::Onext()
// Return the next ccw edge around (from) the origin of the current edge.
{
	return next;
}

inline Edge* Edge::Oprev()
// Return the next cw edge around (from) the origin of the current edge.
{
	return Rot()->Onext()->Rot();
}

inline Edge* Edge::Dnext()
// Return the next ccw edge around (into) the destination of the current edge.
{
	return Sym()->Onext()->Sym();
}

inline Edge* Edge::Dprev()
// Return the next cw edge around (into) the destination of the current edge.
{
	return invRot()->Onext()->invRot();
}

inline Edge* Edge::Lnext()
// Return the ccw edge around the left face following the current edge.
{
	return invRot()->Onext()->Rot();
}

inline Edge* Edge::Lprev()
// Return the ccw edge around the left face before the current edge.
{
	return Onext()->Sym();
}

inline Edge* Edge::Rnext()
// Return the edge around the right face ccw following the current edge.
{
	return Rot()->Onext()->invRot();
}

inline Edge* Edge::Rprev()
// Return the edge around the right face ccw before the current edge.
{
	return Sym()->Onext();
}

/************** Access to data pointers *************************************/

inline Point2d* Edge::Org()
{
	return data;
}

inline Point2d* Edge::Dest()
{
	return Sym()->data;
}

inline const Point2d& Edge::Org2d() const
{
	return *data;
}

inline const Point2d& Edge::Dest2d() const
{
	return (num < 2) ? *((this + 2)->data) : *((this - 2)->data);
}

inline void Edge::EndPoints(Point2d* or, Point2d* de)
{
	data = or;
	Sym()->data = de;
}


inline ostream& operator<<(ostream& os,Edge *e)
{
    os << "[" << e->Org2d() << "-->" << e->Dest2d() << "]";
    return os;
}



class Triangle {
    Edge *anchor;	// An edge of this triangle
    int sx,sy;		// The proposed selection point
    int heap_index;	// index into heap
	// ==NOT_IN_HEAP iff this triangle has no candidate in the heap
    Real err;		// error over triangle,
			// used for data-dependent triangulation
			// = sum of squared error if criterion=SUM2,
			// = maximum error if criterion=SUMINF or MAXINF
public:
    Triangle *next;

    Triangle(Edge *e);
    Edge *get_anchor() { return anchor; }
    int locate() { return heap_index; }
    void set_location(int h) { heap_index = h; }
    void set_selection(int x,int y) { sx=x; sy=y; }
    void get_selection(int *x,int *y) { *x=sx; *y=sy; }
    void set_err(Real error) {err = error;}
    Real get_err() {return err;}

    const Point2d& point1() const { return anchor->Org2d(); }
    const Point2d& point2() const { return anchor->Dest2d(); }
    const Point2d& point3() const { return anchor->Lnext()->Dest2d(); }
    void reanchor(Edge *e) { anchor = e; }
    void attach_face();
    void dont_anchor(Edge *e);

    friend ostream& operator<<(ostream&,Triangle *);
};

inline Triangle::Triangle(Edge *e)
{
    heap_index = NOT_IN_HEAP;
    anchor = e;
    err = UNSCANNED;
    attach_face();
}

inline Triangle *Subdivision::make_face(Edge *e)
{
    Triangle *f = new Triangle(e);

    f->next = first_face;
    first_face = f;

    return f;
}

inline ostream& operator<<(ostream& os,Triangle *t)
{
    os << "Triangle" << t->point1() << t->point2() << t->point3();
    return os;
}


class UpdateRegion {
    Edge *start;
    Edge *current;
public:
    UpdateRegion(Edge *);

    Triangle *first();
    Triangle *next();
};

void Swap(Edge* e);

inline Real TriArea(const Point2d& a, const Point2d& b, const Point2d& c)
// Returns twice the area of the oriented triangle (a, b, c), i.e., the
// area is positive if the triangle is oriented counterclockwise.
{
	return (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
}

#endif /* QUADEDGE_H */
