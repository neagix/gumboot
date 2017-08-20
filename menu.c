
#include "menu.h"

const char	top_right_corner = 191,
			bottom_left_corner = 192,
			horiz_line = 196,
			bottom_right_corner = 217,
			top_left_corner = 218,
			vert_line = 179;

void draw_box_at(int x, int y, int w, int h) {
	int base_x = x;
	int max_x = x+w-1, max_y=y+h-1;
	
	if ((h<2) || (w<2))
		return;
		
	// draw top of the box
	gfx_printch_at(x, y, top_left_corner);
	for(x++;x<max_x;x++) {
		gfx_printch_at(x, y, horiz_line);
	}
	gfx_printch_at(x, y, top_right_corner);
	
	// start drawing body
	x = base_x;
	for (y++;y<max_y;y++) {
		gfx_printch_at(x, y, vert_line);
		gfx_printch_at(max_x, y, vert_line);
	}
	
	// draw bottom of the box
	gfx_printch_at(x, y, bottom_left_corner);
	for(x++;x<max_x;x++) {
		gfx_printch_at(x, y, horiz_line);
	}
	gfx_printch_at(x, y, bottom_right_corner);
	
}
