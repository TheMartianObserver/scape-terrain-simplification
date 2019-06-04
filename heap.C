//
// heap.C
//
// This file implements the basic heap structure used by the simplification
// software.

#include "scape.H"

// This is used for accounting purposes
int heap_cost = 0;



// Heap::swap --
//
// Swaps two nodes in the heap.
//
void Heap::swap(int i,int j)
{
    heap_node tmp = node[i];

    node[i] = node[j];
    node[j] = tmp;

    node[i].tri->set_location(i);
    node[j].tri->set_location(j);

    heap_cost++;
}


// Heap::upheap --
//
// The given node will be moved up in the heap, if necessary.
//
// NOTE: This function (as well as downheap) performs more swapping
// than is strictly necessary.
//
void Heap::upheap(int i)
{
    if( i==0 ) return;

    if( node[i].val > node[parent(i)].val ) {
	swap(i,parent(i));
	upheap(parent(i));
    }
}

// Heap::downheap --
//
// The given node is moved down through the heap, if necessary.
//
void Heap::downheap(int i)
{
    if (i>=size) return;	// perhaps just extracted the last

    int largest = i,
	l = left(i),
	r = right(i);

    if( l<size && node[l].val > node[largest].val ) largest = l;
    if( r<size && node[r].val > node[largest].val ) largest = r;

    if( largest != i ) {
	swap(i,largest);
	downheap(largest);
    }
}

// Heap::insert --
//
// Insert the given triangle into the heap using the specified key value.
//
void Heap::insert(Triangle *t,Real v)
{
    int i = size++;

    node[i].tri = t;
    node[i].val = v;

    node[i].tri->set_location(i);

    upheap(i);
}

// Heap::extract --
//
// Extract the top element from the heap and return it.
//
heap_node *Heap::extract()
{
    if( size<1 ) return 0;

    swap(0,size-1);
    size--;

    downheap(0);

    node[size].tri->set_location(NOT_IN_HEAP);

    return &node[size];
}

// Heap::kill --
//
// Kill a given node in the heap.
//
heap_node& Heap::kill(int i)
{
    if( i>=size )
	cerr << "ATTEMPT TO DELETE OUTSIDE OF RANGE" << endl;

    swap(i,size-1);
    size--;

    if( node[i].val < node[size].val )
	downheap(i);
    else
	upheap(i);

    node[size].tri->set_location(NOT_IN_HEAP);

    return node[size];
}


// Heap::update --
//
// This function is called when the key value of the given node has
// changed.  It will record this change and reorder the heap if
// necessary.
//
void Heap::update(int i,Real v)
{
    assert(i < size);


    Real old=node[i].val;
    node[i].val = v;

    if( v<old )
	downheap(i);
    else
	upheap(i);
}
