#ifndef _DV_GUI_H
#define _DV_GUI_H

/*=============================================================================*/
/* DV_gui.h                                                                    */
/*=============================================================================*/

#include "forms.h"
#include "DV_forms.h"
#include "send_gl_canvas.h"
#include "DV_gui_ext.h"

extern FD_DV_gl_canvas_window *fd_DV_gl_canvas_window;

#define NUM_GL_CANVASES 2
#define GL_LOCAL_VIEW 0
#define GL_HVS 1

extern int gl_canvas_open[NUM_GL_CANVASES];
extern FL_OBJECT *curr_gl_canvas_obj;
extern int curr_gl_canvas;

void switch_gl_canvas(int canvas);

extern int is_set_cm_max,is_set_cm_min,num_particle_f;
int get_gl_which_particle_f(void);

#endif /* _DV_GUI_H */
