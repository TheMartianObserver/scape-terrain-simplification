#include "Basic.H"

#include <stdarg.h>


extern "C" {

    extern int errno;
    extern void perror(const char *);
}


void report_error(char *msg,char *file,int line)
{
    cerr << msg << " (line " << line <<"): " << file << endl;
    if( errno )
	perror("system");

    exit(1);
}

void assert_failed(char *text,char *file,int line)
{
    cerr << "Assertion failed: {" << text <<"} in " << file;
    cerr << " line " << line << endl;

  abort();
}
