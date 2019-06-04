#include "scape.H"
#include <stdarg.h>


void Model::configure(int len, ...)
{
    va_list ap;
    int index = 0;

    va_start(ap, len);
    while( index < len ) {
        process_key(va_arg(ap,model_key),va_arg(ap,long));
	index += 2;
    }
    va_end(ap);
}


void Model::render(int len, ...)
{
    va_list ap;
    int index = 0;

    va_start(ap, len);
    while( index < len ) {
        process_key(va_arg(ap,long),va_arg(ap,long));
	index += 2;
    }
    va_end(ap);


    just_render();
}
