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

/* This is a program which makes USGS DEM data usable on MATLAB 4.0 */

#include <stdio.h>

float x, y;
int totalprofiles;
int profiles;
int length=0;
double minimum, maximum;
int x1, y1;
int profile[1000];

int dum;
int elevation;
int cx, cy;
float skew;

int dem[1000][1000];
FILE *fin, *fout;
char buffer[80];

main()
{

    for(cx=0; cx<1000; cx++)
    {
	for(cy=0; cy<1000; cy++)
	    dem[cx][cy]=-32767;
    }

    puts("Please Enter the name of the DEM file to convert:");
    scanf( "%s", buffer);

    if( (fin = fopen( buffer, "r" )) == 0L )
	puts("Can't open file");

    puts("Please enter the name of the destination files");
    scanf( "%s", buffer);

    fout = fopen( buffer, "wt");

    puts("Please enter the skew calc factor (0.00 - 1.00)");
    scanf("%f", &skew);

    do_conversion();
    fclose( fin, fout);
}

do_conversion()
{
    int i, j;

    for( i=0; i<4; i++)
    {
	printf("Doing Loop #%d\n", i);
	fscanf( fin, "%f", &x );
	fscanf( fin, "%f", &y );
    }
    fscanf(fin, "%lf", &minimum );
    fscanf(fin, "%lf", &maximum );
    fscanf( fin, "%d", &totalprofiles);

    printf("Minimum = %f\n", minimum);
    printf("Maximum = %f\n", maximum);
    printf("Number of Profiles = %d\n", totalprofiles);

    printf("Loading in the File.....\n");

    for(i=0; i<totalprofiles; i++)
    {
	fscanf(fin, "%d", &dum); /* read in profile number */
	fscanf( fin, "%d", &profiles);  /* read in data points in profile */
	if( length < profiles) length = profiles;
	fscanf( fin, "%lf %lf", &x, &y); /* read in x and y coordinates */
	for( j=0; j<profiles; j++)
	{
	    fscanf( fin, "%d", &elevation );
	    dem[i][j]=elevation;
	}
    }

    printf("Trying to Rearrange the Skew...\n");
    /* rearrange the western edge such that the skew is correct */
    for( cx=(int)(skew*totalprofiles); cx<totalprofiles; cx++ )  /* Go across the row to check */
    {
	printf("Checking Profile #%d\r", cx);
	for(cy=0; cy<length; cy++)
	{
	    if( dem[cx][cy] == -32767 )
	    {
		printf("Rearranging Profile %d\n", cx);
		for(y1=0; y1<length; y1++)    /* temporarily make a new profile */
		    profile[y1] = dem[cx][y1];
      
		for(y1=length; y1>-1; y1--)
		    dem[cx][y1] = profile[ length-y1];
		break;
	    }
     
	}
    }

    /* even out the data to the left of center */
    printf("Evening out the data....\n");

    for(cy=0; cy<length; cy++)  /* go down from N to S */
    {
	for(cx=150; cx>-1; cx--)   /* Go west from the 150 profile */
	{
	    if( dem[cx][cy] == -32767 )
	    {
		if( dem[cx+1][cy] = -32767) dem[cx+1][cy]=minimum;
		dem[cx][cy] = dem[cx+1][cy];
	    }
	}
	for(cx=150; cx<totalprofiles; cx++) /* Go East from the 150 profile */
	{
	    if( dem[cx][cy] == -32767 )
	    {
		if( dem[cx-1][cy] = -32767) dem[cx-1][cy] = minimum;
		dem[cx][cy] = dem[cx-1][cy];
	    }
	}
    }

    printf("Writing the data to a flatfile\n");
    for( cy=0; cy<length; cy++) /* routine to write out the flatfile */
    {
	for(cx=0; cx<(totalprofiles-1); cx++)
	    fprintf( fout, "%d ", dem[cx][cy] );
	fprintf(fout, "%d\n", dem[totalprofiles-1][cy] );
    }

}
