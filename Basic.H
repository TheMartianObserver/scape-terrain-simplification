#ifndef BASIC_H_INCLUDED
#define BASIC_H_INCLUDED

//
// Basic.H
//
// Just defines some simple little classes and functions to make
// life a little easier.

#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)>(b))?(b):(a))
#endif

#ifndef ABS
#define ABS(x) (((x)<0)?-(x):(x))
#endif

#ifndef True
#define True 1
#define False 0
#endif

typedef unsigned int bool;

#define Nil NULL


#define fatal_error(s)  report_error(s,__FILE__,__LINE__)

#ifdef assert
#undef assert
#endif
#define  assert(i)  (i)?((void)NULL):assert_failed(# i,__FILE__,__LINE__)

extern void assert_failed(char *,char *,int);
extern void report_error(char *,char *,int);



// Basic array classes

template<class T>
class array {
protected:
    T *data;
    int len;
public:
    array() { data=NULL; len=0; }
    array(int l) { init(l); }
    ~array() { if( data ) delete[] data; }

    void init(int l) { len=l; data=new T[l]; }
    void free() { delete[] data; }
    void resize(int l) {
	T *old = data;
	data = new T[l];
	data = (T *)memcpy(data,old,MIN(len,l)*sizeof(T));
	len = l;
	delete[] old;
    }

    T& ref(int i) {
#ifdef SAFETY
	assert( data );
	assert( i>=0 && i<len );
#endif
	return data[i];
    }

    T& operator()(int i) { return ref(i); }

    int length() { return len; }
};

template<class T>
class array2 {
protected:
    T *data;
    int width,height;
public:
    array2() { data=NULL; }
    array2(int w,int h) { init(w,h); }
    ~array2() { free(); }

    void init(int w,int h) {
	data = new T[w*h];
	width = w; height = h;
    }
    void free() { delete[] data; }


    T& ref(int i,int j) { 
#ifdef SAFETY
	assert( data );
	assert( i>=0 && i<width );
	assert( j>=0 && j<height );
#endif
	return data[j*width + i];
    }

    T& operator()(int i,int j) { return ref(i,j); }

    int w() { return width; }
    int h() { return height; }

    void bitread(ifstream& in)
    { 
#ifdef SAFETY
	assert( data );
#endif
	char *loc = (char *)data;
	int target = width*height*sizeof(T);
	while( target>0 ) {
	    in.read(loc,target);
	    target -= in.gcount();
	    loc += in.gcount();
	}
    }


    T *raw() { return data; }
    void set_raw(T *d) { data = d; }
};


template<class T>
class buffer {
    array<T> data;
    int fill;
public:
    buffer(int len=20) { data.init(len); fill=0; }
    ~buffer() { data.free(); }

    void reset() { fill=0; }
    void freeze() { data.resize(fill); }
    void insert(T x) {
	if( fill >= data.length() )
	    data.resize(data.length()*2);
	data(fill++) = x;
    }
    int length() { return fill; }

    T& ref(int i) { return data.ref(i); }
    T& operator()(int i) { return data.ref(i); }
};


#endif   // BASIC_H_INCLUDED
