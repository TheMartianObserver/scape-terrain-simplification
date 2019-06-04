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

/*  This is a resample program for USGS DEM Quads */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

FILE *fin, *fout;

void define_DEM( void );
void resample_DEM( void );

float *dem_x, *dem_y, *dem_z;

float xul=FLT_MAX, yul=FLT_MIN, xlr=FLT_MIN, ylr=FLT_MAX;
char filein[80], fileout[80];
float mean1, mean2, mean3, total1, total2, total3;

int count=0, command=0;
float emax, emin;

/*-----------------------------------------------*/
main( int argc, char *argv[] )
{

    if( argc > 1 )
    {  
	command=1;
	strcpy( filein, argv[2] );
	strcpy( fileout, argv[1] );
    }

    define_DEM();

    resample_DEM();

    free( (float *)dem_x );
    free( (float *)dem_y );
    free( (float *)dem_z );

    return 0;
}

/*------------------------------------------------------------*/
float max( float x1, float x2)
{
    if( x1 > x2 ) return( x1);
    return( x2);
}

/*------------------------------------------------------------*/
float min( float x1, float x2)
{
    if( x1 < x2 ) return( x1 );
    return( x2);
}

/*------------------------------------------------------------*/
void define_DEM( void )
{
    char filename[80];
    int xmax, num;
    int i, elev;
    float x, y, startx, starty;

    if( ! command )
    {
	printf("Please Enter the Name of the DEM to resample\n");
	scanf("%s", filename );
    }
    else
    {
	strcpy( filename, filein );
    }

    if( (fin = fopen(filename, "rt") )==NULL)
    {
	printf("Can't Open %s\n", filename );
	exit(0);
    }

    dem_x = (float *)malloc( sizeof( float ) );
    dem_y = (float *)malloc( sizeof( float ) );
    dem_z = (float *)malloc( sizeof( float ) );



    for(i=0; i<4; i++)
    {
	fscanf( fin, "%f", &x);
	fscanf( fin, "%f", &y);
	xul = min( xul, x);
	yul = max( yul, y);
	xlr = max( xlr, x);
	ylr = min( ylr, y);
    }

    printf("DEM Dimensions are [%.3f, %.3f] --> [%.3f, %.3f]\n", xul, yul, xlr, ylr);

    fscanf(fin, "%f", &emin);     /* Throw away the min and max */
    fscanf(fin, "%f", &emax);

    fscanf( fin, "%d", &xmax );   /* How many profiles are there in total? */

    for( i=0; i<xmax; i++)       /* Lets read in all of the data cleanly */
    {
	printf(".");
	fscanf(fin, "%d", &num);   /* read in profile number */
	fscanf( fin, "%d", &num);  /* read in the number of elements */

	dem_x = (float *)realloc( dem_x, (count+num)*sizeof(float) );
	dem_y = (float *)realloc( dem_y, (count+num)*sizeof(float) );
	dem_z = (float *)realloc( dem_z, (count+num)*sizeof(float) );

	fscanf( fin, "%f %f", &startx, &starty );   /* read in X & Y coords */
	for( x = 0; x<num; x++)  /* read in the data */
	{
	    fscanf(fin, "%d", &elev);
	    dem_x[count] = startx;
	    dem_y[count] = starty;
	    dem_z[count] = (float)elev;

	    /*  printf("[%f, %f] --> %f\n", dem_x[count], dem_y[count], dem_z[count]);  */
			
	    count++;
	    starty = starty+30.0;
	}

    }

    fclose( fin );          /* end of reading in all of the data */

    return;
}

/*-------------------------------------------------------*/
float distance( float x1, float y1, float x2, float y2)
{
    float d1, d2, d;

    d1 = x2-x1;
    d2 = y2-y1;
    d = d1*d1+d2*d2;
    d = sqrt(d );
    return( d );
}


/*-----------------------------------------------------*/
void resample_DEM( void )
{
    int i, j, q;
    float xs, ys;
    char filename[80];
    float px[7], py[7], pz[7];
    float dist, d[7];
    float px1, py1;

    if( ! command )
    {
	printf("\nPlease Enter the name of the output file\n");
	scanf("%s", filename );
    }
    else
    {
	strcpy( filename, fileout );
    }

    if( (fout=fopen( filename, "wt") ) == NULL )
    {
	printf("Can't create %s\n", filename);
	exit(0);
    }

    for( ys = yul; ys>ylr; ys=ys-30.0  )    /* do 30 m horizontal steps */
    {
	total3 = total2; total2  = total1;  total1 = 0.0;

	for( xs = xul; xs<xlr; xs=xs+30.0)
	{
	    /* printf("Calculating for [%f, %f]\n", xs, ys); */
	    for( q=0; q<6; q++)
		d[q]=FLT_MAX;

	    for(i=0; i<count; i++)    /* Check ALL of the data points */
	    {
		px1 = dem_x[i]; py1 = dem_y[i];
		dist = distance( px1, py1, xs, ys);

		for( j=0; j<6; j++)   /* check to see if this is closer */
		{
		    if( dist < d[j] )  /* it is closer! */
		    {
			/* printf("Dist = %f vs %f\n", dist, d[j]); */
			/* printf("Point[%d]=%.3f, %.3f, D=%.3f\n",i, px1, py1, dist);  */
			for( q = 5; q>j; q--)   /* move the rest off! */ 
			{
			    d[q+1] = d[q];
			    px[q+1] = px[q];
			    py[q+1] = py[q];
			    pz[q+1] = pz[q];
			}             /* End q */
			d[j] = dist;
			px[j] = dem_x[i];
			py[j] = dem_y[i];
			pz[j] = dem_z[i];
			goto outofj;
		    }  /* endif */

		} /* end j */
	    outofj:
	    }  /* end i */

	    px1 = (pz[0]/d[0])+(pz[1]/d[1])+(pz[2]/d[2])+(pz[3]/d[3])+(pz[4]/d[4])+(pz[5]/d[5]);
	    px1 = px1/((1/d[0])+(1/d[1])+(1/d[2])+(1/d[3])+(1/d[4])+(1/d[5]) );
	    total1 = total1+px1;        /* add to the total distance */

	    /*		 printf("Estimate Elevation = %f\n", px1); */

	    fprintf(fout, "%d ", (int)px1);      /* save the elevation */
	    fflush(fout);

	}   /* end xs */
	fprintf(fout, "\n");
    }    /* end ys */

    fclose(fout);
    return;
}
