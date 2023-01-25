#ifndef _DV_GUI_H
#define _DV_GUI_H

//=============================================================================
// DV_gui.h
//=============================================================================

#include "forms.h"
#include "DV_forms.h"

extern int auto_refresh;

void *start_gui(void *arg);
void create_browser_list();
extern FD_DV_gl_canvas_window *fd_DV_gl_canvas_window;

#endif // _DV_GUI_H 
