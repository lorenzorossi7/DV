#include "forms.h"
#include "DV_forms.h"

int main(int argc, char *argv[])
{
   FD_DV_main_window *fd_DV_main_window;

   fl_initialize(&argc, argv, 0, 0, 0);
   fd_DV_main_window = create_form_DV_main_window();

   /* fill-in form initialization code */

   /* show the first form */
   fl_show_form(fd_DV_main_window->DV_main_window,FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV_main_window");
   fl_do_forms();
   return 0;
}
