/*
* libtcod 1.5.1
* Copyright (c) 2008,2009,2010 Jice & Mingos
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Jice or Mingos may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY JICE AND MINGOS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL JICE OR MINGOS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
	WARNING ! This code is not part of libtcod anymore.
	It's the first version of Jonathon Duerig's permissive FOV.
	It has been replaced by Jonathon Duerig enhanced permissive FOV in fov_permissive2.c
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libtcod.h"
#include "libtcod_int.h"

typedef struct {
	int xi,yi,xf,yf;
} line_t;

typedef struct _viewbump_t {
	int x,y;
	int refcount;
	struct _viewbump_t *parent;
} viewbump_t;

typedef struct {
	line_t shallow_line;
	line_t steep_line;
	viewbump_t *shallow_bump;
	viewbump_t *steep_bump;
} view_t;

static view_t **current_view=NULL;
static view_t *views=NULL;
static viewbump_t *bumps=NULL;
static int bumpidx=0;

#define RELATIVE_SLOPE(l,x,y) (((l)->yf-(l)->yi)*((l)->xf-(x)) - ((l)->xf-(l)->xi)*((l)->yf-(y)))
#define BELOW(l,x,y) (RELATIVE_SLOPE(l,x,y) > 0)
#define BELOW_OR_COLINEAR(l,x,y) (RELATIVE_SLOPE(l,x,y) >= 0)
#define ABOVE(l,x,y) (RELATIVE_SLOPE(l,x,y) < 0)
#define ABOVE_OR_COLINEAR(l,x,y) (RELATIVE_SLOPE(l,x,y) <= 0)
#define COLINEAR(l,x,y) (RELATIVE_SLOPE(l,x,y) == 0)
#define LINE_COLINEAR(l,l2) (COLINEAR(l,(l2)->xi,(l2)->yi) && COLINEAR(l,(l2)->xf,(l2)->yf))

static void add_shallow_bump(int x, int y, view_t *view) {
	viewbump_t *shallow, *curbump;
	view->shallow_line.xf=x;
	view->shallow_line.yf=y;
	shallow= &bumps[bumpidx++];
	shallow->x=x;
	shallow->y=y;
	shallow->parent=view->shallow_bump;
	view->shallow_bump=shallow;
	curbump=view->steep_bump;
	while ( curbump ) {
		if ( ABOVE(&view->shallow_line,curbump->x,curbump->y)) {
			view->shallow_line.xi=curbump->x;
			view->shallow_line.yi=curbump->y;
		}
		curbump=curbump->parent;
	}
}

static void add_steep_bump(int x, int y, view_t *view) {
	viewbump_t *steep, *curbump;
	view->steep_line.xf=x;
	view->steep_line.yf=y;
	steep=&bumps[bumpidx++];
	steep->x=x;
	steep->y=y;
	steep->parent=view->steep_bump;
	view->steep_bump=steep;
	curbump=view->shallow_bump;
	while ( curbump ) {
		if ( BELOW(&view->steep_line,curbump->x,curbump->y)) {
			view->steep_line.xi=curbump->x;
			view->steep_line.yi=curbump->y;
		}
		curbump=curbump->parent;
	}
}

static bool check_view(TCOD_list_t active_views, view_t **it) {
	view_t *view=*it;
	line_t *shallow_line=&view->shallow_line;
	line_t *steep_line=&view->steep_line;
	if (LINE_COLINEAR(shallow_line,steep_line)
		&& (COLINEAR(shallow_line,0,1) || COLINEAR(shallow_line,1,0)) ){
//printf ("deleting view %x\n",it);
		// slow !
		TCOD_list_remove_iterator(active_views,(void **)it);
		return false;
	}
	return true;
}
static bool p=true;
static void visit_coords(map_t *m,int startX, int startY, int x, int y, int dx, int dy, 
	TCOD_list_t active_views, bool light_walls) {
	// top left
	int tlx=x, tly=y+1;
	// bottom right
	int brx=x+1, bry=y;
	int realX=x*dx,realY=y*dy;
	int offset;
	view_t *view=NULL;
//printf ("%d %d %d %d / %d %d : %d views\n",x,y,dx,dy,	brx,bry,TCOD_list_size(active_views));
	while (current_view != (view_t **)TCOD_list_end(active_views)) {
		view=*current_view;
//printf("view : %d %d - %d %d\n",view->steep_line.xi,view->steep_line.yi,view->steep_line.xf,view->steep_line.yf);
		if ( ! BELOW_OR_COLINEAR(&view->steep_line,brx,bry) ) {
			break;
		}
		current_view++;
	}
	if ( current_view == (view_t **)TCOD_list_end(active_views) || ABOVE_OR_COLINEAR(&view->shallow_line,tlx,tly)) {
		return;
	}
//if (p) printf ("%d %d shallow %d %d %d %d steep %d %d %d %d\n",startX+x*dx,startY+y*dy,view->shallow_line.xi,view->shallow_line.yi,view->shallow_line.xf,view->shallow_line.yf,view->steep_line.xi,view->steep_line.yi,view->steep_line.xf,view->steep_line.yf);
	offset=startX+realX + (startY+realY)*m->width;
	if ( light_walls || m->cells[offset].transparent ) m->cells[offset].fov=1;
	if (m->cells[offset].transparent==1) return;
	if (  ABOVE(&view->shallow_line,brx,bry) 
		&& BELOW(&view->steep_line,tlx,tly)) {
//printf ("view blocked\n");
		// slow !
		TCOD_list_remove_iterator(active_views,(void **)current_view);
	} else if ( ABOVE(&view->shallow_line,brx,bry)) {
//printf ("shallow bump\n");
		add_shallow_bump(tlx,tly,view);
		check_view(active_views,current_view);
	} else if (BELOW(&view->steep_line,tlx,tly)) {
//printf ("steep bump\n");
		add_steep_bump(brx,bry,view);
		check_view(active_views,current_view);
	} else {
//printf ("view splitted\n");
		view_t *shallower_view= & views[offset];
		int view_index=current_view - (view_t **)TCOD_list_begin(active_views);
		view_t **shallower_view_it;
		view_t **steeper_view_it;
		*shallower_view=**current_view;
		// slow !
		shallower_view_it = (view_t **)TCOD_list_insert_before(active_views,shallower_view,view_index);
		steeper_view_it=shallower_view_it+1;
		current_view=shallower_view_it;
		add_steep_bump(brx,bry,shallower_view);
		if (!check_view(active_views,shallower_view_it)) steeper_view_it--;
		add_shallow_bump(tlx,tly,*steeper_view_it);
		check_view(active_views,steeper_view_it);
		if ( view_index > TCOD_list_size(active_views)) current_view=(view_t **)TCOD_list_end(active_views);
	}
}

static void check_quadrant(map_t *m,int startX,int startY,int dx, int dy, int extentX,int extentY, bool light_walls) {
	TCOD_list_t active_views=TCOD_list_new();
	line_t shallow_line={0,1,extentX,0};
	line_t steep_line={1,0,0,extentY};
	int maxI=extentX+extentY,i=1;
	view_t *view= &views[startX+startY*m->width];

	view->shallow_line=shallow_line;
	view->steep_line=steep_line;
	view->shallow_bump=NULL;
	view->steep_bump=NULL;
	TCOD_list_push(active_views,view);
	current_view=(view_t **)TCOD_list_begin(active_views);
	while ( i  != maxI+1 && ! TCOD_list_is_empty(active_views) ) {
		int startJ=MAX(i-extentX,0);
		int maxJ=MIN(i,extentY);
		int j=startJ;
		while ( j != maxJ+1 && ! TCOD_list_is_empty(active_views) && current_view != (view_t **)TCOD_list_end(active_views) ) {
			int x=i - j;
			int y=j;
//printf ("==>%d %d\n",x,y);
			visit_coords(m,startX,startY,x,y,dx,dy,active_views, light_walls);
			j++;
		}
		i++;
		current_view=(view_t **)TCOD_list_begin(active_views);
	}
}

void TCOD_map_compute_fov_permissive(TCOD_map_t map, int player_x, int player_y, int max_radius, bool light_walls) {
	int c,minx,maxx,miny,maxy;
	map_t *m = (map_t *)map;
//if (p) printf ("perm\n");
	// clean the map
	for (c=m->nbcells-1; c >= 0; c--) {
		m->cells[c].fov=0;
	}
	m->cells[player_x+player_y*m->width].fov=1;
	// preallocate views and bumps
	views=(view_t *)calloc(sizeof(view_t),m->width*m->height);
	bumps=(viewbump_t *)calloc(sizeof(viewbump_t),m->width*m->height);
	// set the fov range
	if ( max_radius > 0 ) {
		minx=MIN(player_x,max_radius);
		maxx=MIN(m->width-player_x-1,max_radius);
		miny=MIN(player_y,max_radius);
		maxy=MIN(m->height-player_y-1,max_radius);
	} else {
		minx=player_x;
		maxx=m->width-player_x-1;
		miny=player_y;
		maxy=m->height-player_y-1;
	}
	// calculate fov. precise permissive field of view
	bumpidx=0;
	check_quadrant(m,player_x,player_y,1,1,maxx,maxy, light_walls);
	bumpidx=0;
	check_quadrant(m,player_x,player_y,1,-1,maxx,miny, light_walls);
	bumpidx=0;
	check_quadrant(m,player_x,player_y,-1,-1,minx,miny, light_walls);
	bumpidx=0;
	check_quadrant(m,player_x,player_y,-1,1,minx,maxy, light_walls);
	free(bumps);
	free(views);
p=false;
}

