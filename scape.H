#include "Basic.H"
#include <math.h>

#include "quadedge.H"
#include "stuff.H"

extern void parse_cmdline(int argc, char *argv[]);

extern char *texFile;
extern char *stmFile;

extern Real emphasis;

extern int datadep;	// triangulation method: 1=data-dependent, 0=Delaunay

extern Real qual_thresh;	// quality threshold, 0<=thresh<=1
				// thresh=0 means pure data-dependent
				// thresh=.5 is a good value
				// thresh=1 means pure shape-dependent
					// (similar to Delaunay)

enum Criterion {SUMINF, MAXINF, SUM2, ABN};
// criteria for triangulating a quadrilateral, if doing data-dep. triangulation
//	SUMINF means minimize the sum of the Linf (maximum absolute) errors
//	MAXINF means minimize the maximum of the Linf (maximum absolute) errors
//	SUM2 means minimize the sum of the L2 (squared) errors

extern Criterion criterion;

extern Real area_thresh;// Maximum fraction of triangle area that is permitted
			// to be partially covered by samples.
			// Controls supersampling resolution.
			// 0 => infinite supersampling
			// .8 => moderate supersampling
			// 1e30 => no supersampling

extern int scancount;		// count of #pixels scanned during an update

extern int nscan, nsuper; // count of triangles scan converted & supersampled

extern int debug;	// debugging level: 0=none, 1=some, 2=more


extern Real thresh;
extern int parallelInsert;
extern int multinsert;
extern int limit;
extern Real alpha;



#define DEM_BAD 65535   // height value in DEM file of points to be ignored

#include "views.H"

#include "hfield.H"
#include "simplfield.H"
