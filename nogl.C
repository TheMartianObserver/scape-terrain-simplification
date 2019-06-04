//
// For compilation without GL, this file fills in dummy routines
// for the rendering related terrain functions.
//

#include "scape.H"


void HField::emit(Real,Real)
{
}

void HField::draw_from_point(int,int)
{
}


long HField::eval_key(model_key)
{
    return NULL;
}


void HField::process_key(model_key,long)
{
}


void HField::just_render()
{
}



void SimplField::emit(Real,Real,Real)
{
}


void SimplField::emit_origin(Edge *)
{
}

void SimplField::render_face(Triangle *)
{
}

long SimplField::eval_key(model_key)
{
    return NULL;
}

void SimplField::process_key(model_key,long)
{
}

void SimplField::just_render()
{
}



gl_win View::init_viewport(char *)
{
    return NULL;
}

void View::redraw()
{
}
