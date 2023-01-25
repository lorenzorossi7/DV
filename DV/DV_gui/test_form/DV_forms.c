/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "DV_forms.h"

static FL_PUP_ENTRY fdmenu_file_menu_0[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Read SDF from file",	0,	"",	 FL_PUP_NONE},
    { "Write SDF to file",	0,	"",	 FL_PUP_NONE},
    { "Load options",	0,	"",	 FL_PUP_NONE},
    { "Save options",	0,	"",	 FL_PUP_NONE},
    { "Quit",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_help_1[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "About",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_Debug_2[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Dump All",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_edit_menu_3[] =
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

static FL_PUP_ENTRY fdmenu_view_menu_4[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Refresh Now",	0,	"",	 FL_PUP_NONE},
    { "Auto-Refresh Mode",	0,	"",	 FL_PUP_RADIO},
    {0}
};

static FL_PUP_ENTRY fdmenu_delete_menu_5[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Selected & Opened",	0,	"",	 FL_PUP_NONE},
    { "Selected",	0,	"",	 FL_PUP_NONE},
    { "All",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_send_to_acceptXYZ_menu_6[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Selected Grids",	0,	"",	 FL_PUP_NONE},
    { "Grids of All Selected",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_SendtoAcceptSDF_7[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Selected Grids",	0,	"",	 FL_PUP_NONE},
    { "Grids of All Selected",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdmenu_Sendtolocalview_8[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Selected Grids",	0,	"",	 FL_PUP_NONE},
    { "Grids of All Selected",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_main_window *create_form_DV_main_window(void)
{
  FL_OBJECT *obj;
  FD_DV_main_window *fdui = (FD_DV_main_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_main_window = fl_bgn_form(FL_NO_BOX, 630, 460);
  obj = fl_add_box(FL_UP_BOX,0,0,630,460,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,430,230,180,210,"Visualization");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,460,320,140,70,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
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
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,file_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_file_menu_0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,20,70,400,370,"Data-Vault contents");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->dv_browser = obj = fl_add_browser(FL_MULTI_BROWSER,30,90,380,340,"");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,dv_browser_cb,0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,430,70,180,60,"Instructions");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->help = obj = fl_add_menu(FL_PULLDOWN_MENU,550,10,60,30,"Help");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,help_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_help_1);
  obj = fl_add_menu(FL_PULLDOWN_MENU,470,10,70,30,"Debug");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,debug_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_Debug_2);
  fdui->edit_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,70,10,60,30,"Edit");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,edit_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_edit_menu_3);
  fdui->view_menu = obj = fl_add_menu(FL_PULLDOWN_MENU,130,10,60,30,"View");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,view_menu_cb,0);
    fl_set_menu_entries(obj, fdmenu_view_menu_4);
  fdui->delete_menu = obj = fl_add_menu(FL_PUSH_MENU,450,90,140,30,"Delete");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,delete_cb,0);
    fl_set_menu_entries(obj, fdmenu_delete_menu_5);
  fdui->send_to_acceptXYZ_menu = obj = fl_add_menu(FL_PUSH_MENU,450,250,140,30,"Send to AcceptXYZ");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,send_to_acceptXYZ_cb,0);
    fl_set_menu_entries(obj, fdmenu_send_to_acceptXYZ_menu_6);
  obj = fl_add_menu(FL_PUSH_MENU,450,280,140,30,"Send to AcceptSDF");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,send_to_acceptSDF_cb,0);
    fl_set_menu_entries(obj, fdmenu_SendtoAcceptSDF_7);
  fdui->diff = obj = fl_add_button(FL_NORMAL_BUTTON,450,140,140,30,"Differentiate");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,diff_cb,0);
  fdui->functions = obj = fl_add_button(FL_NORMAL_BUTTON,450,170,140,30,"Functions");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,functions_cb,0);
  obj = fl_add_menu(FL_PUSH_MENU,450,310,140,30,"Send to local view");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,send_to_local_cb,0);
    fl_set_menu_entries(obj, fdmenu_Sendtolocalview_8);
  obj = fl_add_button(FL_NORMAL_BUTTON,490,350,80,30,"options");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,options_cb,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,450,400,140,30,"Data Browser");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,data_browser_cb,0);
  fl_end_form();

  fdui->DV_main_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_diff_window *create_form_DV_diff_window(void)
{
  FL_OBJECT *obj;
  FD_DV_diff_window *fdui = (FD_DV_diff_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_diff_window = fl_bgn_form(FL_NO_BOX, 820, 570);
  obj = fl_add_box(FL_UP_BOX,0,0,820,570,"");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,800,200,"Differentiate");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->diff_browser = obj = fl_add_browser(FL_HOLD_BROWSER,20,40,220,170,"");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,diff_browser_cb,0);
  fdui->diff_wrt = obj = fl_add_choice(FL_NORMAL_CHOICE2,250,110,100,30,"with respect to");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,diff_wrt_cb,0);
  fdui->diff_mask = obj = fl_add_browser(FL_HOLD_BROWSER,360,50,220,130,"using mask");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,diff_mask_cb,0);
  fdui->diff_mask_val = obj = fl_add_input(FL_NORMAL_INPUT,430,180,150,30,"mask value");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->diff_new_reg = obj = fl_add_input(FL_NORMAL_INPUT,590,70,210,30,"new register name");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->diff_go = obj = fl_add_button(FL_NORMAL_BUTTON,720,170,60,30,"Go !");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,diff_go_cb,0);
  fdui->diff_overwrite = obj = fl_add_checkbutton(FL_PUSH_BUTTON,630,120,120,30,"over-write existing \n registers?");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
    fl_set_button(obj, 1);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,240,130,20,"Test helv-med-r Norm 10");
  obj = fl_add_button(FL_NORMAL_BUTTON,20,260,150,20,"Test helv-med-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,280,170,30,"Test helv-med-r Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,240,130,20,"Test helv-bld-r Norm 10");
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,260,150,20,"Test helv-bld-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,280,170,30,"Test helv-bld-r Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,240,130,20,"Test helv-med-o Norm 10");
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,260,150,20,"Test helv-med-o Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,280,170,30,"Test helv-med-o Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,240,150,20,"Test helv-med-bo Norm 10");
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,260,170,20,"Test helv-med-bo Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,280,190,30,"Test helv-med-bo Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,320,130,20,"Cour.-med-r Norm 10");
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,340,150,20,"Cour. -med-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,360,180,30,"Cour.-med-r Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,320,130,20,"Cour.-bld-r Norm 10");
    fl_set_object_lstyle(obj,FL_FIXEDBOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,360,180,30,"Cour.-bld-r Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_FIXEDBOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,320,130,20,"Cour. -med-o Norm 10");
    fl_set_object_lstyle(obj,FL_FIXEDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,340,150,20,"Cour.-med-o Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_FIXEDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,360,180,30,"Cour.-med-o Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_FIXEDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,320,130,20,"Cour.-med-bo Norm 10");
    fl_set_object_lstyle(obj,FL_FIXEDBOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,340,150,20,"Cour.-med-bo Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_FIXEDBOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,360,180,30,"Cour-med-bo Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_FIXEDBOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,340,150,20,"Cour.-bld-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_FIXEDBOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,400,110,20,"Times-med-r Norm 10");
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,420,120,20,"Times-med-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,440,140,30,"Times-med-r Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,400,100,20,"Times-bld-r Norm 10");
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,440,140,30,"Times.bld-r Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,400,110,20,"Times -med-i Norm 10");
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,420,130,20,"Times.-med-i Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,440,140,30,"Times-med-i Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,400,110,20,"Times.-med-b-i Norm 10");
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,420,130,20,"Times-med-b-i Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,440,150,30,"Times-med-bo Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,420,120,20,"Times-bld-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,480,110,20,"Chart-med-r Norm 10");
    fl_set_object_lstyle(obj,12);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,500,130,20,"Chart-med-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,12);
  obj = fl_add_button(FL_NORMAL_BUTTON,20,520,150,30,"Chart-med-r Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,12);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,480,120,20,"Chart-bld-r Norm 10");
    fl_set_object_lstyle(obj,13);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,520,160,30,"Chart.bld-r Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,13);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,480,110,20,"Chart. -med-i Norm 10");
    fl_set_object_lstyle(obj,14);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,500,130,20,"Chart.-med-i Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,14);
  obj = fl_add_button(FL_NORMAL_BUTTON,400,520,150,30,"Chart.-med-i Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,14);
  obj = fl_add_button(FL_NORMAL_BUTTON,210,500,140,20,"Chart-bld-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,13);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,480,120,20,"Symbol -med-r Norm 10");
    fl_set_object_lstyle(obj,15);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,500,150,20,"Symbol -med-r Norm 12");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,15);
  obj = fl_add_button(FL_NORMAL_BUTTON,590,520,160,30,"Symbol-med-i Norm 14");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,15);
  fl_end_form();

  fdui->DV_diff_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_func_window *create_form_DV_func_window(void)
{
  FL_OBJECT *obj;
  FD_DV_func_window *fdui = (FD_DV_func_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_func_window = fl_bgn_form(FL_NO_BOX, 640, 580);
  obj = fl_add_box(FL_UP_BOX,0,0,640,580,"");
  obj = fl_add_text(FL_NORMAL_TEXT,30,400,60,20,"right click");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,620,560,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_text(FL_NORMAL_TEXT,20,370,70,20,"middle click");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,40,340,50,20,"left click");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
  fdui->func = obj = fl_add_choice(FL_NORMAL_CHOICE2,90,250,180,30,"Function ");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,func_cb,0);
  fdui->func_browser = obj = fl_add_browser(FL_SELECT_BROWSER,20,20,600,220,"");
    fl_set_object_color(obj,FL_LIGHTER_COL1,FL_YELLOW);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj,func_browser_cb,0);
  fdui->func_reg_a = obj = fl_add_input(FL_NORMAL_INPUT,90,330,530,30,"A\n");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,func_reg_a_cb,0);
  fdui->func_reg_b = obj = fl_add_input(FL_NORMAL_INPUT,90,360,530,30,"B\n");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,func_reg_b_cb,0);
  fdui->func_mask = obj = fl_add_input(FL_NORMAL_INPUT,90,390,300,30,"Mask\n");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,null_cb,0);
  fdui->func_overwrite = obj = fl_add_checkbutton(FL_PUSH_BUTTON,440,480,120,30,"over-write existing \n registers?");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,null_cb,0);
    fl_set_button(obj, 1);
  fdui->func_go = obj = fl_add_button(FL_NORMAL_BUTTON,540,520,60,30,"Go !");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj,func_go_cb,0);
  fdui->func_args = obj = fl_add_input(FL_NORMAL_INPUT,360,250,260,30,"Arguments");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,null_cb,0);
  fdui->func_new_reg = obj = fl_add_input(FL_NORMAL_INPUT,90,430,530,30,"New name");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,null_cb,0);
  fdui->func_info = obj = fl_add_text(FL_NORMAL_TEXT,30,480,400,80,"Info:");
    fl_set_object_boxtype(obj,FL_SHADOW_BOX);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
  fdui->func_mask_val = obj = fl_add_input(FL_NORMAL_INPUT,470,390,150,30,"mask value");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->givec = obj = fl_add_input(FL_NORMAL_INPUT,90,280,530,30,"selection\ncontrol");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,null_cb,0);
  fl_end_form();

  fdui->DV_func_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdchoice_render_mode_9[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Point",	0,	"",	 FL_PUP_NONE},
    { "Wire Frame",	0,	"",	 FL_PUP_NONE},
    { "Hidden Line",	0,	"",	 FL_PUP_NONE},
    { "Solid",	0,	"",	 FL_PUP_NONE},
    { "Smooth Solid",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_gl_canvas_window *create_form_DV_gl_canvas_window(void)
{
  FL_OBJECT *obj;
  FD_DV_gl_canvas_window *fdui = (FD_DV_gl_canvas_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_gl_canvas_window = fl_bgn_form(FL_NO_BOX, 630, 620);
  obj = fl_add_box(FL_UP_BOX,0,0,630,620,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,10,610,600,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->gl_canvas = obj = fl_add_glcanvas(FL_NORMAL_CANVAS,20,90,590,510,"");
    fl_set_object_color(obj,FL_BLACK,FL_YELLOW);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->render_mode = obj = fl_add_choice(FL_NORMAL_CHOICE2,60,20,100,20,"Render");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_render_mode_cb,0);
    fl_set_choice_entries(obj, fdchoice_render_mode_9);
    fl_set_choice(obj,5);
  fdui->home = obj = fl_add_button(FL_NORMAL_BUTTON,160,20,50,20,"home");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_home_cb,0);
  fdui->gl_canvas_text = obj = fl_add_text(FL_NORMAL_TEXT,20,50,590,30,"text");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_color(obj,FL_BLACK,FL_MCOL);
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_Y);
  fdui->draw_bboxes = obj = fl_add_checkbutton(FL_PUSH_BUTTON,420,20,100,20,"draw bounding\nboxes");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_draw_bboxes_cb,0);
  fdui->screen_capture = obj = fl_add_button(FL_NORMAL_BUTTON,280,20,80,20,"screen capture");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_screen_capture_cb,0);
  fdui->annotation = obj = fl_add_button(FL_NORMAL_BUTTON,210,20,70,20,"annotation");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_annotation_cb,0);
  obj = fl_add_checkbutton(FL_PUSH_BUTTON,360,20,60,20,"freeze\nlens");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_freeze_lens_cb,0);
  fl_end_form();

  fdui->DV_gl_canvas_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdchoice_coarsen_step_size_10[] =
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

