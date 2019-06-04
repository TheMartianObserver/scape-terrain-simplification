/*
 * This code was originally written by:
 *
 *       Christopher Keane
 *       Research Associate
 *       Joint Education Initiative
 *       3433 A.V. Williams
 *       University of Maryland
 *       College Park, Maryland 20742
 *       (301) 405-2324
 *       keane@earthsun.umd.edu
 *
 * I've reformatted it to make it more readable.   -- Michael Garland
 *
 */

#include <stdio.h>
#include <string.h>

main( argc, argv )
int argc;
char *argv[];

{  
    char infile[24];
    char outfile[24];
    char *dum;

    FILE *InDEM, *OutDEM;
    unsigned count;
    char ic, ic1;

    if( argc == 2 )
    {
	strcpy( infile, argv[1] );
	strcpy( outfile, "temporary.temporary" );
    }else{

	printf("Please enter the name of the source file:");
	gets( infile );
	strcpy( outfile, "temporary.temporary");
    }

    if ((InDEM = fopen(infile,"rb")) == NULL)
    {
	fprintf(stderr,"Cannot open input file: %s\n",infile);
	exit(-1);
    }
    if ((OutDEM = fopen(outfile,"wb")) == NULL)
    {
	fprintf(stderr,"Cannot create output file\n");
	exit(-1);
    }

    count = 0;
    ic = fgetc(InDEM);

    while (!feof(InDEM))
    {
	ic1 = ic;
	ic = fgetc(InDEM);
	if (((ic == '-') || (ic == '+')) && (ic1 == 'D'))
	{
	    count++;
	    printf("Precision Change #%d\r",count); 
	    fputc('E',OutDEM);
	}
	else fputc(ic1,OutDEM);
    }
    fputc(ic,OutDEM);
    fclose(InDEM);
    fclose(OutDEM);

    do_parse( infile );

    return(0);
}

do_parse(char *infile)
{
    FILE *f1, *f2; 
    char filename1[25], *filename2, dum[128], name[60];
    int code, junk, i, j, profiles, points;
    double coord, junk1;
    float junk2;
    int x, y, z;

    strcpy( filename1, "temporary.temporary");
    filename2 = strcat( infile, ".dat" );

    f1 = fopen( filename1, "r");
    if( f1 == NULL )
    {
        puts("Error in opening file....");
        exit(0);
    }
    f2 = fopen( filename2, "wb");


    fgets( name, 40, f1 );          /* read in the DEM file name */

    fseek( f1, 145, 0);             /* skip all kinds of trash */

    fscanf( f1, " %d", &code );      /* Read in Level Code */

    fscanf( f1, " %d", &code );      /* read in Pattern Code */

    fscanf( f1, " %d", &code );      /* read in planimetric system */

    fscanf( f1, " %d", &junk);

    for( i=0; i<15; i++){
	fscanf( f1, " %lf", &junk1 );
    }

    fscanf( f1, " %d", &code );   /* read in palnar unit of measure */

    fscanf( f1, " %d", &code );   /* read in elevation unit of measure */

    fscanf( f1, " %d", &junk);

    for( i=0; i<4; i++){          /* read in corner points */
	fscanf( f1, " %lf", &coord);
	fprintf(f2, "%lf\n", coord);

	fscanf( f1, " %lf", &coord);
	fprintf(f2, "%lf\n", coord);
    }

    fscanf( f1, " %lf", &junk1);
    fprintf(f2, "%lf\n", junk1 );

    fscanf( f1, " %lf", &junk1);
    fprintf(f2, "%lf\n", junk1 );

    fscanf( f1, " %lf", &junk1 );

    fscanf(f1, " %d", &code );

    fscanf(f1,  " %f %f %f", &junk2, &junk2, &junk2);

    fscanf(f1, " %d %d", &points, &profiles );
    fprintf(f2, "%d\n", profiles);


    for( x=0; x<profiles; x++)      /* loop for all profiles */
    {
	printf("Profile #%d\r", x);
	fscanf(f1, " %d %d", &junk, &code );
	fprintf(f2, "%d\n", code );

	fscanf(f1, " %d %d", &points, &junk );
	fprintf(f2, "%d\n", points);
  
	fscanf( f1, " %lf %lf", &junk1, &coord);
	fprintf(f2, "%lf %lf\n", junk1, coord);

	fscanf( f1, " %lf", &junk1 );

	fscanf( f1, " %lf %lf", &junk1, &coord ); 

	for( y=0; y<points; y++)
	{
	    fscanf( f1, " %d", &code );
	    fprintf(f2, "%d\n", code );
	}

    }
    printf("The parsing of the file is finished....\n");
    fclose( f1 );
    fclose(f2);
    unlink( filename1);
    exit(0);
}

getkey()
{
    char buffer[40];

    gets( buffer );
    return(0);
}
