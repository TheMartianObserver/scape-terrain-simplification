//
// scape.C
//
// The basic toplevel program to drive the terrain simplification.
//

#include "scape.H"
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

int width,height;
Real heightscale = .2;

extern int update_cost;

ostream *tin_out = NULL;




double get_time()
{
    struct rusage t;

    getrusage(RUSAGE_SELF,&t);

    return (double)t.ru_utime.tv_sec + (double)t.ru_utime.tv_usec/1000000;
}


void ps_edge(Edge *e,void *closure)
{
    ostream& out = *(ostream *)closure;

    const Point2d& a = e->Org2d();
    const Point2d& b = e->Dest2d();
    out << a.x << " " << a.y << " ";
    out << b.x << " " << b.y << " L" << endl;
}

void ps_face(Triangle *t,void *closure)
{
    ostream& out = *(ostream *)closure;
    int x,y;

    t->get_selection(&x,&y);
    out << x << " " << y << " C" << endl;
}

void output_ps(SimplField& ter)
{
    ofstream out("tin.eps");
    out << "%!PS-Adobe-2.0 EPSF-2.0" << endl;
    out << "%%Creator: Scape" << endl;
    out << "%%BoundingBox: 0 0 " << ter.original()->get_width()-1 << " ";
    out << ter.original()->get_height()-1 << endl;
    out << "%%EndComments" << endl;
    out << "/L {moveto lineto stroke} bind def" << endl;
    out << "/C {newpath 2.5 0 360 arc closepath fill} bind def" << endl;
    out << ".2 setlinewidth" << endl;

    ter.OverEdges(ps_edge,(ostream *)&out);
    out << ".3 setgray" << endl;
    ter.OverFaces(ps_face,(ostream *)&out);

    out << "showpage" << endl;
}


void output_face(Triangle *t,void *closure)
{
    SimplField *ter = (SimplField *)closure;
    ostream& tin = *tin_out;

    const Point2d& p1 = t->point1();
    const Point2d& p2 = t->point2();
    const Point2d& p3 = t->point3();

    tin << "t ";

    tin << p1.x << " " << p1.y << " ";
    tin << ter->original()->eval(p1)*heightscale << "   ";

    tin << p2.x << " " << p2.y << " ";
    tin << ter->original()->eval(p2)*heightscale << "   ";

    tin << p3.x << " " << p3.y << " ";
    tin << ter->original()->eval(p3)*heightscale << endl;
}


void write_mesh(SimplField& ter)
{
    ofstream tin("out.tin");
    tin_out = &tin;

    ter.OverFaces(output_face,&ter);
}




void greedy_insert(SimplField& ter)
{
    int i;
    double start, time = 0.;
    start = get_time();

    for(i=5;i<=limit && ter.select_new_point();i++)
	;


    time += get_time()-start;

    cout << "#" << endl;
    cout << "# Total time: " << time << endl;
}


main(int argc,char **argv)
{
    parse_cmdline(argc, argv);

    if (datadep)
	cout << "# doing data-dependent triangulation" << endl
	    << "#  with "
	    << (criterion==SUMINF ? "sum" : criterion==MAXINF ?
		"max" : criterion==SUM2 ? "sqerr" : "abn")
	    << " criterion, threshold "
	    << qual_thresh << ", and fraction " << area_thresh
	    << endl;
    else
	cout << "# doing Delaunay triangulation" << endl;
    cout << "# emphasis=" << emphasis << " npoint=" << limit << endl;
    if( parallelInsert ) {
	cout << "# Using constant threshold parallel insert:  thresh=";
	cout << thresh << endl;
    }
    if( multinsert ) {
	cout << "# Using fractional threshold insert:  thresha="<<alpha;
	cout << endl;
    }

    ifstream mntns(stmFile);
    HField H(mntns,texFile);
    SimplField ter(&H);

    width  = H.get_width();
    height = H.get_height();

    greedy_insert(ter);
    write_mesh(ter);
    //
    // You can output a PostScript version of the mesh by uncommenting the
    // following line.
    //output_ps(ter);

    return 0;
}
