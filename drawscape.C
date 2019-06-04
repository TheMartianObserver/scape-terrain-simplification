//
// drawscape.C
//
// This is a quick little program to allow users to look at the
// full resolution height fields (for purposes of comparison).
//

#include <gl.h>
#include <device.h>

#include "scape.H"

int width,height;
float heightscale;

// handle_expose --
//
// This function will be called whenever an expose event is generated
// in the viewing window.
//
int handle_expose(View *v,Event *event)
{
    if( event->dev != REDRAW ) return 0;

    v->redraw();

    return 1;
}

// do_rotation --
//
// A simple little function that allows for a fairly primitive
// spin interaction.
//
void do_rotation(View *v,float angle,Device d)
{
    long statex,statey,newx,newy;

    mmode(MVIEWING);

    statex = getvaluator(MOUSEX);
    statey = getvaluator(MOUSEY);

    while( getbutton(d) ) {
	newx = getvaluator(MOUSEX);
	newy = getvaluator(MOUSEY);

	if( newx-statex || newy-statey ) {
	    v->inc_rot(angle*(float)(statey-newy),
		       angle*(float)(newx-statex),
		       0);

	    v->redraw();

	    statex = newx;
	    statey = newy;
	}

    }
}

// handle_input --
//
// Function to handle input events in the viewing window.
//
int handle_input(View *v,Event *event)
{
    Model *m = v->get_model();

    switch( event->dev ) {
    case LEFTMOUSE:
	do_rotation(v,1.0,LEFTMOUSE);
	break;

    case RIGHTMOUSE:
    case MIDDLEMOUSE:
	cout << "Mouse" << endl;
	break;
	
    case KEYBD: {
	char c = (char)event->val;
	switch( c ) {
	case 'q':
	    winclose(v->win());
	    exit(0);
	    break;

	case 'v':
	    m->set_key(RENDER_WITH_COLOR,
		       !m->eval_key(RENDER_WITH_COLOR));
	    qenter(REDRAW,(short)v->win());
	    break;

	case 'f':
	    m->set_key(RENDER_AS_SURFACE,
		       !m->eval_key(RENDER_AS_SURFACE));
	    qenter(REDRAW,(short)v->win());
	    break;


	case '+':
	    v->mult_scale(1,1,2);
	    v->redraw();
	    break;
	case '-':
	    v->mult_scale(1,1,.5);
	    v->redraw();
	    break;

	}
	break;
    }
    default:
	return 0;
    }

    return 1;
}


void render(HField& ter)
{
    View view(&ter,"Scape Terrain Viewer");
    EventHandler input(handle_input);
    EventHandler expose(handle_expose);

    view.push_handler(&input);
    view.push_handler(&expose);

    qdevice(LEFTMOUSE);
    qdevice(KEYBD);

    qreset();
    qenter(REDRAW,(short)view.win());

    view.scale(1,1,heightscale);

    for(int done=0;!done;) {
	Event event;

	event.dev = qread(&event.val);
	view.event_dispatch(&event);
    }
}


main(int argc,char **argv)
{
    texFile = NULL;
    parse_cmdline(argc, argv);

    cout << "Opening input file " << stmFile << endl;

    ifstream mntns(stmFile);
    HField ter(mntns,texFile);
    width = ter.get_width();
    height = ter.get_height();

    // Real zrange = ter.zmax() - ter.zmin();
    // heightscale = .3*width/(zrange ? zrange : 1);

    render(ter);
}
