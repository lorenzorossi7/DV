/* Form definition file generated with fdesign. */

#include <GL/gl.h>
#include <GL/glx.h>
            
#include "forms.h"
#include "glcanvas.h"
#include <stdlib.h>
#include "DV_forms.h"

static FL_PUP_ENTRY fdmenu_file_menu_0[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Read SDF from file",	0,	"",	 FL_PUP_NONE},
    { "Read Perimeter SDF from file",	0,	"",	 FL_PUP_NONE},
    { "Write selected SDF(s) to file",	0,	"",	 FL_PUP_NONE},
    { "Load options",	0,	"",	 FL_PUP_NONE},
    { "Save options",	0,	"",	 FL_PUP_NONE},
    { "About",	0,	"",	 FL_PUP_NONE},
    { "Quit",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_Debug_1[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Dump All",	0,	"",	 FL_PUP_NONE},
    { "Memory Usage",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_edit_menu_2[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Open All Registers",	0,	"",	 FL_PUP_NONE},
    { "   Times",	0,	"",	 FL_PUP_NONE},
    { "   Levels",	0,	"",	 FL_PUP_NONE},
    { "Close All Registers",	0,	"",	 FL_PUP_NONE},
    { "   Times",	0,	"",	 FL_PUP_NONE},
    { "   Levels",	0,	"",	 FL_PUP_NONE},
    { "Select All Registers",	0,	"",	 FL_PUP_NONE},
    { "   Times",	0,	"",	 FL_PUP_NONE},
    { "   Levels",	0,	"",	 FL_PUP_NONE},
    { "   Grids",	0,	"",	 FL_PUP_NONE},
    { "Deselect All Registers",	0,	"",	 FL_PUP_NONE},
    { "   Times",	0,	"",	 FL_PUP_NONE},
    { "   Levels",	0,	"",	 FL_PUP_NONE},
    { "   Grids",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_view_menu_3[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Refresh",	0,	"",	 FL_PUP_NONE},
    { "Auto-Refresh Mode",	0,	"",	 FL_PUP_NONE},
    { "Raise All Windows",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_delete_menu_4[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Selected and Open",	0,	"",	 FL_PUP_NONE},
    { "Selected",	0,	"",	 FL_PUP_NONE},
    { "All",	0,	"",	 FL_PUP_NONE},
    { "Last Time-Step of Selected Registers",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_Merge_5[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Selected Registers",	0,	"",	 FL_PUP_NONE},
    { "All Registers ",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_mstest_6[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "User functions",	0,	"",	 FL_PUP_NONE},
    { "Load user functions",	0,	"",	 FL_PUP_NONE},
    { "Save user functions",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_main_window *create_form_DV_main_window(void)
{
  FL_OBJECT *obj;
  FD_DV_main_window *fdui = (FD_DV_main_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_main_window = fl_bgn_form(FL_NO_BOX, 630, 470);
  obj = fl_add_box(FL_UP_BOX,0,0,630,470,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,430,230,180,230,"Visualization");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,460,270,140,80,"");
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM_RIGHT);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,430,130,180,80,"");
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_UP_FRAME,10,10,610,30,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->file_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,10,10,60,30,"File");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,file_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_file_menu_0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,20,70,400,390,"Data-Vault contents");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->dv_browser = obj = fl_add_browser(FL_MULTI_BROWSER,30,90,380,360,"");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,dv_browser_cb,0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,430,70,180,60,"Instructions");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_menu(FL_PULLDOWN_MENU,540,10,70,30,"Debug");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,debug_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_Debug_1);
  fdui->edit_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,70,10,60,30,"Edit");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,edit_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_edit_menu_2);
  fdui->view_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,130,10,60,30,"View");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,view_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_view_menu_3);
  fdui->delete_menu = obj = fl_add_menu(FL_PUSH_MENU,450,90,70,30,"Delete");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,delete_cb,0);
    fl_set_menu_entries(obj, fdmenu_delete_menu_4);
  fdui->diff = obj = fl_add_button(FL_NORMAL_BUTTON,450,140,140,30,"Differentiate");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,diff_cb,0);
  fdui->functions = obj = fl_add_button(FL_NORMAL_BUTTON,450,170,140,30,"Functions");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,functions_cb,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,520,300,70,30,"options");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,options_cb,0);
  fdui->data_browser = obj = fl_add_button(FL_NORMAL_BUTTON,450,380,140,30,"Data Browser");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,data_browser_cb,0);
  fdui->send_to_local = obj = fl_add_button(FL_NORMAL_BUTTON,450,250,140,30,"Send to local view");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,send_to_local_cb,0);
  obj = fl_add_menu(FL_PUSH_MENU,520,90,70,30,"Merge");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,merge_cb,0);
    fl_set_menu_entries(obj, fdmenu_Merge_5);
  fdui->hvs_window = obj = fl_add_button(FL_NORMAL_BUTTON,450,420,140,30,"3D Slicer");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,hvs_cb,0);
  fdui->up = obj = fl_add_scrollbutton(FL_TOUCH_BUTTON,470,300,20,30,"8");
    fl_set_object_shortcut(obj,"<&A><&D>",1);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,browser_up_cb,0);
  fdui->down = obj = fl_add_scrollbutton(FL_TOUCH_BUTTON,490,300,20,30,"2");
    fl_set_object_shortcut(obj,"<&B><&C>",1);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,browser_down_cb,0);
  fdui->mstest = obj = fl_add_menu(FL_PULLDOWN_MENU,190,10,60,30,"Tools");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,mstest_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_mstest_6);
  fl_end_form();

  fdui->DV_main_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdchoice_diff_stencil_7[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "3 point ",	0,	"",	 FL_PUP_NONE},
    { "2 point CN",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_diff_window *create_form_DV_diff_window(void)
{
  FL_OBJECT *obj;
  FD_DV_diff_window *fdui = (FD_DV_diff_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_diff_window = fl_bgn_form(FL_NO_BOX, 560, 300);
  obj = fl_add_box(FL_UP_BOX,0,0,560,300,"");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,540,270,"Differentiate");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->diff_browser = obj = fl_add_browser(FL_HOLD_BROWSER,20,30,250,170,"");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,diff_browser_cb,0);
  fdui->diff_wrt = obj = fl_add_choice(FL_NORMAL_CHOICE2,50,210,80,30,"w.r.t");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,diff_wrt_cb,0);
  fdui->diff_mask = obj = fl_add_browser(FL_HOLD_BROWSER,320,70,220,130,"mask");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthEast, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,diff_mask_cb,0);
  fdui->diff_mask_val = obj = fl_add_input(FL_NORMAL_INPUT,390,210,150,30,"mask value");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->diff_new_reg = obj = fl_add_input(FL_NORMAL_INPUT,260,250,210,30,"new\nname");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->diff_go = obj = fl_add_button(FL_NORMAL_BUTTON,480,250,60,30,"Go !");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,diff_go_cb,0);
  fdui->diff_overwrite = obj = fl_add_checkbutton(FL_PUSH_BUTTON,20,250,120,30,"over-write existing \n registers?");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->diff_stencil = obj = fl_add_choice(FL_NORMAL_CHOICE2,180,210,90,30,"spatial\nstencil");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
    fl_set_choice_entries(obj, fdchoice_diff_stencil_7);
  fdui->diff_filter = obj = fl_add_input(FL_NORMAL_INPUT,320,30,220,30,"filter");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fl_end_form();

  fdui->DV_diff_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdchoice_filter_hist_8[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "history",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_arg_hist_9[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "history",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_func_window *create_form_DV_func_window(void)
{
  FL_OBJECT *obj;
  FD_DV_func_window *fdui = (FD_DV_func_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_func_window = fl_bgn_form(FL_NO_BOX, 660, 550);
  obj = fl_add_box(FL_UP_BOX,0,0,660,550,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,640,530,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->func_browser = obj = fl_add_browser(FL_SELECT_BROWSER,230,40,410,190,"Register");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,func_browser_cb,0);
  fdui->func_reg_a = obj = fl_add_input(FL_NORMAL_INPUT,70,260,570,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,func_reg_a_cb,0);
  fdui->func_reg_b = obj = fl_add_input(FL_NORMAL_INPUT,70,290,570,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,func_reg_b_cb,0);
  fdui->func_mask = obj = fl_add_input(FL_NORMAL_INPUT,70,320,330,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,null_cb,0);
  fdui->func_overwrite = obj = fl_add_checkbutton(FL_PUSH_BUTTON,440,450,120,30,"over-write existing \n registers?");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,null_cb,0);
  fdui->func_go = obj = fl_add_button(FL_NORMAL_BUTTON,570,490,60,30,"Go !");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,func_go_cb,0);
  fdui->func_args = obj = fl_add_input(FL_NORMAL_INPUT,400,360,170,30,"Arguments");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->func_new_reg = obj = fl_add_input(FL_NORMAL_INPUT,70,400,570,30,"New \nname ");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,null_cb,0);
  fdui->func_info = obj = fl_add_text(FL_NORMAL_TEXT,20,450,410,80,"Info:");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
  fdui->func_mask_val = obj = fl_add_input(FL_NORMAL_INPUT,500,320,140,30,"Mask value");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  obj = fl_add_text(FL_NORMAL_TEXT,30,260,20,30,"A");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,30,290,20,30,"B");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,320,50,30,"Mask");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->func = obj = fl_add_browser(FL_HOLD_BROWSER,20,40,200,190,"Function");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthWest);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,func_cb,0);
  obj = fl_add_text(FL_NORMAL_TEXT,250,230,390,30,"select register A/B/Mask with left/middle/right mouse button");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->apply_filter = obj = fl_add_checkbutton(FL_PUSH_BUTTON,10,360,30,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj,null_cb,0);
  fdui->filter_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,250,360,70,30,"");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,func_filter_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_filter_hist_8);
  fdui->arg_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,570,360,70,30,"");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,func_arg_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_arg_hist_9);
  fdui->givec = obj = fl_add_input(FL_NORMAL_INPUT,80,360,170,30,"Filter ");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,null_cb,0);
  fl_end_form();

  fdui->DV_func_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdmenu_gl_options_menu_10[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Point",	0,	"",	 FL_PUP_NONE},
    { "Wire frame",	0,	"",	 FL_PUP_NONE},
    { "Line x",	0,	"",	 FL_PUP_NONE},
    { "Line y",	0,	"",	 FL_PUP_NONE},
    { "Line x+y",	0,	"",	 FL_PUP_NONE},
    { "Solid",	0,	"",	 FL_PUP_NONE},
    { "Smooth solid",	0,	"",	 FL_PUP_NONE},
    { "Particles",	0,	"",	 FL_PUP_NONE},
    { "--------------------------",	0,	"",	 FL_PUP_NONE},
    { "Draw bounding-boxes",	0,	"",	 FL_PUP_NONE},
    { "--------------------------",	0,	"",	 FL_PUP_NONE},
    { "Screen Capture",	0,	"",	 FL_PUP_NONE},
    { "Text, Colors, Etc. ",	0,	"",	 FL_PUP_NONE},
    { "--------------------------",	0,	"",	 FL_PUP_NONE},
    { "Use Key Frames",	0,	"",	 FL_PUP_NONE},
    { "   Set",	0,	"",	 FL_PUP_NONE},
    { "   Clear",	0,	"",	 FL_PUP_NONE},
    { "   Clear All",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_gl_zoom_hist_11[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "zoom",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_gl_theta_hist_12[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "theta",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_gl_phi_hist_13[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "phi",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_gl_canvas_window *create_form_DV_gl_canvas_window(void)
{
  FL_OBJECT *obj;
  FD_DV_gl_canvas_window *fdui = (FD_DV_gl_canvas_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_gl_canvas_window = fl_bgn_form(FL_NO_BOX, 960, 730);
  obj = fl_add_box(FL_UP_BOX,0,0,960,730,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,940,710,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,590,10,360,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->key_frame_ind = obj = fl_add_text(FL_NORMAL_TEXT,600,20,30,20,"*");
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,100,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,110,10,480,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->gl_canvas = obj = fl_add_glcanvas(FL_NORMAL_CANVAS,20,70,920,640,"");
    fl_set_object_color(obj,FL_BLACK,FL_YELLOW);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->home_button = obj = fl_add_button(FL_NORMAL_BUTTON,130,20,50,30,"Home:");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_home_cb,0);
  fdui->gl_zoom = obj = fl_add_input(FL_NORMAL_INPUT,250,20,60,30,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->gl_theta = obj = fl_add_input(FL_NORMAL_INPUT,370,20,60,30,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->gl_phi = obj = fl_add_input(FL_NORMAL_INPUT,490,20,50,30,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->gl_options_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,20,20,80,30,"Options");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_options_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_gl_options_menu_10);
  fdui->gl_zoom_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,190,20,60,30,"");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_zoom_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_gl_zoom_hist_11);
  fdui->gl_theta_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,310,20,60,30,"");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_theta_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_gl_theta_hist_12);
  fdui->gl_phi_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,430,20,60,30,"");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_phi_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_gl_phi_hist_13);
  obj = fl_add_button(FL_NORMAL_BUTTON,550,20,30,30,"1D");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_home_1d_cb,0);
  fl_end_form();

  fdui->DV_gl_canvas_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdchoice_coarsen_step_14[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "1",	0,	"",	 FL_PUP_NONE},
    { "2",	0,	"",	 FL_PUP_NONE},
    { "3",	0,	"",	 FL_PUP_NONE},
    { "4",	0,	"",	 FL_PUP_NONE},
    { "5",	0,	"",	 FL_PUP_NONE},
    { "6",	0,	"",	 FL_PUP_NONE},
    { "7",	0,	"",	 FL_PUP_NONE},
    { "8",	0,	"",	 FL_PUP_NONE},
    { "9",	0,	"",	 FL_PUP_NONE},
    { "10",	0,	"",	 FL_PUP_NONE},
    { "11",	0,	"",	 FL_PUP_NONE},
    { "12",	0,	"",	 FL_PUP_NONE},
    { "13",	0,	"",	 FL_PUP_NONE},
    { "14",	0,	"",	 FL_PUP_NONE},
    { "15",	0,	"",	 FL_PUP_NONE},
    { "16",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_scale_hist_15[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "history",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_cmap_hist_16[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "history",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_cmap_type_17[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Linear",	0,	"",	 FL_PUP_NONE},
    { "Log +-",	0,	"",	 FL_PUP_NONE},
    { "Log +",	0,	"",	 FL_PUP_NONE},
    { "Log -",	0,	"",	 FL_PUP_NONE},
    { "B&W contour",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_sync_t_18[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "sync by time index",	0,	"",	 FL_PUP_NONE},
    { "sync by time value",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_concat_grids_19[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "no concatenation",	0,	"",	 FL_PUP_NONE},
    { "concat. same time",	0,	"",	 FL_PUP_NONE},
    { "concatenate all",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_givec_hist_20[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "history",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_which_canvas_21[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Local view",	0,	"",	 FL_PUP_NONE},
    { "HVS",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_cmap_scheme_22[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Standard",	0,	"",	 FL_PUP_NONE},
    { "B&W",	0,	"",	 FL_PUP_NONE},
    { "W&B",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_sl_opts_window *create_form_DV_sl_opts_window(void)
{
  FL_OBJECT *obj;
  FD_DV_sl_opts_window *fdui = (FD_DV_sl_opts_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_sl_opts_window = fl_bgn_form(FL_NO_BOX, 530, 290);
  obj = fl_add_box(FL_UP_BOX,0,0,530,290,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,510,260,"Grid conversion options for                                  ");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->monotone = obj = fl_add_checkbutton(FL_PUSH_BUTTON,80,240,30,30,"monotone\nshading");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_monotone_cb,0);
  fdui->scale = obj = fl_add_input(FL_NORMAL_INPUT,60,40,100,30,"scale");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->cmap_range_minz = obj = fl_add_input(FL_NORMAL_INPUT,60,80,80,30,"color\nmap");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->cmap_range_maxz = obj = fl_add_input(FL_NORMAL_INPUT,140,80,80,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->amr = obj = fl_add_checkbutton(FL_PUSH_BUTTON,310,240,30,30,"AMR");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_amr_cb,0);
  fdui->use_mask = obj = fl_add_checkbutton(FL_PUSH_BUTTON,10,120,30,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_use_mask_cb,0);
  fdui->mask = obj = fl_add_input(FL_NORMAL_INPUT,80,120,310,30,"mask");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->mask_val = obj = fl_add_input(FL_NORMAL_INPUT,440,120,70,30,"value");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->use_givec = obj = fl_add_checkbutton(FL_PUSH_BUTTON,10,160,30,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_use_givec_cb,0);
  fdui->givec = obj = fl_add_input(FL_NORMAL_INPUT,80,160,360,30,"filter");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->coarsen_step = obj = fl_add_choice(FL_NORMAL_CHOICE2,80,200,50,30,"coarsen");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_coarsen_cb,0);
    fl_set_choice_entries(obj, fdchoice_coarsen_step_14);
  fdui->scale_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,160,40,70,30,"");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_scale_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_scale_hist_15);
  fdui->cmap_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,220,80,70,30,"");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_cmap_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_cmap_hist_16);
  fdui->cmap_type = obj = fl_add_choice(FL_NORMAL_CHOICE2,360,80,70,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
    fl_set_choice_entries(obj, fdchoice_cmap_type_17);
    fl_set_choice(obj,1);
  fdui->zmin_clip = obj = fl_add_input(FL_NORMAL_INPUT,330,40,70,30,"z-clip min");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->zmax_clip = obj = fl_add_input(FL_NORMAL_INPUT,440,40,70,30,"max");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->sync_t = obj = fl_add_choice(FL_NORMAL_CHOICE2,160,200,160,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_sync_cb,0);
    fl_set_choice_entries(obj, fdchoice_sync_t_18);
  fdui->concat_grids = obj = fl_add_choice(FL_NORMAL_CHOICE2,350,200,160,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_concat_grids_cb,0);
    fl_set_choice_entries(obj, fdchoice_concat_grids_19);
    fl_set_choice(obj,2);
  fdui->givec_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,440,160,70,30,"");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_givec_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_givec_hist_20);
  fdui->ignore_ccoords = obj = fl_add_checkbutton(FL_PUSH_BUTTON,400,240,30,30,"ignore\nc.c.");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_ignore_cb,0);
  fdui->fg_color = obj = fl_add_button(FL_NORMAL_BUTTON,110,240,40,30,"color");
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_fg_color_cb,0);
  fdui->levelcolor = obj = fl_add_checkbutton(FL_PUSH_BUTTON,230,240,30,30,"level(dx)\ncoloring");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_levelcolor_cb,0);
  fdui->bbox_only = obj = fl_add_checkbutton(FL_PUSH_BUTTON,480,240,30,30,"3D b.b.\nonly");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_3D_bbox_only_cb,0);
  fdui->which_canvas = obj = fl_add_choice(FL_NORMAL_CHOICE2,220,10,90,20,"");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_which_canvas_cb,0);
    fl_set_choice_entries(obj, fdchoice_which_canvas_21);
    fl_set_choice(obj,1);
  obj = fl_add_text(FL_NORMAL_TEXT,310,10,20,20,"(");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  obj = fl_add_text(FL_NORMAL_TEXT,320,10,40,20,"italic");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  obj = fl_add_text(FL_NORMAL_TEXT,360,10,140,20,"fields apply to both)");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->color_clip = obj = fl_add_checkbutton(FL_PUSH_BUTTON,330,80,30,30,"3D\nclip");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_color_clip_cb,0);
  fdui->cmap_scheme = obj = fl_add_choice(FL_NORMAL_CHOICE2,440,80,70,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_cmap_scheme_cb,0);
    fl_set_choice_entries(obj, fdchoice_cmap_scheme_22);
  fl_end_form();

  fdui->DV_sl_opts_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_capture_window *create_form_DV_capture_window(void)
{
  FL_OBJECT *obj;
  FD_DV_capture_window *fdui = (FD_DV_capture_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_capture_window = fl_bgn_form(FL_NO_BOX, 360, 170);
  obj = fl_add_box(FL_UP_BOX,0,0,360,170,"");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,340,140,"Screen Capture");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->dir = obj = fl_add_input(FL_NORMAL_INPUT,110,40,170,30,"directory");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->browse = obj = fl_add_button(FL_NORMAL_BUTTON,280,40,60,30,"browse");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,capture_browse_cb,0);
  fdui->format = obj = fl_add_input(FL_NORMAL_INPUT,120,80,110,30,"file name\nformat string");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->first_index = obj = fl_add_input(FL_NORMAL_INPUT,280,80,60,30,"first\nindex");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->start_stop = obj = fl_add_button(FL_NORMAL_BUTTON,220,120,120,30,"Start capture");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,capture_cb,0);
  fdui->jpeg_qf = obj = fl_add_input(FL_NORMAL_INPUT,120,120,90,30,"jpeg quality\nfactor(0-100)");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fl_end_form();

  fdui->DV_capture_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdchoice_border_width_23[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "1",	0,	"",	 FL_PUP_NONE},
    { "2",	0,	"",	 FL_PUP_NONE},
    { "3",	0,	"",	 FL_PUP_NONE},
    { "4",	0,	"",	 FL_PUP_NONE},
    { "5",	0,	"",	 FL_PUP_NONE},
    { "6",	0,	"",	 FL_PUP_NONE},
    { "7",	0,	"",	 FL_PUP_NONE},
    { "8",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_bbox_width_24[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "1",	0,	"",	 FL_PUP_NONE},
    { "2",	0,	"",	 FL_PUP_NONE},
    { "3",	0,	"",	 FL_PUP_NONE},
    { "4",	0,	"",	 FL_PUP_NONE},
    { "5",	0,	"",	 FL_PUP_NONE},
    { "6",	0,	"",	 FL_PUP_NONE},
    { "7",	0,	"",	 FL_PUP_NONE},
    { "8",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_point_size_25[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "1",	0,	"",	 FL_PUP_NONE},
    { "2",	0,	"",	 FL_PUP_NONE},
    { "3",	0,	"",	 FL_PUP_NONE},
    { "4",	0,	"",	 FL_PUP_NONE},
    { "5",	0,	"",	 FL_PUP_NONE},
    { "6",	0,	"",	 FL_PUP_NONE},
    { "7",	0,	"",	 FL_PUP_NONE},
    { "8",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_line_width_26[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "1",	0,	"",	 FL_PUP_NONE},
    { "2",	0,	"",	 FL_PUP_NONE},
    { "3",	0,	"",	 FL_PUP_NONE},
    { "4",	0,	"",	 FL_PUP_NONE},
    { "5",	0,	"",	 FL_PUP_NONE},
    { "6",	0,	"",	 FL_PUP_NONE},
    { "7",	0,	"",	 FL_PUP_NONE},
    { "8",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_which_canvas_27[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Local view",	0,	"",	 FL_PUP_NONE},
    { "HVS",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_annotation_window *create_form_DV_annotation_window(void)
{
  FL_OBJECT *obj;
  FD_DV_annotation_window *fdui = (FD_DV_annotation_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_annotation_window = fl_bgn_form(FL_NO_BOX, 500, 320);
  obj = fl_add_box(FL_UP_BOX,0,0,500,320,"");
  obj = fl_add_text(FL_NORMAL_TEXT,380,170,110,40,"click on canvas\nto place");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,480,190,"Annotation for        ");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  obj = fl_add_text(FL_NORMAL_TEXT,380,220,110,40,"left click positions\nright click scales");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,210,480,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,260,480,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  fdui->edit = obj = fl_add_choice(FL_NORMAL_CHOICE2,70,40,410,30,"edit:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,gl_ann_edit_cb,0);
    fl_set_choice_align(obj, FL_ALIGN_LEFT);
  fdui->format_str = obj = fl_add_input(FL_NORMAL_INPUT,70,80,410,30,"format\nstring");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,gl_ann_format_str_cb,0);
  fdui->var_options = obj = fl_add_text(FL_NORMAL_TEXT,20,120,460,50,"Allowed variable names in format string (syntax $var#.#[f|e]):\nt,  [x|y|z|c|clip_z]min,  [x|y|z|c|clip_z]max,  zscale,  shape[x|y]");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->font = obj = fl_add_choice(FL_NORMAL_CHOICE2,50,170,170,30,"font");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_font_cb,0);
  fdui->color = obj = fl_add_button(FL_NORMAL_BUTTON,220,170,60,30,"color");
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_color_cb,0);
  fdui->pos_text = obj = fl_add_checkbutton(FL_PUSH_BUTTON,350,170,30,30,"position");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_pos_text_cb,0);
  fdui->disp_cmap = obj = fl_add_checkbutton(FL_PUSH_BUTTON,90,220,30,30,"display\ncolormap");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_disp_cmap_cb,0);
  fdui->pos_cmap = obj = fl_add_checkbutton(FL_PUSH_BUTTON,350,220,30,30,"position");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_pos_cmap_cb,0);
  fdui->border_color = obj = fl_add_button(FL_NORMAL_BUTTON,220,220,60,30,"color");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_border_color_cb,0);
  fdui->border_width = obj = fl_add_choice(FL_NORMAL_CHOICE2,180,220,40,30,"border\nwidth");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_border_width_cb,0);
    fl_set_choice_entries(obj, fdchoice_border_width_23);
  fdui->bbox_width = obj = fl_add_choice(FL_NORMAL_CHOICE2,60,270,40,30,"b.box\nwidth");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_bbox_width_cb,0);
    fl_set_choice_entries(obj, fdchoice_bbox_width_24);
    fl_set_choice(obj,1);
  fdui->bb_color = obj = fl_add_button(FL_NORMAL_BUTTON,100,270,40,30,"color");
    fl_set_object_lcolor(obj,FL_GREEN);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_bbox_color_cb,0);
  fdui->point_size = obj = fl_add_choice(FL_NORMAL_CHOICE2,190,270,40,30,"point\nsize");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_1d_pt_size_cb,0);
    fl_set_choice_entries(obj, fdchoice_point_size_25);
    fl_set_choice(obj,4);
  fdui->line_width = obj = fl_add_choice(FL_NORMAL_CHOICE2,280,270,40,30,"line\nwidth");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_1d_line_width_cb,0);
    fl_set_choice_entries(obj, fdchoice_line_width_26);
    fl_set_choice(obj,2);
  fdui->anti_alias = obj = fl_add_checkbutton(FL_PUSH_BUTTON,370,270,30,30,"anti-\nalias");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_anti_alias_cb,0);
  fdui->bg_color = obj = fl_add_button(FL_NORMAL_BUTTON,410,270,70,30,"BG color");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_bg_color_cb,0);
  fdui->which_canvas = obj = fl_add_choice(FL_NORMAL_CHOICE2,140,10,100,20,"");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_which_canvas_cb,0);
    fl_set_choice_entries(obj, fdchoice_which_canvas_27);
    fl_set_choice(obj,1);
  fdui->autoscale = obj = fl_add_checkbutton(FL_PUSH_BUTTON,450,130,30,30,"auto-\nscale");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fl_end_form();

  fdui->DV_annotation_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_dbrowser_window *create_form_DV_dbrowser_window(void)
{
  FL_OBJECT *obj;
  FD_DV_dbrowser_window *fdui = (FD_DV_dbrowser_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_dbrowser_window = fl_bgn_form(FL_NO_BOX, 750, 480);
  obj = fl_add_box(FL_UP_BOX,0,0,750,480,"");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,570,20,170,80,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,560,80,"Data browser");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->info = obj = fl_add_text(FL_NORMAL_TEXT,20,30,540,60,"text\ntext\ntext");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,100,730,370,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->data = obj = fl_add_text(FL_NORMAL_TEXT,250,180,480,280,"text");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_RIGHT_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->h_slider = obj = fl_add_slider(FL_HOR_BROWSER_SLIDER,250,110,480,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,db_h_slider_cb,0);
  fdui->v_slider = obj = fl_add_slider(FL_VERT_BROWSER_SLIDER,20,180,40,280,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthWest);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,db_v_slider_cb,0);
  fdui->i_info = obj = fl_add_text(FL_NORMAL_TEXT,250,140,480,40,"i index, coord info");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->j_info = obj = fl_add_text(FL_NORMAL_TEXT,60,180,190,280,"j index, coord info");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthWest);
    fl_set_object_resize(obj, FL_RESIZE_Y);
  fdui->data_fs = obj = fl_add_input(FL_NORMAL_INPUT,140,110,100,30,"data print string");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,db_data_fs_cb,0);
  fdui->coord_fs = obj = fl_add_input(FL_NORMAL_INPUT,140,140,100,30,"coord print string");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,db_coord_fs_cb,0);
  fdui->screen_dump = obj = fl_add_checkbutton(FL_RADIO_BUTTON,570,30,30,20,"screen\ndump");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,db_screen_dump_cb,0);
  obj = fl_add_checkbutton(FL_RADIO_BUTTON,570,70,30,20,"reverse\ny coord");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,db_reverse_y_cb,0);
  fdui->sm_save = obj = fl_add_button(FL_NORMAL_BUTTON,660,30,70,60,"save as\nsm image\nfile");
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,db_sm_save_cb,0);
  fl_end_form();

  fdui->DV_dbrowser_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdmenu_hvs_options_menu_28[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Point",	0,	"",	 FL_PUP_NONE},
    { "Volumetric",	0,	"",	 FL_PUP_NONE},
    { "None",	0,	"",	 FL_PUP_NONE},
    { "--------------------------",	0,	"",	 FL_PUP_NONE},
    { "Draw bounding-boxes",	0,	"",	 FL_PUP_NONE},
    { "Draw slice box",	0,	"",	 FL_PUP_NONE},
    { "--------------------------",	0,	"",	 FL_PUP_NONE},
    { "Screen Capture",	0,	"",	 FL_PUP_NONE},
    { "Text, Colors, Etc. ",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_hvs_window *create_form_DV_hvs_window(void)
{
  FL_OBJECT *obj;
  FD_DV_hvs_window *fdui = (FD_DV_hvs_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_hvs_window = fl_bgn_form(FL_NO_BOX, 530, 470);
  obj = fl_add_box(FL_UP_BOX,0,0,530,470,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,510,450,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,110,10,70,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,180,10,340,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,100,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->gl_canvas = obj = fl_add_glcanvas(FL_NORMAL_CANVAS,20,70,460,380,"");
    fl_set_object_boxtype(obj,FL_RSHADOW_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_BLACK);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->hvs_options_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,20,20,80,30,"Options");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,hvs_options_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_hvs_options_menu_28);
  fdui->home_button = obj = fl_add_button(FL_NORMAL_BUTTON,120,20,50,30,"Home");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,hvs_home_cb,0);
  obj = fl_add_text(FL_NORMAL_TEXT,190,20,40,30,"Slice:");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->x_input = obj = fl_add_input(FL_NORMAL_INPUT,250,20,80,30,"X");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,hvs_x_slice_cb,0);
  fdui->y_input = obj = fl_add_input(FL_NORMAL_INPUT,350,20,70,30,"Y");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,hvs_y_slice_cb,0);
  fdui->z_input = obj = fl_add_input(FL_NORMAL_INPUT,440,20,70,30,"Z");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,hvs_z_slice_cb,0);
  fdui->slice_slider = obj = fl_add_thumbwheel(FL_VERT_THUMBWHEEL,490,220,20,200,"");
    fl_set_object_gravity(obj, FL_NorthEast, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,hvs_slice_slider_cb,0);
  fdui->inc_slice = obj = fl_add_scrollbutton(FL_TOUCH_BUTTON,490,190,20,30,"8");
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,hvs_inc_slice_cb,0);
  fdui->dec_slice = obj = fl_add_scrollbutton(FL_TOUCH_BUTTON,490,420,20,30,"2");
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,hvs_dec_slice_cb,0);
  fdui->x_button = obj = fl_add_button(FL_RADIO_BUTTON,490,70,20,30,"X");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,hvs_x_slice_cb,0);
  fdui->y_button = obj = fl_add_button(FL_RADIO_BUTTON,490,110,20,30,"Y");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,hvs_y_slice_cb,0);
  fdui->z_button = obj = fl_add_button(FL_RADIO_BUTTON,490,150,20,30,"Z");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,hvs_z_slice_cb,0);
  fl_end_form();

  fdui->DV_hvs_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_mstest_item1_window *create_form_DV_mstest_item1_window(void)
{
  FL_OBJECT *obj;
  FD_DV_mstest_item1_window *fdui = (FD_DV_mstest_item1_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_mstest_item1_window = fl_bgn_form(FL_NO_BOX, 710, 620);
  obj = fl_add_box(FL_UP_BOX,0,0,710,620,"");
  fdui->mstest_item1_browser = obj = fl_add_browser(FL_MULTI_BROWSER,330,30,220,190,"Register");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_browser_cb,0);
  fdui->mstest_item1_func = obj = fl_add_browser(FL_HOLD_BROWSER,20,30,200,190,"Function");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,mstest_item1_func_cb,0);
  fdui->mstest_item1_edit = obj = fl_add_button(FL_NORMAL_BUTTON,280,30,40,40,"Edit");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_new_cb,0);
  fdui->mstest_item1_input = obj = fl_add_browser(FL_HOLD_BROWSER,20,330,200,190,"Input");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,mstest_item1_input_cb,0);
  fdui->mstest_item1_delete = obj = fl_add_button(FL_NORMAL_BUTTON,230,79,90,42,"Delete");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_delete_cb,0);
  fdui->mstest_item1_input_up = obj = fl_add_button(FL_NORMAL_BUTTON,230,420,90,41,"Up");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_input_up_cb,0);
  fdui->mstest_item1_input_down = obj = fl_add_button(FL_NORMAL_BUTTON,230,470,90,41,"Down");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_input_down_cb,0);
  fdui->mstest_item1_info = obj = fl_add_text(FL_NORMAL_TEXT,20,230,630,70,"Info:");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
  fdui->mstest_item1_up = obj = fl_add_button(FL_NORMAL_BUTTON,230,129,90,41,"Up");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_up_cb,0);
  fdui->mstest_item1_down = obj = fl_add_button(FL_NORMAL_BUTTON,230,179,90,41,"Down");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_down_cb,0);
  fdui->mstest_item1_new = obj = fl_add_button(FL_NORMAL_BUTTON,230,30,40,40,"New");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_new_cb,1);
  fdui->mstest_item1_input_remove = obj = fl_add_button(FL_NORMAL_BUTTON,230,340,90,60,"Remove");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_input_remove_cb,0);
  fdui->mstest_item1_delmode = obj = fl_add_checkbutton(FL_PUSH_BUTTON,340,350,140,40,"adapt to input");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->mstest_item1_go = obj = fl_add_button(FL_NORMAL_BUTTON,340,420,120,90,"Go !");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_go_cb,0);
  fdui->mstest_item1_add = obj = fl_add_button(FL_NORMAL_BUTTON,560,100,90,42,"Add");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_item1_add_cb,0);
  fdui->mstest_item1_godelmode = obj = fl_add_checkbutton(FL_PUSH_BUTTON,480,460,140,40,"remove  after go");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->mstest_item1_gomode = obj = fl_add_checkbutton(FL_PUSH_BUTTON,480,420,140,40,"all input");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->filter = obj = fl_add_input(FL_NORMAL_INPUT,60,550,160,30,"Filter");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,null_cb,0);
  fl_end_form();

  fdui->DV_mstest_item1_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_mstest_userfdef_window *create_form_DV_mstest_userfdef_window(void)
{
  FL_OBJECT *obj;
  FD_DV_mstest_userfdef_window *fdui = (FD_DV_mstest_userfdef_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_mstest_userfdef_window = fl_bgn_form(FL_NO_BOX, 710, 560);
  obj = fl_add_box(FL_UP_BOX,0,0,710,560,"");
  fdui->mstest_userfdef_browser = obj = fl_add_browser(FL_SELECT_BROWSER,300,40,340,210,"Register");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,mstest_userfdef_browser_cb,0);
  fdui->mstest_userfdef_reg_a = obj = fl_add_input(FL_NORMAL_INPUT,60,290,270,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,mstest_userfdef_AB_etc_cb,0);
  fdui->mstest_userfdef_reg_b = obj = fl_add_input(FL_NORMAL_INPUT,60,320,270,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,mstest_userfdef_AB_etc_cb,1);
  fdui->mstest_userfdef_mask = obj = fl_add_input(FL_NORMAL_INPUT,60,350,270,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,mstest_userfdef_AB_etc_cb,2);
  fdui->mstest_userfdef_done = obj = fl_add_button(FL_NORMAL_BUTTON,420,510,90,30,"Apply");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_userfdef_apply_cb,0);
  fdui->mstest_userfdef_args = obj = fl_add_input(FL_NORMAL_INPUT,90,430,170,30,"Arguments");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,mstest_userfdef_AB_etc_cb,4);
  fdui->mstest_userfdef_new = obj = fl_add_input(FL_NORMAL_INPUT,60,470,270,30,"New \nname ");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,mstest_userfdef_AB_etc_cb,5);
  fdui->mstest_userfdef_mvalue = obj = fl_add_input(FL_NORMAL_INPUT,90,390,140,30,"Mask value");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,mstest_userfdef_AB_etc_cb,03);
  obj = fl_add_text(FL_NORMAL_TEXT,20,290,20,30,"A");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,320,20,30,"B");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,350,50,30,"Mask");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->mstest_userfdef_func = obj = fl_add_browser(FL_SELECT_BROWSER,40,90,200,190,"Function");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthWest);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,mstest_userfdef_func_cb,0);
  obj = fl_add_text(FL_NORMAL_TEXT,250,250,390,30,"select register A/B/Mask with left/middle/right mouse button");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->mstest_userfdef_macro = obj = fl_add_browser(FL_HOLD_BROWSER,350,310,200,190,"Macro code");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_userfdef_macro_cb,0);
  fdui->mstest_userfdef_name = obj = fl_add_input(FL_NORMAL_INPUT,60,20,180,30,"name");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,mstest_userfdef_AB_etc_cb,6);
  fdui->mstest_userfdef_delete = obj = fl_add_button(FL_NORMAL_BUTTON,560,320,90,50,"Delete");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_userfdef_delete_cb,0);
  fdui->mstest_userfdef_up = obj = fl_add_button(FL_NORMAL_BUTTON,560,380,90,50,"Up");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_userfdef_up_cb,0);
  fdui->mstest_userfdef_down = obj = fl_add_button(FL_NORMAL_BUTTON,560,440,90,50,"Down");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,mstest_userfdef_down_cb,0);
  fl_end_form();

  fdui->DV_mstest_userfdef_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_f_select_window *create_form_DV_f_select_window(void)
{
  FL_OBJECT *obj;
  FD_DV_f_select_window *fdui = (FD_DV_f_select_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_f_select_window = fl_bgn_form(FL_NO_BOX, 350, 200);
  obj = fl_add_box(FL_UP_BOX,0,0,350,200,"");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,330,170,"Select color function");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->wf = obj = fl_add_input(FL_NORMAL_INPUT,80,40,100,30,"Function");
    fl_set_object_callback(obj,f_select_cb,0);
  fdui->select = obj = fl_add_button(FL_NORMAL_BUTTON,210,40,120,30,"Select");
    fl_set_object_callback(obj,f_select_cb,0);
  fdui->show_min = obj = fl_add_input(FL_NORMAL_INPUT,80,80,100,30,"Display\nif between");
    fl_set_object_callback(obj,f_select_cb,0);
  fdui->show_max = obj = fl_add_input(FL_NORMAL_INPUT,210,80,120,30,"and");
    fl_set_object_callback(obj,f_select_cb,0);
  fdui->set_cm_min = obj = fl_add_checkbutton(FL_PUSH_BUTTON,140,120,30,30,"Set colormap min?");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,f_select_scmin_cb,0);
  fdui->set_cm_max = obj = fl_add_checkbutton(FL_PUSH_BUTTON,300,120,30,30,"Set colormap max?");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_callback(obj,f_select_scmax_cb,0);
  fdui->cm_min = obj = fl_add_input(FL_NORMAL_INPUT,50,150,120,30,"min");
    fl_set_object_callback(obj,f_select_cm_min_cb,0);
  fdui->cm_max = obj = fl_add_input(FL_NORMAL_INPUT,210,150,120,30,"max");
    fl_set_object_callback(obj,f_select_cm_max_cb,0);
  fl_end_form();

  fdui->DV_f_select_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

