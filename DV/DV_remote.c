//========================================================================================
// DV_remote.c
//
// the following is used to compile a version of DV without the GUI interface, and
// adds a few stub routines to allow for compilation without the forms or X libraries
//========================================================================================

#include "DV_gui_ext.h"
#include "DVault.h"

const int no_GUI=1;
int auto_refresh=0;

void *start_gui(void *arg)
{
   printf("error ... start_gui called from remote server\n");
}

void create_browser_list()
{
   printf("error ... create_browser_list called from remote server\n");
}

