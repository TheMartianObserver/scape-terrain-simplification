#include "scape.H"

int limit = 100;
int datadep = 0;

Real alpha=1.0;
int multinsert=0;

Real thresh=0.0;
int parallelInsert=0;

Real emphasis = 0;
Real qual_thresh = .5;	// quality threshold
Criterion criterion = SUMINF;
int debug = 0;


char *texFile = NULL;
char *stmFile = NULL;

static char option_usage[] = "Options: \n\
-datadep                      do data dependent triangulation\n\
-delaunay                     do Delaunay triangulation [default]\n\
-qthresh <quality_thresh>     set threshold for data-dep tri. [default=.5]\n\
-sum                          use sum of max err for data-dep tri. [default]\n\
-max                          use max of max err for data-dep tri.\n\
-sqerr			      use sum of squared err for data-dep tri.\n\
-abn                          use angle between normals for datap-dep tri.\n\
-frac <area_thresh>	      set threshold for supersampling [default=1e30]\n\
-npoint <#points>             set number of points to select [default=100]\n\
-tex <texturefile> <emphasis> set texture and its emphasis [default=0]\n\
-debug <debuglevel>           set debugging level [default=0]\n\
-fracthresh <alpha>           use fractional threshold parallel insertion\n\
-constthresh <thresh>         use constant threshold parallel insertion\n\
";



static void usage(char *progname)
{
    cerr << "Usage:" << endl;
    cerr << progname << " filename [options]" << endl;
    cerr << option_usage << endl;
    exit(1);
}


void parse_cmdline(int argc, char *argv[])
{
    if( argc<2 ) usage(argv[0]);
    if( argv[1][0] == '-' ) usage(argv[0]);

    int i;

    stmFile = argv[1];

    for(i=2;i<argc;i++) {
	if (!strcmp(argv[i], "-datadep"))
	    datadep = 1;
	else if (!strcmp(argv[i], "-delaunay"))
	    datadep = 0;
	else if (!strcmp(argv[i], "-npoint") && i+1<argc)
	    limit = atoi(argv[++i]);
	else if (!strcmp(argv[i], "-qthresh") && i+1<argc)
	    qual_thresh = atof(argv[++i]);
	else if (!strcmp(argv[i], "-tex") && i+2<argc) {
	    texFile = argv[++i];
	    emphasis = atof(argv[++i]);
	}
	else if (!strcmp(argv[i], "-debug") && i+1<argc)
	    debug = atoi(argv[++i]);
	else if (!strcmp(argv[i],"-constthresh") && i+1<argc) {
	    parallelInsert = 1;
	    thresh = atof(argv[++i]);
	}
	else if( !strcmp(argv[i],"-fracthresh") && i+1<argc) {
	    multinsert = 1;
	    alpha = atof(argv[++i]);
	}
	else if (!strcmp(argv[i], "-frac") && i+1<argc)
	    area_thresh = atof(argv[++i]);
	else if (!strcmp(argv[i], "-sum"))
	    criterion = SUMINF;
	else if (!strcmp(argv[i], "-max"))
	    criterion = MAXINF;
	else if (!strcmp(argv[i], "-sqerr"))
	    criterion = SUM2;
	else if (!strcmp(argv[i], "-abn"))
	    criterion = ABN;
	else {
	    usage(argv[0]);
	}
    }
}