static FL_PUP_ENTRY fdchoice_scale_hist_11[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "history",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_cmap_hist_12[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "history",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_cmap_type_13[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "Linear",	0,	"",	 FL_PUP_NONE},
    { "Log +-",	0,	"",	 FL_PUP_NONE},
    { "Log +",	0,	"",	 FL_PUP_NONE},
    { "Log -",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_sync_14[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "time index",	0,	"",	 FL_PUP_NONE},
    { "time value",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_concat_15[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "none",	0,	"",	 FL_PUP_NONE},
    { "same time",	0,	"",	 FL_PUP_NONE},
    { "all",	0,	"",	 FL_PUP_NONE},
    {0}
};

static FL_PUP_ENTRY fdchoice_givec_hist_16[] =
{ 
    /*  itemtext   callback  shortcut   mode */
    { "history",	0,	"",	 FL_PUP_NONE},
    {0}
};

FD_DV_sl_opts_window *create_form_DV_sl_opts_window(void)
{
  FL_OBJECT *obj;
  FD_DV_sl_opts_window *fdui = (FD_DV_sl_opts_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_sl_opts_window = fl_bgn_form(FL_NO_BOX, 530, 250);
  obj = fl_add_box(FL_UP_BOX,0,0,530,250,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
  obj = fl_add_text(FL_NORMAL_TEXT,440,40,80,30,"use R# for\nrelative scale");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,510,220,"Send to local view options");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->scale = obj = fl_add_input(FL_NORMAL_INPUT,60,40,100,30,"scale");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->cmap_range_minz = obj = fl_add_input(FL_NORMAL_INPUT,80,80,70,30,"colormap\nrange:");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->cmap_range_maxz = obj = fl_add_input(FL_NORMAL_INPUT,150,80,70,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->amr = obj = fl_add_checkbutton(FL_PUSH_BUTTON,450,210,60,20,"AMR");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_amr_cb,0);
  fdui->use_mask = obj = fl_add_checkbutton(FL_PUSH_BUTTON,10,120,100,20,"");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_use_mask_cb,0);
  fdui->mask = obj = fl_add_input(FL_NORMAL_INPUT,120,120,320,30,"use mask:    \n(mask/value)");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->mask_val = obj = fl_add_input(FL_NORMAL_INPUT,440,120,70,30,"");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_BOTTOM_RIGHT);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->use_givec = obj = fl_add_checkbutton(FL_PUSH_BUTTON,10,160,100,20,"");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_use_givec_cb,0);
  fdui->givec = obj = fl_add_input(FL_NORMAL_INPUT,120,160,320,30,"selection  \ncontrol  ");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->coarsen_step_size = obj = fl_add_choice(FL_NORMAL_CHOICE2,80,200,50,30,"coarsen\nstep size");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_coarsen_cb,0);
    fl_set_choice_entries(obj, fdchoice_coarsen_step_size_10);
    fl_set_choice(obj,1);
  fdui->scale_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,160,40,70,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_scale_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_scale_hist_11);
    fl_set_choice(obj,1);
  fdui->cmap_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,220,80,70,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_cmap_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_cmap_hist_12);
    fl_set_choice(obj,1);
  fdui->cmap_type = obj = fl_add_choice(FL_NORMAL_CHOICE2,410,80,100,30,"colormap\ntype");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
    fl_set_choice_entries(obj, fdchoice_cmap_type_13);
    fl_set_choice(obj,1);
  fdui->zmin_clip = obj = fl_add_input(FL_NORMAL_INPUT,280,40,60,30,"zmin\nclip");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->zmax_clip = obj = fl_add_input(FL_NORMAL_INPUT,380,40,60,30,"zmax\nclip");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->sync = obj = fl_add_choice(FL_NORMAL_CHOICE2,180,200,80,30,"sync\nby");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_sync_cb,0);
    fl_set_choice_entries(obj, fdchoice_sync_14);
    fl_set_choice(obj,1);
  fdui->concat = obj = fl_add_choice(FL_NORMAL_CHOICE2,360,200,80,30,"concatenate\ngrids");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_concat_grids_cb,0);
    fl_set_choice_entries(obj, fdchoice_concat_15);
    fl_set_choice(obj,1);
  fdui->givec_hist = obj = fl_add_choice(FL_NORMAL_CHOICE2,440,160,70,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLDITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,sl_opts_givec_hist_cb,0);
    fl_set_choice_entries(obj, fdchoice_givec_hist_16);
    fl_set_choice(obj,1);
  fl_end_form();

  fdui->DV_sl_opts_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_capture_window *create_form_DV_capture_window(void)
{
  FL_OBJECT *obj;
  FD_DV_capture_window *fdui = (FD_DV_capture_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_capture_window = fl_bgn_form(FL_NO_BOX, 350, 170);
  obj = fl_add_box(FL_UP_BOX,0,0,350,170,"");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,330,140,"Screen Capture");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->dir = obj = fl_add_input(FL_NORMAL_INPUT,110,40,160,30,"directory");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->browse = obj = fl_add_button(FL_NORMAL_BUTTON,270,40,60,30,"browse");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,capture_browse_cb,0);
  fdui->format = obj = fl_add_input(FL_NORMAL_INPUT,110,80,110,30,"file name\nformat string");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->first_index = obj = fl_add_input(FL_NORMAL_INPUT,270,80,60,30,"first\nindex");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->start_stop = obj = fl_add_button(FL_NORMAL_BUTTON,210,120,120,30,"Start capture");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,capture_cb,0);
  fdui->jpeg_qf = obj = fl_add_input(FL_NORMAL_INPUT,110,120,90,30,"jpeg quality\nfactor(0-100)");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fl_end_form();

  fdui->DV_capture_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

static FL_PUP_ENTRY fdchoice_border_width_17[] =
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

static FL_PUP_ENTRY fdchoice_boundingboxwidth_18[] =
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

FD_DV_annotation_window *create_form_DV_annotation_window(void)
{
  FL_OBJECT *obj;
  FD_DV_annotation_window *fdui = (FD_DV_annotation_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_annotation_window = fl_bgn_form(FL_NO_BOX, 500, 320);
  obj = fl_add_box(FL_UP_BOX,0,0,500,320,"");
  obj = fl_add_text(FL_NORMAL_TEXT,370,220,120,40,"left click positions\nright click scales");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,210,480,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  obj = fl_add_text(FL_NORMAL_TEXT,370,170,110,40,"click on canvas\nto place");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,480,190,"Annotation");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  fdui->edit = obj = fl_add_choice(FL_NORMAL_CHOICE2,50,40,430,30,"edit:");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,gl_ann_edit_cb,0);
    fl_set_choice_align(obj, FL_ALIGN_LEFT);
  fdui->format_str = obj = fl_add_input(FL_NORMAL_INPUT,60,80,420,30,"format\nstring");
    fl_set_object_color(obj,FL_DARKER_COL1,FL_LIGHTER_COL1);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,gl_ann_format_str_cb,0);
  fdui->var_options = obj = fl_add_text(FL_NORMAL_TEXT,20,120,400,40,"Allowed variable names in format string (syntax --- $var#.#[f|e]):\nt,  [x|y|z|c|clip_z]min,  [x|y|z|c|clip_z]max,  zscale,  shape[x|y]");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,null_cb,0);
  fdui->font = obj = fl_add_choice(FL_NORMAL_CHOICE2,50,170,160,30,"font");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_font_cb,0);
  fdui->color = obj = fl_add_button(FL_NORMAL_BUTTON,210,170,60,30,"color");
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_color_cb,0);
  fdui->pos_text = obj = fl_add_checkbutton(FL_PUSH_BUTTON,340,170,30,30,"position");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_pos_text_cb,0);
  fdui->disp_cmap = obj = fl_add_checkbutton(FL_PUSH_BUTTON,80,220,30,30,"display\ncolormap");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_disp_cmap_cb,0);
  fdui->pos_cmap = obj = fl_add_checkbutton(FL_PUSH_BUTTON,340,220,30,30,"position");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_pos_cmap_cb,0);
  fdui->border_color = obj = fl_add_button(FL_NORMAL_BUTTON,210,220,60,30,"color");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_border_color_cb,0);
  fdui->border_width = obj = fl_add_choice(FL_NORMAL_CHOICE2,170,220,40,30,"border\nwidth");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_border_width_cb,0);
    fl_set_choice_entries(obj, fdchoice_border_width_17);
    fl_set_choice(obj,1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,260,480,50,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  obj = fl_add_choice(FL_NORMAL_CHOICE2,140,270,40,30,"bounding box width");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESBOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_bbox_width_cb,0);
    fl_set_choice_entries(obj, fdchoice_boundingboxwidth_18);
    fl_set_choice(obj,1);
  obj = fl_add_button(FL_NORMAL_BUTTON,180,270,60,30,"color");
    fl_set_object_lcolor(obj,FL_GREEN);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,gl_ann_bbox_color_cb,0);
  fl_end_form();

  fdui->DV_annotation_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_DV_dbrowser_window *create_form_DV_dbrowser_window(void)
{
  FL_OBJECT *obj;
  FD_DV_dbrowser_window *fdui = (FD_DV_dbrowser_window *) fl_calloc(1, sizeof(*fdui));

  fdui->DV_dbrowser_window = fl_bgn_form(FL_NO_BOX, 740, 480);
  obj = fl_add_box(FL_UP_BOX,0,0,740,480,"");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,640,20,90,80,"");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,20,630,80,"Data browser");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->info = obj = fl_add_text(FL_NORMAL_TEXT,20,30,610,60,"text\ntext\ntext");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,100,720,370,"");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->data = obj = fl_add_text(FL_NORMAL_TEXT,230,180,490,280,"text");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_RIGHT_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->h_slider = obj = fl_add_slider(FL_HOR_BROWSER_SLIDER,230,110,490,30,"");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj,db_h_slider_cb,0);
  fdui->v_slider = obj = fl_add_slider(FL_VERT_BROWSER_SLIDER,20,180,40,280,"");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthWest);
    fl_set_object_resize(obj, FL_RESIZE_Y);
    fl_set_object_callback(obj,db_v_slider_cb,0);
  fdui->i_info = obj = fl_add_text(FL_NORMAL_TEXT,230,140,490,40,"i index, coord info");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->j_info = obj = fl_add_text(FL_NORMAL_TEXT,60,180,170,280,"j index, coord info");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_color(obj,FL_BOTTOM_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthWest);
    fl_set_object_resize(obj, FL_RESIZE_Y);
  fdui->data_fs = obj = fl_add_input(FL_NORMAL_INPUT,120,110,100,30,"data print string");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,db_data_fs_cb,0);
  fdui->coord_fs = obj = fl_add_input(FL_NORMAL_INPUT,120,140,100,30,"coord print string");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj,db_coord_fs_cb,0);
  fdui->screen_dump = obj = fl_add_checkbutton(FL_RADIO_BUTTON,650,30,30,20,"screen\ndump");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,db_screen_dump_cb,0);
  obj = fl_add_checkbutton(FL_RADIO_BUTTON,650,70,30,20,"reverse\ny coord");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_TIMES_STYLE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj,db_reverse_y_cb,0);
    fl_set_button(obj, 1);
  fl_end_form();

  fdui->DV_dbrowser_window->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

