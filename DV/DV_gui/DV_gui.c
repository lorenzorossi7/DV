//=============================================================================
// DV_gui.c
// 
// call-back functions and initialization
//=============================================================================

#include <GL/gl.h>
#include <GL/glx.h>
#include "forms.h"
#include "DV_forms.h"
#include "DVault.h"
#include "DV_gui.h"
#include "reg.h"
/* TODO: Rename misc.h -> DVmisc.h or some such.  Thanks ... MWC */
#include "../misc.h"
#include "fncs.h"
#include "send_bbh_xyz.h"
#include "send_accept_sdf.h"
#include "send_gl_canvas.h"
#include "DVCliser.h"
#include <stdlib.h>
#include "func_table.h"
#include "userf_table.h"
#include <string.h>

int ret_arg=0;
int auto_refresh=0;
FD_DV_main_window *fd_DV_main_window;
FD_DV_diff_window *fd_DV_diff_window;
FD_DV_func_window *fd_DV_func_window;
FD_DV_gl_canvas_window *fd_DV_gl_canvas_window;
FD_DV_hvs_window *fd_DV_hvs_window;
FD_DV_sl_opts_window *fd_DV_sl_opts_window;
FD_DV_capture_window *fd_DV_capture_window;
FD_DV_f_select_window *fd_DV_f_select_window;
FD_DV_annotation_window *fd_DV_annotation_window;
FD_DV_dbrowser_window *fd_DV_dbrowser_window;

/* Beginning of Martin Snajdr added*/
FD_DV_mstest_item1_window *fd_DV_mstest_item1_window;
FD_DV_mstest_userfdef_window *fd_DV_mstest_userfdef_window;
int read_userf_file(char *file_name);
int write_userf_file(char *file_name);
int mstest_item1_close_cb(FL_FORM* form, void *data);
int mstest_userfdef_close_cb(FL_FORM* form, void *data);
void update_userf(int ufunc);
void fill_userf_list();
void fill_macro_code(int item);
void fill_AB_etc(int ufunc,int mline);
void switch_macro_lines(int line1,int line2, int select);
void delete_macro_line(int line);
void cut_space(char *str1,char *str2);
int get_func_indx(char *str2);
void save_AB_etc(int ufunc,int mline,int field);
void create_new_userf();
void initialize_userf();
void add_macro_line(int i);
int delete_userf_line(int line);
void switch_userf_lines(int line1,int line2, int select);
void switch_item1_input_lines(int line1,int line2, int select);
void delete_item1_input_line(int line);
int contains_input(char *str,int *input_pos,int *input_len,int *input_num,int *maxinput);
void backup_userf(int line);
void restore_userf(int line);
void free_userf_table_entry(userf_table_entry *ute);
void free_userf_macro_line(userf_macro_line *ufml);
void execute_userf(int fline,int line,int ninp_regs);
void subs_regs(char *regstr,char *str,int line);
int get_ninput(int func);
/* End of martin Snajdr added*/

int db_open=0;
int db_screen_dump=0;
int db_reverse_y=1;

const int no_GUI=0;

char diff_reg[MAX_NAME_LENGTH]="",diff_mask_reg[MAX_NAME_LENGTH]="",
     diff_wrt[MAX_NAME_LENGTH]="t",diff_new_reg[MAX_NAME_LENGTH]="";

double diff_mask_val=1;
int diff_wrt_ind=1;

#define RB_ACCEPT_SDF 1
#define RB_GL_CANVAS  2
int rb_send_to=RB_GL_CANVAS;

#define POS_OFF 0
#define POS_TEXT 1
#define POS_CMAP 2
int gl_pos=POS_OFF;

int diff_close_cb(FL_FORM* form, void *data);
int close_cb(FL_FORM* form, void *data);
int func_close_cb(FL_FORM* form, void *data);
int options_close_cb(FL_FORM* form, void *data);
int capture_close_cb(FL_FORM* form, void *data);
int f_select_close_cb(FL_FORM* form, void *data);
int annotation_close_cb(FL_FORM* form, void *data);
int gl_canvas_close_cb(FL_FORM* form, void *data);
int hvs_close_cb(FL_FORM* form, void *data);
int dbrowser_close_cb(FL_FORM* form, void *data);
int gl_canvas_event_handler(FL_OBJECT *obj, Window win, int win_width,
                            int win_height, XEvent *xev, void *data);

void db_new_grid(reg *r,time_str *ts,grid *g);
void refresh_db(void);
void add_to_hist(const char *s, FL_OBJECT *obj);
void load_def_options(void);
void set_gc_opts(int force);
void switch_gl_canvas(int canvas);
void set_hvs_thumbwheel(void);

int browser_key_cb(XEvent *ev, void *data);

//=============================================================================
// The following var's are used to manage multiple 'send_to_local_view'
// type gl_canvases. 
//=============================================================================

int curr_gl_canvas=-1;
FL_OBJECT *curr_gl_canvas_obj=0;
int gl_canvas_open[NUM_GL_CANVASES]={0,0};
double model_mat_save[16];

grid_conversion_params gc_opts_s[NUM_GL_CANVASES];
render_params rend_opts_s[NUM_GL_CANVASES];
camera_params cam_opts_s[NUM_GL_CANVASES];
int *grid_dl_s[NUM_GL_CANVASES]={0,0},*bbox_dl_s[NUM_GL_CANVASES]={0,0};
int dl_view_offset_s[NUM_GL_CANVASES]={0,2*MAX_DISPLAY_LISTS};
int n_dl_s[NUM_GL_CANVASES];
double last_model_mat_s[NUM_GL_CANVASES][16];
double model_mat_save_s[NUM_GL_CANVASES][16];
vect os_cmap_p1_s[NUM_GL_CANVASES],os_cmap_p2_s[NUM_GL_CANVASES];
color os_cmap_bcol_s[NUM_GL_CANVASES];
int os_cmap_bwidth_s[NUM_GL_CANVASES],show_os_cmap_s[NUM_GL_CANVASES],os_cmap_image_first_s[NUM_GL_CANVASES];
char *os_cmap_image_s[NUM_GL_CANVASES]={0,0};
int os_cmap_size_x_s[NUM_GL_CANVASES],os_cmap_size_y_s[NUM_GL_CANVASES];
int prev_os_bwidth_s[NUM_GL_CANVASES];
color prev_os_bcol_s[NUM_GL_CANVASES];
double prev_os_x1_s[NUM_GL_CANVASES],prev_os_y1_s[NUM_GL_CANVASES];
int prev_os_cmap_type_s[NUM_GL_CANVASES];
int prev_os_cmap_scheme_s[NUM_GL_CANVASES];

int prev_canvas_w[NUM_GL_CANVASES];
int prev_canvas_h[NUM_GL_CANVASES];

//=============================================================================
// starts the GUI
//=============================================================================
void *start_gui(void *arg)
{
   int argc=1,i,r,g,b;
   char *argv[1]={"dummy"};
   char buf[10];

   // some of the following bit of code was generated by fdesign (with "emit main" menu
   // option)
   
   fl_initialize(&argc, argv, 0, 0, 0);
   fd_DV_main_window = create_form_DV_main_window();
   fd_DV_diff_window = create_form_DV_diff_window();
   fd_DV_func_window = create_form_DV_func_window();
   fd_DV_sl_opts_window = create_form_DV_sl_opts_window();
   fd_DV_gl_canvas_window = create_form_DV_gl_canvas_window();
   fd_DV_hvs_window = create_form_DV_hvs_window();
   fd_DV_capture_window = create_form_DV_capture_window();
   fd_DV_f_select_window = create_form_DV_f_select_window();
   fd_DV_annotation_window = create_form_DV_annotation_window();
   fd_DV_dbrowser_window = create_form_DV_dbrowser_window();
   fd_DV_mstest_item1_window = create_form_DV_mstest_item1_window();
   fd_DV_mstest_userfdef_window = create_form_DV_mstest_userfdef_window();

   fl_hide_object(fd_DV_f_select_window->cm_min);
   fl_hide_object(fd_DV_f_select_window->cm_max);

   /* fill-in form initialization code */

   /* show the first form */
   fl_set_app_nomainform(1); // to get around raising/lowering bug
   fl_show_form(fd_DV_main_window->DV_main_window,FL_PLACE_CENTERFREE,FL_FULLBORDER,"Data Vault");

   /* register double-clicking in browser window */
   // didn't work well at all -- only registered about 1/3 of the double clicks
//   fl_set_browser_dblclick_callback(fd_DV_main_window->dv_browser,dv_browser_cb,1);
//   fl_set_object_dblclick(fd_DV_main_window->dv_browser,100); 
   /* increase font size (option in fdesign doesn't work) */
   fl_set_browser_fontsize(fd_DV_main_window->dv_browser,12);
   fl_set_browser_fontsize(fd_DV_diff_window->diff_browser,12);
   fl_set_browser_fontsize(fd_DV_diff_window->diff_mask,12);
   fl_set_browser_fontsize(fd_DV_func_window->func_browser,12);
   fl_set_choice_fontsize(fd_DV_diff_window->diff_wrt,12);
   fl_set_browser_fontsize(fd_DV_func_window->func,12);
   fl_set_input(fd_DV_diff_window->diff_mask_val,"1");
   fl_set_input(fd_DV_func_window->func_mask_val,"1");
   fl_set_choice_fontstyle(fd_DV_sl_opts_window->sync_t,FL_ITALIC_STYLE);
   fl_set_choice_fontstyle(fd_DV_sl_opts_window->concat_grids,FL_ITALIC_STYLE);

   //--------------------------------------------------------------------------
   // default grid conversion options, and some rend opts defaults
   //--------------------------------------------------------------------------
   fl_set_input(fd_DV_sl_opts_window->scale,"R.1");
   fl_set_input(fd_DV_hvs_window->x_input,"0");
   fl_set_input(fd_DV_hvs_window->y_input,"0");
   fl_set_input(fd_DV_hvs_window->z_input,"0");
   gc_opts_s[GL_LOCAL_VIEW].in_zscale=strdup(fl_get_input(fd_DV_sl_opts_window->scale));
   gc_opts_s[GL_HVS].in_zscale=strdup("1");

   for (i=0; i<NUM_GL_CANVASES; i++)
   {
      gc_opts_s[i].in_cmap_minz=0;
      gc_opts_s[i].in_cmap_maxz=0;
      gc_opts_s[i].in_zmin_clip=0;
      gc_opts_s[i].in_zmax_clip=0;
      gc_opts_s[i].givec=0;
      gc_opts_s[i].concat_grids=CONCAT_TIME;
      gc_opts_s[i].sync_t=SYNC_T_INDEX;
      gc_opts_s[i].cmap_type=CMAP_LIN;
      gc_opts_s[i].cmap_scheme=CMAP_STANDARD;
      fl_set_input(fd_DV_sl_opts_window->mask_val,"0");
      gc_opts_s[i].mask_val=0;

      // common mask (and givec)
      fl_set_input(fd_DV_sl_opts_window->mask,"/chr");
      if (i==0) gc_opts_s[i].mask=strdup(fl_get_input(fd_DV_sl_opts_window->mask));
      else gc_opts_s[i].mask=gc_opts_s[0].mask;

      gc_opts_s[i].coarsen_step=1;
      gc_opts_s[i].ignore_ccoords=0;
      gc_opts_s[i].amr=0;
      rend_opts_s[i].use_spec=0;
      rend_opts_s[i].shininess=100;
      rend_opts_s[i].line_width_1d=2;
      rend_opts_s[i].use_key_frames=0;
      rend_opts_s[i].anti_alias=0;
      rend_opts_s[i].first=1;
      gc_opts_s[i].monotone=0;
      rend_opts_s[i].bg_color.r=0;
      rend_opts_s[i].bg_color.g=0;
      rend_opts_s[i].bg_color.b=0;
      gc_opts_s[i].fg_color.r=0.75;
      gc_opts_s[i].fg_color.g=0.75;
      gc_opts_s[i].fg_color.b=0.75;
      gc_opts_s[i].slice=SLICE_X;
      gc_opts_s[i].slice_v=0;
      if (i==GL_LOCAL_VIEW)
      {
         gc_opts_s[i].do_slice=1; 
         gc_opts_s[i].bbox_only=0; 
         gc_opts_s[i].color_clip=0; 
         rend_opts_s[i].render_mode=RENDER_MODE_SOLID;
         rend_opts_s[i].smooth=1;
         rend_opts_s[i].draw_bboxes=0;
         rend_opts_s[i].draw_slice_box=0;
         rend_opts_s[i].pt_size_1d=4;
         prev_canvas_w[i]=fd_DV_gl_canvas_window->gl_canvas->w;
         prev_canvas_h[i]=fd_DV_gl_canvas_window->gl_canvas->h;
      }
      else 
      {
         gc_opts_s[i].do_slice=0;
         gc_opts_s[i].bbox_only=0;
         gc_opts_s[i].color_clip=0; 
         rend_opts_s[i].render_mode=RENDER_MODE_POINT;
         rend_opts_s[i].smooth=0;
         rend_opts_s[i].draw_bboxes=1;
         rend_opts_s[i].draw_slice_box=1;
         fl_set_menu_item_mode(fd_DV_hvs_window->hvs_options_menu,5,FL_PUP_CHECK | FL_PUP_RADIO);
         fl_set_menu_item_mode(fd_DV_hvs_window->hvs_options_menu,6,FL_PUP_CHECK | FL_PUP_RADIO);
         rend_opts_s[i].pt_size_1d=1;
         prev_canvas_w[i]=fd_DV_hvs_window->gl_canvas->w;
         prev_canvas_h[i]=fd_DV_hvs_window->gl_canvas->h;
      }

      if (!(bbox_dl_s[i]=(int *)malloc(sizeof(int)*MAX_DISPLAY_LISTS)))
         printf("DV_gui ... out of memory; can't allocate display lists\n");
      if (!(grid_dl_s[i]=(int *)malloc(sizeof(int)*MAX_DISPLAY_LISTS)))
         printf("DV_gui ... out of memory; can't allocate display lists\n");

      cam_opts_s[i].dispx=cam_opts.dispy=0;
      cam_opts_s[i].rot_axis.x=0;
      cam_opts_s[i].rot_axis.y=0;
      cam_opts_s[i].rot_axis.z=1;
      cam_opts_s[i].rot_angle=0;
      cam_opts_s[i].zoom=1;
      cam_opts_s[i].proj_w=0;

      os_cmap_image_first_s[0]=1;

   }

   //--------------------------------------------------------------------------
   // default screen capture options
   //--------------------------------------------------------------------------
   fl_set_input(fd_DV_capture_window->first_index,"0");
   fl_set_input(fd_DV_capture_window->format,"scr_save_%i.jpeg");
   fl_set_input(fd_DV_capture_window->dir,"");
   fl_set_input(fd_DV_capture_window->jpeg_qf,"100");

   //--------------------------------------------------------------------------
   // default f_select options
   //--------------------------------------------------------------------------
   fl_set_input(fd_DV_f_select_window->wf,"0");
   fl_set_input(fd_DV_f_select_window->show_min,"0.0");
   fl_set_input(fd_DV_f_select_window->show_max,"1.0");

   auto_refresh=1;
   fl_set_menu_item_mode(fd_DV_main_window->view_menu,2,FL_PUP_CHECK | FL_PUP_RADIO);

   fl_set_goodies_font(FL_NORMAL_STYLE,14);

   fl_set_atclose(close_cb,0);
   fl_set_form_atclose(fd_DV_main_window->DV_main_window,close_cb,0);
   fl_set_form_atclose(fd_DV_diff_window->DV_diff_window,diff_close_cb,0);
   fl_set_form_atclose(fd_DV_func_window->DV_func_window,func_close_cb,0);
   fl_set_form_atclose(fd_DV_gl_canvas_window->DV_gl_canvas_window,gl_canvas_close_cb,0);
   fl_set_form_atclose(fd_DV_hvs_window->DV_hvs_window,hvs_close_cb,0);
   fl_set_form_atclose(fd_DV_sl_opts_window->DV_sl_opts_window,options_close_cb,0);
   fl_set_form_atclose(fd_DV_capture_window->DV_capture_window,capture_close_cb,0);
   fl_set_form_atclose(fd_DV_f_select_window->DV_f_select_window,f_select_close_cb,0);
   fl_set_form_atclose(fd_DV_annotation_window->DV_annotation_window,annotation_close_cb,0);
   fl_set_form_atclose(fd_DV_dbrowser_window->DV_dbrowser_window,dbrowser_close_cb,0);
   fl_set_form_atclose(fd_DV_mstest_item1_window->DV_mstest_item1_window,mstest_item1_close_cb,0);
   fl_set_form_atclose(fd_DV_mstest_userfdef_window->DV_mstest_userfdef_window,mstest_userfdef_close_cb,0);

   fl_set_form_minsize(fd_DV_main_window->DV_main_window,600,460);

   //--------------------------------------------------------------------------
   // set up function browser 
   //--------------------------------------------------------------------------
   i=0;
   initialize_userf();
   while(strlen(func_table[i].name)) 
   {
      fl_add_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_func,func_table[i].name);
      fl_add_browser_line(fd_DV_func_window->func,func_table[i++].name);
   }

   fl_select_browser_line(fd_DV_func_window->func,1);
//   fl_select_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_func,1);

   //--------------------------------------------------------------------------
   // setup canvas event handler
   //--------------------------------------------------------------------------
   fl_add_canvas_handler(fd_DV_gl_canvas_window->gl_canvas,ButtonPress,gl_canvas_event_handler,0);
   fl_add_canvas_handler(fd_DV_gl_canvas_window->gl_canvas,ButtonRelease,gl_canvas_event_handler,0);
   fl_add_canvas_handler(fd_DV_gl_canvas_window->gl_canvas,MotionNotify,gl_canvas_event_handler,0);
   fl_add_canvas_handler(fd_DV_gl_canvas_window->gl_canvas,Expose,gl_canvas_event_handler,0);
   fl_add_canvas_handler(fd_DV_gl_canvas_window->gl_canvas,ResizeRequest,gl_canvas_event_handler,0);

   //--------------------------------------------------------------------------
   // use same handler for hvs window
   //--------------------------------------------------------------------------
   fl_add_canvas_handler(fd_DV_hvs_window->gl_canvas,ButtonPress,gl_canvas_event_handler,0);
   fl_add_canvas_handler(fd_DV_hvs_window->gl_canvas,ButtonRelease,gl_canvas_event_handler,0);
   fl_add_canvas_handler(fd_DV_hvs_window->gl_canvas,MotionNotify,gl_canvas_event_handler,0);
   fl_add_canvas_handler(fd_DV_hvs_window->gl_canvas,Expose,gl_canvas_event_handler,0);
   fl_add_canvas_handler(fd_DV_hvs_window->gl_canvas,ResizeRequest,gl_canvas_event_handler,0);

   //--------------------------------------------------------------------------
   // annotation defaults, and font setup
   //--------------------------------------------------------------------------
   fl_set_object_lcol(fd_DV_annotation_window->color,3);
   fl_get_icm_color(3,&r,&g,&b);
   for(i=0;i<NUM_FONTS;i++)
      fl_addto_choice(fd_DV_annotation_window->font,xfont_names[i]); 
   for(i=0;i<MAX_OS_TEXTS;i++)
   {
      sprintf(buf,"%i :",i+1);
      fl_addto_choice(fd_DV_annotation_window->edit,buf);
      os_text_list[i].col.r=(double)r/255;
      os_text_list[i].col.g=(double)g/255;
      os_text_list[i].col.b=(double)b/255;
      os_text_list[i].col_ind=3;
      os_text_list[i].pos.x=20;
      os_text_list[i].pos.y=20;
      os_text_list[i].format_str=0;
      os_text_list[i].print_str=0;
      os_text_list[i].which_canvas=0;
   }
   switch_gl_canvas(GL_LOCAL_VIEW);

   //--------------------------------------------------------------------------
   // data browser defaults
   //--------------------------------------------------------------------------
   fl_set_input(fd_DV_dbrowser_window->data_fs,"%14.8f");
   fl_set_input(fd_DV_dbrowser_window->coord_fs,"%12.6f");

   add_to_hist("1.0",fd_DV_gl_canvas_window->gl_zoom_hist);
   add_to_hist("1.5",fd_DV_gl_canvas_window->gl_zoom_hist);
   add_to_hist("0.5",fd_DV_gl_canvas_window->gl_zoom_hist);
   add_to_hist("0.0",fd_DV_gl_canvas_window->gl_theta_hist);
   add_to_hist("40.0",fd_DV_gl_canvas_window->gl_theta_hist);
   add_to_hist("60.0",fd_DV_gl_canvas_window->gl_theta_hist);
   add_to_hist("0.0",fd_DV_gl_canvas_window->gl_phi_hist);
   add_to_hist("-70.0",fd_DV_gl_canvas_window->gl_phi_hist);
   add_to_hist("30.0",fd_DV_gl_canvas_window->gl_phi_hist);
   add_to_hist("-30.0",fd_DV_gl_canvas_window->gl_phi_hist);
   fl_set_input(fd_DV_gl_canvas_window->gl_zoom,"1.5");
   fl_set_input(fd_DV_gl_canvas_window->gl_theta,"40.0");
   fl_set_input(fd_DV_gl_canvas_window->gl_phi,"-70.0");

   load_def_options();

   fl_do_forms();

   pthread_exit((void *)&ret_arg);

   return 0;
}

//=============================================================================
// The following functions are for implementing multiple canvases:
// save_gl_canvas(i) copies the current canvas variables to the '_s' 
//    save storage
// restore_gl_canvas(i) copies values from the '_s' save storage to the 
//    current canvas variables
// switch_gl_canvas(i) switches to canvas i
//
// NOTE: a few options (filter, mask, sync, amr, and concat) apply to all canvases ,
//       and even though separate copies of these variables are maintained
//       here, they are always set to the same value.
//=============================================================================
void save_gl_canvas(int canvas)
{
   char *p,*q;
   int i;

   if (canvas!=GL_LOCAL_VIEW && canvas!=GL_HVS) return;

   p=(char *)(&gc_opts); q=(char *)(&gc_opts_s[canvas]);
   for (i=0; i<sizeof(struct grid_conversion_params); i++) *q++=*p++;

   p=(char *)(&rend_opts); q=(char *)(&rend_opts_s[canvas]);
   for (i=0; i<sizeof(struct render_params); i++) *q++=*p++;

   p=(char *)(&cam_opts); q=(char *)(&cam_opts_s[canvas]);
   for (i=0; i<sizeof(struct camera_params); i++) *q++=*p++;

   n_dl_s[canvas]=n_dl;
   dl_view_offset_s[canvas]=dl_view_offset;
   grid_dl_s[canvas]=grid_dl;
   bbox_dl_s[canvas]=bbox_dl;

   for (i=0; i<16; i++) 
   {
      last_model_mat_s[canvas][i]=last_model_mat[i];
      model_mat_save_s[canvas][i]=model_mat_save[i];
   }

   for (i=0; i<3; i++) 
   {
      (&(os_cmap_p1_s[canvas].x))[i]=(&(os_cmap_p1.x))[i];
      (&(os_cmap_p2_s[canvas].x))[i]=(&(os_cmap_p2.x))[i];
      (&(os_cmap_bcol_s[canvas].r))[i]=(&(os_cmap_bcol.r))[i];
      (&(prev_os_bcol_s[canvas].r))[i]=(&(prev_os_bcol.r))[i];
   }
   os_cmap_bwidth_s[canvas]=os_cmap_bwidth;
   show_os_cmap_s[canvas]=show_os_cmap;
   os_cmap_image_first_s[canvas]=os_cmap_image_first;
   os_cmap_image_s[canvas]=os_cmap_image;
   os_cmap_size_x_s[canvas]=os_cmap_size_x;
   os_cmap_size_y_s[canvas]=os_cmap_size_y;
   prev_os_bwidth_s[canvas]=prev_os_bwidth;
   prev_os_x1_s[canvas]=prev_os_x1;
   prev_os_y1_s[canvas]=prev_os_y1;
   prev_os_cmap_type_s[canvas]=prev_os_cmap_type;
   prev_os_cmap_scheme_s[canvas]=prev_os_cmap_scheme;

}

void restore_gl_canvas(int canvas)
{
   char *p,*q;
   int i;

   if (canvas!=GL_LOCAL_VIEW && canvas!=GL_HVS) return;

   p=(char *)(&gc_opts); q=(char *)(&gc_opts_s[canvas]);
   for (i=0; i<sizeof(struct grid_conversion_params); i++) *p++=*q++;

   p=(char *)(&rend_opts); q=(char *)(&rend_opts_s[canvas]);
   for (i=0; i<sizeof(struct render_params); i++) *p++=*q++;

   p=(char *)(&cam_opts); q=(char *)(&cam_opts_s[canvas]);
   for (i=0; i<sizeof(struct camera_params); i++) *p++=*q++;

   n_dl=n_dl_s[canvas];
   dl_view_offset=dl_view_offset_s[canvas];
   grid_dl=grid_dl_s[canvas];
   bbox_dl=bbox_dl_s[canvas];

   for (i=0; i<16; i++) 
   {
      last_model_mat[i]=last_model_mat_s[canvas][i];
      model_mat_save[i]=model_mat_save_s[canvas][i];
   }

   for (i=0; i<3; i++) 
   {
      (&(os_cmap_p1.x))[i]=(&(os_cmap_p1_s[canvas].x))[i];
      (&(os_cmap_p2.x))[i]=(&(os_cmap_p2_s[canvas].x))[i];
      (&(os_cmap_bcol.r))[i]=(&(os_cmap_bcol_s[canvas].r))[i];
      (&(prev_os_bcol.r))[i]=(&(prev_os_bcol_s[canvas].r))[i];
   }
   os_cmap_bwidth=os_cmap_bwidth_s[canvas];
   os_cmap_image_first=os_cmap_image_first_s[canvas];
   show_os_cmap=show_os_cmap_s[canvas];
   os_cmap_image=os_cmap_image_s[canvas];
   os_cmap_size_x=os_cmap_size_x_s[canvas];
   os_cmap_size_y=os_cmap_size_y_s[canvas];
   prev_os_bwidth=prev_os_bwidth_s[canvas];
   prev_os_x1=prev_os_x1_s[canvas];
   prev_os_y1=prev_os_y1_s[canvas];
   prev_os_cmap_type=prev_os_cmap_type_s[canvas];
   prev_os_cmap_scheme=prev_os_cmap_scheme_s[canvas];

}

void switch_gl_canvas(int canvas)
{
   if (gl_canvas_open[canvas])
   {
      switch(canvas)
      {
         case GL_LOCAL_VIEW:
            curr_gl_canvas_obj=fd_DV_gl_canvas_window->gl_canvas;
            break;
         case GL_HVS:
            curr_gl_canvas_obj=fd_DV_hvs_window->gl_canvas;
            break;
         default:
            printf("switch_gl_canvas: error ... unknown canvas\n");
            return;
      }
      fl_activate_glcanvas(curr_gl_canvas_obj);
   }
   else curr_gl_canvas_obj=0;

   if (curr_gl_canvas == canvas) return;
   if (curr_gl_canvas >=0) 
   {
      save_gl_canvas(curr_gl_canvas);
   }
   restore_gl_canvas(canvas);
   curr_gl_canvas=canvas;

   return;
}  

//-----------------------------------------------------------------------------
// for historic reasons, *the* gl_canvas is GL_LOCAL_VIEW
//-----------------------------------------------------------------------------
        
int gl_canvas_close_cb(FL_FORM* form, void *data)
{
   switch_gl_canvas(GL_LOCAL_VIEW);
   glGetDoublev(GL_MODELVIEW_MATRIX,model_mat_save);
   os_cmap_image_first=1;
   save_gl_canvas(GL_LOCAL_VIEW);

   fl_hide_form(form);
   gl_canvas_open[GL_LOCAL_VIEW]=0;
   curr_gl_canvas_obj=0;
   return 1;
}

void open_gl_canvas(void)
{
   static int first=1;
   FL_OBJECT *obj=fd_DV_gl_canvas_window->gl_canvas;

   fl_show_form(fd_DV_gl_canvas_window->DV_gl_canvas_window,
                FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV local view");
   fl_set_form_minsize(fd_DV_gl_canvas_window->DV_gl_canvas_window,100,100);
   //--------------------------------------------------------------------------
   // as discussed in xforms manual, Ch 21. on canvas objects:
   //--------------------------------------------------------------------------
   fl_remove_selected_xevent(fl_get_canvas_id(fd_DV_gl_canvas_window->gl_canvas),
                             PointerMotionMask|PointerMotionHintMask);
   gl_canvas_open[GL_LOCAL_VIEW]=1;
   switch_gl_canvas(GL_LOCAL_VIEW);
   if (!first)
   {
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glMultMatrixd(model_mat_save);
   }
   else first=0;

   set_gl_render_mode();
   cam_opts.hw_aspect_ratio=(double)obj->h/obj->w;
   refresh_gl_display();
}

int hvs_close_cb(FL_FORM* form, void *data)
{
   switch_gl_canvas(GL_HVS);
   glGetDoublev(GL_MODELVIEW_MATRIX,model_mat_save);
   os_cmap_image_first=1;
   save_gl_canvas(GL_HVS);

   fl_hide_form(form);
   gl_canvas_open[GL_HVS]=0;
   curr_gl_canvas_obj=0;
   return 1;
}

void open_hvs(void)
{
   static int first=1;
   FL_OBJECT *obj=fd_DV_hvs_window->gl_canvas;

   fl_show_form(fd_DV_hvs_window->DV_hvs_window,
                FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV HVS");
   fl_set_form_minsize(fd_DV_hvs_window->DV_hvs_window,100,100);
   //--------------------------------------------------------------------------
   // as discussed in xforms manual, Ch 21. on canvas objects:
   //--------------------------------------------------------------------------
   fl_remove_selected_xevent(fl_get_canvas_id(obj),
                             PointerMotionMask|PointerMotionHintMask);
   gl_canvas_open[GL_HVS]=1;
   switch_gl_canvas(GL_HVS);
   if (!first)
   {
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glMultMatrixd(model_mat_save);
   }
   else first=0;

   set_gl_render_mode();
   cam_opts.hw_aspect_ratio=(double)obj->h/obj->w;
   refresh_gl_display();
}

int close_cb(FL_FORM* form, void *data)
{
    printf("... stopping the server and closing the gui\n");
    // The following is not the most elegant method of
    // shutting down the server ...
    stop_dv_service=1;
    if (ser0_connect_DV(getenv("DVHOST"),DV_PORT)<0)
    printf("... ERROR ... cannot connect to DVHOST\n");
    // I'm not sure whether this is the correct way to shut
    // down the GUI from within a callback routine, but here goes...
    fl_hide_form(form);
    pthread_exit((void *)&ret_arg);
    return 1;
}

int diff_close_cb(FL_FORM* form, void *data)
{
   fl_hide_form(form);
   return 1;
}

int func_close_cb(FL_FORM* form, void *data)
{
   fl_hide_form(form);
   return 1;
}

int options_close_cb(FL_FORM* form, void *data)
{
   fl_hide_form(form);
   return 1;
}

int capture_close_cb(FL_FORM* form, void *data)
{
   fl_hide_form(form);
   return 1;
}

int f_select_close_cb(FL_FORM* form, void *data)
{
   fl_hide_form(form);
   return 1;
}

int annotation_close_cb(FL_FORM* form, void *data)
{
   fl_hide_form(form);
   return 1;
}

int dbrowser_close_cb(FL_FORM* form, void *data)
{
   fl_hide_form(form);
   db_open=0;
   return 1;
}
//=============================================================================
// builds the browser list(s) from scratch
//=============================================================================
void create_browser_list()
{
   reg *r;
   time_str *ts;
   level *l;
   grid *g;
   int line=0,ti,li,gi,i,topline;
   int diff_line=0,diff_topline,diff_selected=0,mask_selected=0;
   int mask_line=0,mask_topline;
   int func_line=0,func_topline;
   int mstest_item1_line=0,mstest_item1_topline;
   int mstest_userfdef_line=0,mstest_userfdef_topline;
   FL_OBJECT *bl=fd_DV_main_window->dv_browser;
   FL_OBJECT *diff_bl=fd_DV_diff_window->diff_browser;
   FL_OBJECT *mask_bl=fd_DV_diff_window->diff_mask;
   FL_OBJECT *func_bl=fd_DV_func_window->func_browser;
   FL_OBJECT *mstest_item1_bl=fd_DV_mstest_item1_window->mstest_item1_browser;
   FL_OBJECT *mstest_userfdef_bl=fd_DV_mstest_userfdef_window->mstest_userfdef_browser;
   char buffer[MAX_NAME_LENGTH*2];
   s_iter it;

   // this routine can be called from both threads, so lock immediately
   // to prevent simultaneuos xforms calls
   LOCK_REG_LIST;

   fl_freeze_form(bl->form);
   fl_freeze_form(diff_bl->form);
   fl_freeze_form(func_bl->form);
   fl_freeze_form(mstest_item1_bl->form);
   fl_freeze_form(mstest_userfdef_bl->form);
   topline=fl_get_browser_topline(bl);
   diff_topline=fl_get_browser_topline(diff_bl);
   mask_topline=fl_get_browser_topline(mask_bl);
   func_topline=fl_get_browser_topline(func_bl);
   mstest_item1_topline=fl_get_browser_topline(mstest_item1_bl);
   mstest_userfdef_topline=fl_get_browser_topline(mstest_userfdef_bl);
   fl_clear_browser(bl);
   fl_clear_browser(diff_bl);
   fl_clear_browser(mask_bl);
   fl_clear_browser(func_bl);
   fl_clear_browser(mstest_item1_bl);
   fl_clear_browser(mstest_userfdef_bl);


   fl_add_browser_line(mask_bl,"<none>");
   mask_line++;
   if (!strlen(diff_mask_reg) || !strcmp("<none>",diff_mask_reg))
   {
      strcpy(diff_mask_reg,"<none>");
      fl_select_browser_line(mask_bl,mask_line);
      mask_selected=1;
   }

   r=reg_list;
   while(r)
   {
      LOCK_REG(r);
      //----------------------------------------------------------------
      // It is more useful, I think, to display global
      // level information in the dv window, so here we just call
      // init_s_iter to calculate this information for us,stored
      // in the ln field of the level structure. (For historical reasons 
      // we do not use the s_iter in this function)
      //----------------------------------------------------------------
      init_s_iter(&it,r," "); 

      sprintf(buffer,"@b%s",r->name);

      fl_add_browser_line(bl,buffer);
      line++;

      fl_add_browser_line(func_bl,buffer);
      func_line++;

      fl_add_browser_line(mstest_item1_bl,buffer);
      mstest_item1_line++;

      fl_add_browser_line(mstest_userfdef_bl,buffer);
      mstest_userfdef_line++;

      fl_add_browser_line(diff_bl,buffer);
      diff_line++;
      if (!strcmp(diff_reg,&buffer[2]))
      {
         fl_select_browser_line(diff_bl,diff_line);
         diff_selected=1;
      }

      fl_add_browser_line(mask_bl,buffer);
      mask_line++;
      if (!strcmp(diff_mask_reg,&buffer[2]))
      {
         fl_select_browser_line(mask_bl,mask_line);
         mask_selected=1;
      }

      r->browser_index=line;
      if (r->selected) fl_select_browser_line(bl,line);
      ts=r->ts;
      ti=0;
      while(ts)
      {
         ti++;
         if (r->opened)
         {
            sprintf(buffer,"   t(%i)=%f",ti,ts->time);
            fl_add_browser_line(bl,buffer);
            line++;
            ts->browser_index=line;
            if (ts->selected) fl_select_browser_line(bl,line);
         }
         else
         {
            ts->browser_index=0; 
            ts->opened=0; 
         }
         l=ts->levels;
         li=0;
         while(l)
         {
            li++;
            if (ts->opened)
            {
               sprintf(buffer,"      dx(%i)=%f",l->ln,l->dx);
               fl_add_browser_line(bl,buffer);
               line++;
               l->browser_index=line;
               if (l->selected) fl_select_browser_line(bl,line);
            }
            else
            {
               l->browser_index=0; 
               l->opened=0; 
            }
            g=l->grids;
            gi=0;
            while(g)
            {
               gi++;
               if (l->opened)
               {
                  sprintf(buffer,"         g(%i) shape=%i",gi,g->shape[0]);
                  fl_add_browser_line(bl,buffer);
                  i=1;
                  while(i<g->dim)
                  {
                     sprintf(buffer,"x%i",g->shape[i++]);
                     fl_addto_browser_chars(bl,buffer);
                  }
                  line++;
                  g->browser_index=line;
                  if (g->selected) fl_select_browser_line(bl,line);
               }
               else
               {
                  g->browser_index=0;
               }
               g=g->next;
            }
            l=l->next;
         }
         ts=ts->next;
      }
      FREE_REG(r);
      r=r->next;
   }

   if (topline<=line)
      fl_set_browser_topline(bl,topline);

   if (func_topline<=func_line)
      fl_set_browser_topline(func_bl,func_topline);

   if (mstest_item1_topline<=mstest_item1_line)
      fl_set_browser_topline(mstest_item1_bl,func_topline);

   if (mstest_userfdef_topline<=mstest_userfdef_line)
      fl_set_browser_topline(mstest_userfdef_bl,func_topline);

   if (diff_topline<=diff_line)
      fl_set_browser_topline(diff_bl,diff_topline);

   if (mask_topline<=mask_line)
      fl_set_browser_topline(mask_bl,mask_topline);

   if (!mask_selected)
   {
      strcpy(diff_mask_reg,"<none>");
      fl_select_browser_line(mask_bl,1);
   }
      
   if (!diff_selected)
   {
      strcpy(diff_reg,"");
   }

   fl_unfreeze_form(bl->form);
   fl_unfreeze_form(diff_bl->form);
   fl_unfreeze_form(func_bl->form);
   fl_unfreeze_form(mstest_item1_bl->form);
   fl_unfreeze_form(mstest_userfdef_bl->form);

   FREE_REG_LIST;

   return;
}
 
//=============================================================================
// null call-back for certain objects
//=============================================================================
void null_cb(FL_OBJECT *obj, long obj_arg)
{
   return;
}

//=============================================================================
// save certain options, particularly annotation info, to the specified
// file (more variables will be added as the need arises)
//
/* format:

os_text:pos.x,pos.y,col_ind,font,which_canvas\nformat_str\n
os_cmap:p1.x,p1.y,p2.x,p2.y,bcol.r,bcol.g,bcol.b,bwidth,show_os_cmap
os_cmap[hsv]:p1.x,p1.y,p2.x,p2.y,bcol.r,bcol.g,bcol.b,bwidth,show_os_cmap
bbox:width,col.r,col.g,col.b
bbox[hsv]:width,col.r,col.g,col.b

*/ 
//=============================================================================
int save_options(char *file_name)
{
   FILE *stream;
   int i;

   save_gl_canvas(curr_gl_canvas);
   
   if (!(stream=fopen(file_name,"w")))
   {
      printf("save_options: unable to open file %s\n",file_name);
      return 0;
   }

   for (i=0; i<MAX_OS_TEXTS; i++)
   {
      if (os_text_list[i].format_str && strlen(os_text_list[i].format_str))
         fprintf(stream,"os_text:%f,%f,%i,%i,%i\n%s\n",os_text_list[i].pos.x,
                 os_text_list[i].pos.y,os_text_list[i].col_ind,os_text_list[i].font,
                 os_text_list[i].which_canvas,os_text_list[i].format_str);
   }

   fprintf(stream,"os_cmap:%f,%f,%f,%f,%f,%f,%f,%i,%i\n",
           os_cmap_p1_s[GL_LOCAL_VIEW].x,os_cmap_p1_s[GL_LOCAL_VIEW].y,
           os_cmap_p2_s[GL_LOCAL_VIEW].x,os_cmap_p2_s[GL_LOCAL_VIEW].y,
           os_cmap_bcol_s[GL_LOCAL_VIEW].r,os_cmap_bcol_s[GL_LOCAL_VIEW].g,
           os_cmap_bcol_s[GL_LOCAL_VIEW].b,os_cmap_bwidth_s[GL_LOCAL_VIEW],
           show_os_cmap_s[GL_LOCAL_VIEW]);

   fprintf(stream,"bbox:%i,%f,%f,%f\n",rend_opts_s[GL_LOCAL_VIEW].bbox_width,
           rend_opts_s[GL_LOCAL_VIEW].bbox_color.r,
           rend_opts_s[GL_LOCAL_VIEW].bbox_color.g,rend_opts_s[GL_LOCAL_VIEW].bbox_color.b);

   fprintf(stream,"os_cmap[hvs]:%f,%f,%f,%f,%f,%f,%f,%i,%i\n",
           os_cmap_p1_s[GL_HVS].x,os_cmap_p1_s[GL_HVS].y,
           os_cmap_p2_s[GL_HVS].x,os_cmap_p2_s[GL_HVS].y,
           os_cmap_bcol_s[GL_HVS].r,os_cmap_bcol_s[GL_HVS].g,
           os_cmap_bcol_s[GL_HVS].b,os_cmap_bwidth_s[GL_HVS],
           show_os_cmap_s[GL_HVS]);

   fprintf(stream,"bbox[hvs]:%i,%f,%f,%f\n",rend_opts_s[GL_HVS].bbox_width,
           rend_opts_s[GL_HVS].bbox_color.r,
           rend_opts_s[GL_HVS].bbox_color.g,rend_opts_s[GL_HVS].bbox_color.b);

   fclose(stream);
   return 1;
}

void replace_ann_edit_fs(const char *fs, int i)
{
   char *b;

   if (os_text_list[i].format_str) free(os_text_list[i].format_str);
   os_text_list[i].format_str=0;
   if (!(b=(char *)malloc(sizeof(char)*(strlen(fs)+10))))
   {
      printf("replace_ann_edit_fs: ERROR ... out of memory\n");
      return;
   }
   if (strlen(fs))
   {
      if (   !(os_text_list[i].format_str=(char *)malloc(sizeof(char)*(strlen(fs)+1))))
      {
         printf("replace_ann_edit_fs: ERROR ... out of memory\n");
         free(b);
         return;
      }
      strcpy(os_text_list[i].format_str,fs);
   } else fs=0;

   if(fs) sprintf(b,"%i :%s",i+1,fs); else sprintf(b,"%i :",i+1);
   fl_replace_choice(fd_DV_annotation_window->edit,i+1,b); 
   free(b);
}

void load_def_options(void)
{
   FILE *stream;
   char filename[256],*home;

   if (stream=fopen("default_opts.dvo","r"))
   {
      fclose(stream);
      printf("retrieving default options from the current directory...\n");
      if (load_options("default_opts.dvo")) printf("done\n");
   }
   else 
   {
      if (!(home=getenv("HOME")))
      {
         printf("environment variable $HOME not set ... no default options loaded\n");
         return;
      }
      strncpy(filename,home,220);
      strcat(filename,"/.DV/default_opts.dvo");
      if (!(stream=fopen(filename,"r")))
      {
         printf("attemping to create directory $HOME/.DV\n");
         if ((system("mkdir $HOME/.DV"))==-1)
         {
            printf("... failed. no default options loaded\n");
            return;
         }
         printf("copying /usr/local/lib/DV/default_opts.dvo to $HOME/.DV\n");
         if ((system("cp /usr/local/lib/DV/default_opts.dvo $HOME/.DV"))==-1)
         {
            printf("... failed. no default options loaded\n");
            return;
         }
      }
      else fclose(stream);
      printf("retrieving default options from $HOME/.DV ...\n");
      if (load_options(filename)) printf("done\n");
   }
}


int load_options(char *file_name)
{
   FILE *stream;
   char buf[1024];
   float x1,y1,x2,y2,rf,gf,bf;
   int i,r,g,b;

   save_gl_canvas(curr_gl_canvas);
   
   if (!(stream=fopen(file_name,"r")))
   {
      printf("load_options: unable to open file %s\n",file_name);
      return 0;
   }

   i=0;

   while(fgets(buf,1024,stream))
   {
      if (!(strncmp(buf,"os_text:",8)) && i<MAX_OS_TEXTS)
      {
         sscanf(&buf[8],"%f,%f,%i,%i,%i",&x1,&y1,&os_text_list[i].col_ind,&os_text_list[i].font,
                &os_text_list[i].which_canvas);
         fl_get_icm_color(os_text_list[i].col_ind,&r,&g,&b);
         os_text_list[i].col.r=(double)r/255;
         os_text_list[i].col.g=(double)g/255;
         os_text_list[i].col.b=(double)b/255;
         os_text_list[i].pos.x=x1;
         os_text_list[i].pos.y=y1;
         if (os_text_list[i].print_str) free(os_text_list[i].print_str); 
         os_text_list[i].print_str=0;
         if (!(fgets(buf,1024,stream)))
            buf[0]=0;
         else if (strlen(buf))
            buf[strlen(buf)-1]=0;
         replace_ann_edit_fs(buf,i);
         i++;
      }
      else if (!(strncmp(buf,"os_cmap:",8)))
      {
         sscanf(&buf[8],"%f,%f,%f,%f,%f,%f,%f,%i,%i\n",&x1,&y1,&x2,&y2,
                &rf,&gf,&bf,&os_cmap_bwidth,&show_os_cmap);
         os_cmap_p1_s[GL_LOCAL_VIEW].x=x1;
         os_cmap_p1_s[GL_LOCAL_VIEW].y=y1;
         os_cmap_p2_s[GL_LOCAL_VIEW].x=x2;
         os_cmap_p2_s[GL_LOCAL_VIEW].y=y2;
         os_cmap_bcol_s[GL_LOCAL_VIEW].r=rf;
         os_cmap_bcol_s[GL_LOCAL_VIEW].g=gf;
         os_cmap_bcol_s[GL_LOCAL_VIEW].b=bf;
         os_cmap_bwidth_s[GL_LOCAL_VIEW]=os_cmap_bwidth;
         show_os_cmap_s[GL_LOCAL_VIEW]=show_os_cmap;
      }
      else if (!(strncmp(buf,"bbox:",5)))
      {
         sscanf(&buf[5],"%i,%f,%f,%f\n",&rend_opts_s[GL_LOCAL_VIEW].bbox_width,&rf,&gf,&bf);
         rend_opts_s[GL_LOCAL_VIEW].bbox_color.r=rf;
         rend_opts_s[GL_LOCAL_VIEW].bbox_color.g=gf;
         rend_opts_s[GL_LOCAL_VIEW].bbox_color.b=bf;
      }
      else if (!(strncmp(buf,"os_cmap[hvs]:",13)))
      {
         sscanf(&buf[13],"%f,%f,%f,%f,%f,%f,%f,%i,%i\n",&x1,&y1,&x2,&y2,
                &rf,&gf,&bf,&os_cmap_bwidth,&show_os_cmap);
         os_cmap_p1_s[GL_HVS].x=x1;
         os_cmap_p1_s[GL_HVS].y=y1;
         os_cmap_p2_s[GL_HVS].x=x2;
         os_cmap_p2_s[GL_HVS].y=y2;
         os_cmap_bcol_s[GL_HVS].r=rf;
         os_cmap_bcol_s[GL_HVS].g=gf;
         os_cmap_bcol_s[GL_HVS].b=bf;
         os_cmap_bwidth_s[GL_HVS]=os_cmap_bwidth;
         show_os_cmap_s[GL_HVS]=show_os_cmap;
      }
      else if (!(strncmp(buf,"bbox[hvs]:",10)))
      {
         sscanf(&buf[10],"%i,%f,%f,%f\n",&rend_opts_s[GL_HVS].bbox_width,&rf,&gf,&bf);
         rend_opts_s[GL_HVS].bbox_color.r=rf;
         rend_opts_s[GL_HVS].bbox_color.g=gf;
         rend_opts_s[GL_HVS].bbox_color.b=bf;
      }
   }

   restore_gl_canvas(curr_gl_canvas);

   buf[0]=0;
   while(i<MAX_OS_TEXTS)
      replace_ann_edit_fs(buf,i++);

   i=1;

   fl_set_input(fd_DV_annotation_window->format_str,os_text_list[i].format_str);
   fl_set_choice(fd_DV_annotation_window->font,os_text_list[i].font+1);
   fl_set_object_lcol(fd_DV_annotation_window->color,os_text_list[i].col_ind);
   fl_set_icm_color(FL_FREE_COL1,255*os_cmap_bcol.r,255*os_cmap_bcol.g,255*os_cmap_bcol.b);
   fl_set_choice(fd_DV_annotation_window->border_width,os_cmap_bwidth);
   fl_set_object_lcol(fd_DV_annotation_window->border_color,FL_FREE_COL1);
   fclose(stream);
   refresh_gl_display();
   return 1;
}
//=============================================================================
// file menu
//=============================================================================
void file_menu_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;
   int item,skip;
   char *file_name;
   char name[MAX_NAME_LENGTH];
   char name2[MAX_NAME_LENGTH];
   reg *r;
   FILE *stream;

   IFL printf("in file_menu_cb:\n");

   item=fl_get_menu(obj);

   switch(item)
   {
         case 1: case 2:
         IFL printf("item 1/2 (Read SDF from file) selected\n");
         if (file_name=(char *)fl_show_fselector("Select SDF file",0,"*.sdf",0))
         {
            instr_read_sdf(file_name,item-1,0);
            create_browser_list();
         }
         break;
      case 3:
         IFL printf("item 3 (Write SDF to file) selected\n");
         LOCK_REG_LIST;
         r=reg_list;
         while(r)
         {
            if (r->selected)
            {
               strcpy(name,r->name);
               skip=0;
               if (file_name=(char *)fl_show_fselector("Enter file name",0,"*.sdf",name))
               {
                  strcpy(name2,file_name);
                  strcat(name2,".sdf");
                  if (stream=fopen(file_name,"r"))
                  {
                     fclose(stream);
                     if (!(fl_show_question("Overwrite existing file?",0))) skip=1;
                  }
                  else if (stream=fopen(name2,"r"))
                  {
                     fclose(stream);
                     if (!(fl_show_question("Overwrite existing file?",0))) skip=1;
                  }
                  if (!skip)
                  {
                     if (!(instr_write_sdf(name,file_name,0))) fl_show_message("Error saving to file",file_name,0);
                  }
               }
            }
            r=r->next;
         }
         FREE_REG_LIST;
         break;
      case 4: 
         if (file_name=(char *)fl_show_fselector("Select option file",0,"*.dvo",0))
            if (!(load_options(file_name))) fl_show_message("Error reading options:",file_name,0);
         break;
      case 5:
         if (file_name=(char *)fl_show_fselector("Enter file name",0,"*.dvo",name))
         {
            if (stream=fopen(file_name,"r"))
            {
               fclose(stream);
               if (!(fl_show_question("Overwrite existing file?",0))) break;
            }
            if (!(save_options(file_name))) fl_show_message("Error saving to file",file_name,0);
         }
         break;
      case 6:
         fl_show_message("The Data Vault","V0.3  Copyright 2000-2005",
			 "F. Pretorius, M.W.Choptuik, M.Snajdr, R.Stevenson");
         break;
      case 7:
         close_cb(fl_get_app_mainform(),0);
         break;
      default:
         printf("file_menu_cb: ERROR --- unknown menu item %i\n",item);
   }
   return;
}

//=============================================================================
// edit menu
//=============================================================================
#define OPEN_ALL 1
#define OPEN_ALL_TIMES 2
#define OPEN_ALL_LEVELS 3
#define CLOSE_ALL 4
#define CLOSE_ALL_TIMES 5
#define CLOSE_ALL_LEVELS 6
#define SELECT_ALL 7
#define SELECT_ALL_TIMES 8
#define SELECT_ALL_LEVELS 9
#define SELECT_ALL_GRIDS 10
#define DESELECT_ALL 11
#define DESELECT_ALL_TIMES 12
#define DESELECT_ALL_LEVELS 13
#define DESELECT_ALL_GRIDS 14

void edit_menu_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;
   int item,shift,p;
   s_iter it;
   grid *g;
   reg *r;

   IFL printf("in edit_menu_cb:\n");

   shift=fl_keysym_pressed(XK_Shift_L)||fl_keysym_pressed(XK_Shift_R);

   item=fl_get_menu(obj);

   LOCK_REG_LIST;
   r=reg_list;
   while(r)
   {
      LOCK_REG(r);
      g=init_s_iter(&it,r,0);
      while(g)
      {
         p=0;
         switch(item)
         {
            case OPEN_ALL_LEVELS: if (!shift || it.l->selected) { it.l->opened=1; p=1; }
            case OPEN_ALL_TIMES: if (!shift || p || it.ts->selected) { it.ts->opened=1; p=1; }
            case OPEN_ALL: if (!shift || p || r->selected) r->opened=1;
               break;
            case CLOSE_ALL: if (!shift || r->selected) { r->opened=0; p=1; }
            case CLOSE_ALL_TIMES: if (!shift || p || it.ts->selected) { it.ts->opened=0; p=1; }
            case CLOSE_ALL_LEVELS: if (!shift || p || it.l->selected) it.l->opened=0;
               break;
            case SELECT_ALL_GRIDS: if (!shift || it.l->selected) g->selected=1; 
               break;
            case SELECT_ALL_LEVELS: if (!shift || it.ts->selected) it.l->selected=1;
               break;
            case SELECT_ALL_TIMES: if (!shift || r->selected) it.ts->selected=1;
               break;
            case SELECT_ALL: r->selected=1;
               break;
            case DESELECT_ALL_GRIDS: if (!shift || it.l->selected) g->selected=0; 
               break;
            case DESELECT_ALL_LEVELS: if (!shift || it.ts->selected) it.l->selected=0;
               break;
            case DESELECT_ALL_TIMES: if (!shift || r->selected) it.ts->selected=0;
               break;
            case DESELECT_ALL: r->selected=0;
               break;
         }
         g=next_g(&it);
      }
      FREE_REG(r);
      r=r->next;
   }
   FREE_REG_LIST;

   create_browser_list();
}

//=============================================================================
// debug menu
//=============================================================================
void debug_menu_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;
   int item;

   IFL printf("in debug_menu_cb:\n");

   item=fl_get_menu(obj);

   switch(item)
   {
      case 1: 
         IFL printf("item 1 (dump all) selected\n");
         instr_dump_all();
         break;
      case 2: 
         instr_mem_usage();
         break;
      default:
         printf("debug_menu_cb: ERROR --- unknown menu item %i\n",item);
   }
   return;
}

//=============================================================================
// utility function for managing 'history' choice functions.
// the operation at this time is quite simplistic ... if the
// number of objects exceeds 32, then the history is cleared.
//=============================================================================
#define MAX_HIST 32
void add_to_hist(const char *s, FL_OBJECT *obj)
{
   int i,n,found=0;
   char *p;

   n=fl_get_choice_maxitems(obj);
   if (n>MAX_HIST)
   {
      n=1; fl_clear_choice(obj);
      fl_addto_choice(obj,"history");
   }
   for (i=2; i<=n; i++)
   {
      p=(char *)fl_get_choice_item_text(obj,i);
      if (!strcmp(p,s)) found=1;
   }
   if (!found) fl_addto_choice(obj,s);
}

//=============================================================================
// reads grid conversion parameters from input gadgets
//
// NOTE: the same mask and givec are saved to both LOCAL_VIEW and HVS
//=============================================================================
void set_gc_opts(int force)
{
   char buf[256];
   int canvas;
   
   canvas=fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1;

   if (!force) switch_gl_canvas(canvas);

   if (gc_opts.in_zscale) free(gc_opts.in_zscale);
   gc_opts.in_zscale=strdup(fl_get_input(fd_DV_sl_opts_window->scale));
   if (gc_opts.in_cmap_minz) free(gc_opts.in_cmap_minz);
   gc_opts.in_cmap_minz=strdup(fl_get_input(fd_DV_sl_opts_window->cmap_range_minz));
   if (gc_opts.in_cmap_maxz) free(gc_opts.in_cmap_maxz);
   gc_opts.in_cmap_maxz=strdup(fl_get_input(fd_DV_sl_opts_window->cmap_range_maxz));

   if (gc_opts.givec) free(gc_opts.givec);
   gc_opts.givec=gc_opts_s[GL_LOCAL_VIEW].givec=gc_opts_s[GL_HVS].givec
                =strdup(fl_get_input(fd_DV_sl_opts_window->givec));

   if ((strlen(gc_opts.in_cmap_minz)+strlen(gc_opts.in_cmap_maxz)) < 250)
   {
      sprintf(buf,"%s,%s",gc_opts.in_cmap_minz,gc_opts.in_cmap_maxz);
      add_to_hist(buf,fd_DV_sl_opts_window->cmap_hist); 
   }

   if (strlen(gc_opts.in_zscale) < 250)
      add_to_hist(gc_opts.in_zscale,fd_DV_sl_opts_window->scale_hist);

   if (strlen(gc_opts.givec) < 250)
      add_to_hist(gc_opts.givec,fd_DV_sl_opts_window->givec_hist);

   if (gc_opts.mask) free(gc_opts.mask);
   gc_opts.mask=gc_opts_s[GL_LOCAL_VIEW].mask=gc_opts_s[GL_HVS].mask
               =strdup(fl_get_input(fd_DV_sl_opts_window->mask));
   gc_opts.mask_val=gc_opts_s[GL_LOCAL_VIEW].mask_val=gc_opts_s[GL_HVS].mask_val
                    =atof(fl_get_input(fd_DV_sl_opts_window->mask_val));

   if (gc_opts.in_zmin_clip) free(gc_opts.in_zmin_clip);
   gc_opts.in_zmin_clip=strdup(fl_get_input(fd_DV_sl_opts_window->zmin_clip));
   if (gc_opts.in_zmax_clip) free(gc_opts.in_zmax_clip);
   gc_opts.in_zmax_clip=strdup(fl_get_input(fd_DV_sl_opts_window->zmax_clip));
   gc_opts.cmap_type=fl_get_choice(fd_DV_sl_opts_window->cmap_type);
   gc_opts.cmap_scheme=fl_get_choice(fd_DV_sl_opts_window->cmap_scheme);

   if (!force) save_gl_canvas(canvas); else save_gl_canvas(curr_gl_canvas);
}
   
void options_cb(FL_OBJECT *obj, long obj_arg)
{
   fl_show_form(fd_DV_sl_opts_window->DV_sl_opts_window,FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV local view options");
   fl_set_form_minsize(fd_DV_sl_opts_window->DV_sl_opts_window,200,100);
   XRaiseWindow(fl_display,fd_DV_sl_opts_window->DV_sl_opts_window->window);
}

//=============================================================================
// datavault browser
//
// 'single-select' mode when clicked with middle/right mouse button
//
// Currently, clicking with the right mouse button on a time/level/grid 
// also sends the 'closest' selected grid in the hierarchy to accept_sdf. 
//=============================================================================
#define DBL_CLICK_TIME 300
void dv_browser_cb(FL_OBJECT *obj, long arg)
{
   int ltrace=0,ss_mode;
   reg *r,*rb_r,*ls_r;
   time_str *ts,*rb_ts,*ls_ts;
   level *l;
   grid *g,*rb_g=0,*ls_g=0;
   int line,ti,li,gi,i,lline,mbutton,ct,dbl_click,surpress_reselect;
   long sec,usec;
   FL_OBJECT *bl=fd_DV_main_window->dv_browser;
   static int pt=0,pline=-1;
   s_iter it;
   char *rb_name;

   mbutton=fl_mouse_button();
   if (mbutton==FL_LEFT_MOUSE) ss_mode=0; else ss_mode=1;

   fl_gettime(&sec,&usec);
   lline=abs(fl_get_browser(bl));
   // the following line counteracts the supposed Xforms 'single click bug'
   surpress_reselect=0;
   if (!(fl_isselected_browser_line(bl,lline)) && ss_mode==1) surpress_reselect=1;
   ct=usec/1000+sec*1000;
   if ((ct-pt)<DBL_CLICK_TIME && pline==lline) dbl_click=1; else dbl_click=0;
   pt=ct;
   pline=lline;

   IFL printf("in dv_browser_cb, arg=%i, lline=%i, dbl_click=%i, ct=%i:\n",arg,lline,dbl_click,ct);

   LOCK_REG_LIST;

   r=reg_list;

   line=0;

   if (ss_mode) fl_freeze_form(bl->form);

   while(r)
   {
      LOCK_REG(r);
      // if (ss_mode) we must also de-select items that are unopened,
      // and those won't get traversed in later loops; so de-select here:
      if (ss_mode) 
      {
         g=init_s_iter(&it,r,0);
         r->selected=0;
         while(g)
         {
            g->selected=0;
            it.ts->selected=0;
            it.l->selected=0;
            g=next_g(&it);
         }
      }
      if ((line=r->browser_index)>0)
      {
         if (!ss_mode) r->selected=fl_isselected_browser_line(bl,line);
         else
         {
            if (line==lline && !surpress_reselect)
            {
               r->selected=1;
               fl_select_browser_line(bl,line);
            }
            else
            {
               r->selected=0;
               fl_deselect_browser_line(bl,line);
            }
         }
         if (r->opened)
         {
            ts=r->ts;
            while(ts)
            {
               if ((line=ts->browser_index)>0)
               {
                  if (!ss_mode) ts->selected=fl_isselected_browser_line(bl,line);
                  else
                  {
                     if (line==lline  && !surpress_reselect)
                     {
                        ts->selected=1;
                        fl_select_browser_line(bl,line);
                        if (mbutton==FL_RIGHT_MOUSE) 
                        {
                           rb_r=r; rb_ts=ts; rb_g=ts->levels->grids;
                        }   
                        ls_r=r; ls_ts=ts; ls_g=ts->levels->grids;
                     }
                     else
                     {
                        ts->selected=0;
                        fl_deselect_browser_line(bl,line);
                     }
                  }
                  if (ts->opened)
                  {
                     l=ts->levels;
                     while(l)
                     {
                        if ((line=l->browser_index)>0)
                        {
                           if (!ss_mode) l->selected=fl_isselected_browser_line(bl,line);
                           else
                           {
                              if (line==lline  && !surpress_reselect)
                              {
                                 l->selected=1;
                                 fl_select_browser_line(bl,line);
                                 if (mbutton==FL_RIGHT_MOUSE) 
                                 {
                                    rb_r=r; rb_ts=ts; rb_g=l->grids;
                                 }   
                                 ls_r=r; ls_ts=ts; ls_g=l->grids;
                              }
                              else
                              {
                                 l->selected=0;
                                 fl_deselect_browser_line(bl,line);
                              }
                           }
                           if (l->opened)
                           {
                              g=l->grids;
                              while(g)
                              {
                                 if ((line=g->browser_index)>0)
                                 {
                                    if (!ss_mode) g->selected=fl_isselected_browser_line(bl,line);
                                    else
                                    {
                                       if (line==lline  && !surpress_reselect)
                                       {
                                          g->selected=1;
                                          fl_select_browser_line(bl,line);
                                          if (mbutton==FL_RIGHT_MOUSE) 
                                          {
                                             rb_r=r; rb_ts=ts; rb_g=g;
                                          }   
                                          ls_r=r; ls_ts=ts; ls_g=g;
                                       }
                                       else
                                       {
                                          g->selected=0;
                                          fl_deselect_browser_line(bl,line);
                                       }
                                    }
                                 }
                                 g=g->next;
                              }
                           }
                           if (dbl_click && l->browser_index==lline)
                              l->opened=(l->opened ? 0 : 1);
                        }
                        l=l->next;
                     }
                  }
                  if (dbl_click && ts->browser_index==lline)
                     ts->opened=(ts->opened ? 0 : 1);
               }
               ts=ts->next;
            }
         }
         if (dbl_click && r->browser_index==lline)
            r->opened=(r->opened ? 0 : 1);
      }
      FREE_REG(r);
      r=r->next;
   }
  
   if (ls_g && db_open) db_new_grid(ls_r,ls_ts,ls_g);
   if (rb_g)
   {
      switch(rb_send_to)
      {
         case RB_ACCEPT_SDF: 
            send_accept_sdf_1g(rb_r->name,rb_ts->time,rb_g);
            break;
         case RB_GL_CANVAS: 
            if (!gl_canvas_open[GL_LOCAL_VIEW]) open_gl_canvas();
            else XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
            rb_name=rb_r->name;
            break;
         default:
            break;
      }
   }

   if (ss_mode) fl_unfreeze_form(bl->form);

   FREE_REG_LIST;

   if (rb_g && RB_GL_CANVAS==rb_send_to)
   {
      set_gc_opts(0);
      if (gc_opts.concat_grids!=CONCAT_NONE) send_gl_canvas(1,0,0,0,1);
      else send_gl_canvas(1,rb_g,rb_name,0,1);
      set_hvs_thumbwheel();
   }

   if (dbl_click) create_browser_list();

   return;
}

void browser_up_cb(FL_OBJECT *obj, long arg)
{
   reg *r,*rdb;
   level *l;
   time_str *ts,*tsdb;
   grid *g,*gdb;
   s_iter it;
   int sel;
   int num_gsel;
   FL_OBJECT *bl=fd_DV_main_window->dv_browser;

   LOCK_REG_LIST;
   r=reg_list; if (!r) { FREE_REG_LIST; return; }

   fl_freeze_form(bl->form);

   num_gsel=0;
   sel=r->selected;
   while(r->next) 
   {
      r->selected=r->next->selected;
      r=r->next;
   }
   r->selected=sel;

   r=reg_list;
   while(r)
   {
      if (r->browser_index && r->selected) 
         fl_select_browser_line(bl,r->browser_index);
      else if (r->browser_index && !r->selected) 
         fl_deselect_browser_line(bl,r->browser_index);
      LOCK_REG(r);
      ts=r->ts;
      sel=ts->selected;
      while(ts->next) 
      {
         ts->selected=ts->next->selected;
         ts=ts->next;
      }
      ts->selected=sel;
      ts=r->ts;
      while(ts)
      {
         if (ts->browser_index && ts->selected) 
         {
            fl_select_browser_line(bl,ts->browser_index);
            num_gsel++;
            tsdb=ts; rdb=r; gdb=ts->levels->grids;
         }
         else if (ts->browser_index && !ts->selected) 
            fl_deselect_browser_line(bl,ts->browser_index);
         l=ts->levels;
         sel=l->selected;
         while(l->next) 
         {
            l->selected=l->next->selected;
            l=l->next;
         }
         l->selected=sel;
         l=ts->levels;
         while(l)
         {
            if (l->browser_index && l->selected) 
            {
               fl_select_browser_line(bl,l->browser_index);
               num_gsel++;
               tsdb=ts; rdb=r; gdb=l->grids;
            }
            else if (l->browser_index && !l->selected) 
               fl_deselect_browser_line(bl,l->browser_index);
            g=l->grids;
            sel=g->selected;
            while(g->next) 
            {
               g->selected=g->next->selected;
               g=g->next;
            }
            g->selected=sel;
            g=l->grids;
            while(g)
            {
               if (g->browser_index && g->selected) 
               {
                  fl_select_browser_line(bl,g->browser_index);
                  num_gsel++;
                  tsdb=ts; rdb=r; gdb=g;
               }
               else if (g->browser_index && !g->selected) 
                  fl_deselect_browser_line(bl,g->browser_index);
               g=g->next;
            }
            l=l->next;
         }
         ts=ts->next;
      }
      FREE_REG(r);
      r=r->next;
   }
   FREE_REG_LIST;

   fl_unfreeze_form(bl->form);
   if (num_gsel==1 && db_open) db_new_grid(rdb,tsdb,gdb);

   if (!gl_canvas_open[GL_LOCAL_VIEW]) open_gl_canvas();
   else XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
   set_gc_opts(0);
   send_gl_canvas(1,0,0,0,1);
}

void browser_down_cb(FL_OBJECT *obj, long arg)
{
   reg *r,*rdb;
   level *l;
   time_str *ts,*tsdb;
   grid *g,*gdb;
   s_iter it;
   int sel,num_gsel;
   FL_OBJECT *bl=fd_DV_main_window->dv_browser;

   fl_freeze_form(bl->form);

   num_gsel=0;

   LOCK_REG_LIST;
   r=reg_list; if (!r) { FREE_REG_LIST; return; }
   while(r->next) r=r->next;
   sel=r->selected;
   while(r->prev) 
   {
      r->selected=r->prev->selected;
      r=r->prev;
   }
   r->selected=sel;

   r=reg_list;
   while(r)
   {
      if (r->browser_index && r->selected)
         fl_select_browser_line(bl,r->browser_index);
      else if (r->browser_index && !r->selected)
         fl_deselect_browser_line(bl,r->browser_index);
      LOCK_REG(r);
      ts=r->ts;
      while(ts->next) ts=ts->next;
      sel=ts->selected;
      while(ts->prev) 
      {
         ts->selected=ts->prev->selected;
         ts=ts->prev;
      }
      ts->selected=sel;
      while(ts)
      {
         if (ts->browser_index && ts->selected)
         {
            fl_select_browser_line(bl,ts->browser_index);
            num_gsel++;
            tsdb=ts; rdb=r; gdb=ts->levels->grids;
         }
         else if (ts->browser_index && !ts->selected)
            fl_deselect_browser_line(bl,ts->browser_index);
         l=ts->levels;
         while(l->next) l=l->next;
         sel=l->selected;
         while(l->prev) 
         {
            l->selected=l->prev->selected;
            l=l->prev;
         }
         l->selected=sel;
         while(l)
         {
            if (l->browser_index && l->selected)
            {
               fl_select_browser_line(bl,l->browser_index);
               num_gsel++;
               tsdb=ts; rdb=r; gdb=l->grids;
            }
            else if (l->browser_index && !l->selected)
               fl_deselect_browser_line(bl,l->browser_index);
            g=l->grids;
            while(g->next) g=g->next;
            sel=g->selected;
            while(g->prev) 
            {
               g->selected=g->prev->selected;
               g=g->prev;
            }
            g->selected=sel;
            while(g)
            {
               if (g->browser_index && g->selected)
               {
                  fl_select_browser_line(bl,g->browser_index);
                  num_gsel++;
                  tsdb=ts; rdb=r; gdb=g;
               }
               else if (g->browser_index && !g->selected)
                  fl_deselect_browser_line(bl,g->browser_index);
               g=g->next;
            }
            l=l->next;
         }
         ts=ts->next;
      }
      FREE_REG(r);
      r=r->next;
   }
   FREE_REG_LIST;

   fl_unfreeze_form(bl->form);
   if (num_gsel==1 && db_open) db_new_grid(rdb,tsdb,gdb);

   if (!gl_canvas_open[GL_LOCAL_VIEW]) open_gl_canvas();
   else XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
   set_gc_opts(0);
   send_gl_canvas(1,0,0,0,1);
}

//=============================================================================
// View
//=============================================================================
void view_menu_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;
   int item;
   int mode;

   IFL printf("in options_menu_cb:\n");

   item=fl_get_menu(obj);

   switch(item)
   {
      case 1: 
         IFL printf("item 1 (refresh view) selected\n");
         create_browser_list();
         break;
      case 2: 
         IFL printf("item 2 (auto-refresh) selected\n");
         if (auto_refresh)
         {
            auto_refresh=0;
            mode=FL_PUP_RADIO;
         }
         else
         {
            auto_refresh=1;
            mode=FL_PUP_RADIO | FL_PUP_CHECK;
         }
            
         fl_set_menu_item_mode(obj,2,mode);
         create_browser_list();
         break;
      case 3: 
         IFL printf("item 3 (raise all windows) selected\n");
         if (fd_DV_diff_window->DV_diff_window->window) 
            XRaiseWindow(fl_display,fd_DV_diff_window->DV_diff_window->window);
         if (fd_DV_capture_window->DV_capture_window->window) 
            XRaiseWindow(fl_display,fd_DV_capture_window->DV_capture_window->window);
         if (fd_DV_f_select_window->DV_f_select_window->window) 
            XRaiseWindow(fl_display,fd_DV_f_select_window->DV_f_select_window->window);
         if (fd_DV_annotation_window->DV_annotation_window->window)
            XRaiseWindow(fl_display,fd_DV_annotation_window->DV_annotation_window->window);
         if (fd_DV_sl_opts_window->DV_sl_opts_window->window)
            XRaiseWindow(fl_display,fd_DV_sl_opts_window->DV_sl_opts_window->window);
         if (fd_DV_func_window->DV_func_window->window)
            XRaiseWindow(fl_display,fd_DV_func_window->DV_func_window->window);
         if (fd_DV_dbrowser_window->DV_dbrowser_window->window)
            XRaiseWindow(fl_display,fd_DV_dbrowser_window->DV_dbrowser_window->window);
         if (fd_DV_hvs_window->DV_hvs_window->window)
            XRaiseWindow(fl_display,fd_DV_hvs_window->DV_hvs_window->window);
         if (fd_DV_gl_canvas_window->DV_gl_canvas_window->window)
            XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
         if (fd_DV_mstest_item1_window->DV_mstest_item1_window->window)
            XRaiseWindow(fl_display,fd_DV_mstest_item1_window->DV_mstest_item1_window->window);
         if (fd_DV_mstest_userfdef_window->DV_mstest_userfdef_window->window)
            XRaiseWindow(fl_display,fd_DV_mstest_userfdef_window->DV_mstest_userfdef_window->window);
         break;
      default:
         printf("options_menu_cb: ERROR --- unknown menu item %i\n",item);
   }
   return;
}

//=============================================================================
// help --- disabled
//=============================================================================
void help_menu_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;

   IFL printf("in help_menu_cb:\n");

   return;
}

//=============================================================================
// delete
//=============================================================================
void delete_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;
   int item;
   int mode;

   IFL printf("in delete_cb:\n");

   item=fl_get_menu(obj);

   switch(item)
   {
      case 1: 
         IFL printf("item 1 (selected and opened) selected\n");
         mode=DM_SELECTED_AND_OPENED;
         break;
      case 2: 
         IFL printf("item 2 (selected) selected\n");
         mode=DM_SELECTED;
         break;
      case 3:
         IFL printf("item 3 (all) selected\n");
         mode=DM_ALL;
         break;
      case 4:
         IFL printf("item 4 (last) selected\n");
         mode=DM_LAST_TIME_OF_SELECTED;
         break;
      default:
         printf("delete_cb: ERROR --- unknown item %i\n",item);
         return;
   }
   
   delete_grids(mode);

   create_browser_list();

   return;
}

//=============================================================================
// merge
//=============================================================================
void merge_cb(FL_OBJECT *obj, long obj_arg)
{
   int item;
   int mode;

   item=fl_get_menu(obj);

   switch(item)
   {
      case 1: 
         mode=MERGE_SELECTED;
         break;
      case 2: 
         mode=MERGE_ALL;
         break;
      default:
         printf("merge_cb: ERROR --- unknown item %i\n",item);
         return;
   }
   
   merge_reg(mode);

   create_browser_list();

   return;
}

//=============================================================================
// functions
//=============================================================================
void functions_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;

   IFL printf("in functions_cb:\n");

   fl_show_form(fd_DV_func_window->DV_func_window,FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV functions");
   fl_set_form_minsize(fd_DV_func_window->DV_func_window,500,400);
   XRaiseWindow(fl_display,fd_DV_func_window->DV_func_window->window);
}

int get_selected_func()
{
   int i,imax;

   imax=fl_get_browser_maxline(fd_DV_func_window->func);
   for (i=1; i<=imax; i++) if (fl_isselected_browser_line(fd_DV_func_window->func,i)) return i;

   printf("get_selected_func: Error, no function selected!!\n");
   return 1;
}
   
int func_build_name()
{
   int i;
   char buffer[3*MAX_NAME_LENGTH],*a,*b;

   buffer[0]=0;
   i=get_selected_func()-1;

   a=(char *)fl_get_input(fd_DV_func_window->func_reg_a);
   b=(char *)fl_get_input(fd_DV_func_window->func_reg_b);

   if ((func_table[i].func_type==TYPE_BINARY_FUNC ||  
        func_table[i].func_type==TYPE_MERGE ) && a && b)
      sprintf(buffer,func_table[i].new_func_ps,a,b);
   else if ((func_table[i].func_type==TYPE_UNARY_FUNC ||
             func_table[i].func_type==TYPE_RENAME) && a)
      sprintf(buffer,func_table[i].new_func_ps,a);

   buffer[MAX_NAME_LENGTH-1]=0;
   fl_set_input(fd_DV_func_window->func_new_reg,buffer);

   return i;
}

void func_browser_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;
   int mbutton,line;
   char *name;

   IFL printf("in func_browser_cb:\n");

   line=fl_get_browser(obj);
   name=&((char *)fl_get_browser_line(obj,line))[2];

   mbutton=fl_mouse_button();
   if (mbutton==FL_LEFT_MOUSE)
      fl_set_input(fd_DV_func_window->func_reg_a,name);
   else if (mbutton==FL_MIDDLE_MOUSE)
      fl_set_input(fd_DV_func_window->func_reg_b,name);
   else
      fl_set_input(fd_DV_func_window->func_mask,name);

   func_build_name();
}

void func_reg_a_cb(FL_OBJECT *obj, long obj_arg)
{
   func_build_name();
}

void func_reg_b_cb(FL_OBJECT *obj, long obj_arg)
{
   func_build_name();
}

void func_filter_hist_cb(FL_OBJECT *obj, long obj_arg)
{
   if (fl_get_choice(obj)==1) return;
   fl_set_input(fd_DV_func_window->givec,fl_get_choice_item_text(obj,fl_get_choice(obj)));
   fl_set_choice(obj,1);
}

void func_arg_hist_cb(FL_OBJECT *obj, long obj_arg)
{
   if (fl_get_choice(obj)==1) return;
   fl_set_input(fd_DV_func_window->func_args,fl_get_choice_item_text(obj,fl_get_choice(obj)));
   fl_set_choice(obj,1);
}

void func_go_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0,ret,i;
   char *t1="__DV_Gui_temporary_register: t1";
   reg *r;
   char *mask,*a,*b,*args,*new_reg,*givec;
   double mask_val;

   IFL printf("in func_go_cb:\n");

   i=get_selected_func()-1;

   if (func_table[i].func_type==TYPE_NULL_FUNC) return;

   args=(char *)fl_get_input(fd_DV_func_window->func_args);
   if (func_table[i].func_type==TYPE_SPECIAL)
   {
      ((SPECIAL_FNC)func_table[i].func)(args);
      create_browser_list();
      return;
   }
   a=(char *)fl_get_input(fd_DV_func_window->func_reg_a);
   b=(char *)fl_get_input(fd_DV_func_window->func_reg_b);
   mask=(char *)fl_get_input(fd_DV_func_window->func_mask);
   mask_val=atof(fl_get_input(fd_DV_func_window->func_mask_val));
   new_reg=(char *)fl_get_input(fd_DV_func_window->func_new_reg);
   if (fl_get_button(fd_DV_func_window->apply_filter))
      givec=(char *)fl_get_input(fd_DV_func_window->givec);
   else givec=0;

   if (strlen(new_reg)>=MAX_NAME_LENGTH)
   {
      fl_show_messages("New register name is longer than the maximum allowed name");
      return;
   }

   if (!(r=find_reg(a,1,1)))
   {
      fl_show_messages("Register A not found");
      return;
   }
   FREE_REG(r);

   // for effeciency, treat rename separately
   if (func_table[i].func_type==TYPE_RENAME)
   {
      rename_reg(a,new_reg);
      create_browser_list();
      return;
   }


   // for effeciency, treat delete separately
   if (func_table[i].func_type==TYPE_DELETE)
   {
      delete_reg(a);
      create_browser_list();
      return;
   }


   if (strlen(mask))
   {
      if (!(r=find_reg(mask,1,1)))
      {
         fl_show_messages("Mask register not found");
         return;
      }
      FREE_REG(r);
   }

   if (!fl_get_button(fd_DV_func_window->func_overwrite) && (r=find_reg(new_reg,1,1)))
   {
      FREE_REG(r);
      if (!(fl_show_question("overwrite existing register?",0))) return;
   }

   //--------------------------------------------------------------------------
   // use a temporary register in case the new register is the same as the old
   //--------------------------------------------------------------------------
   delete_reg(t1);
   if (func_table[i].func_type==TYPE_UNARY_FUNC)
   {
      if (!(apply_unary_gf((UNARY_FNC)func_table[i].func,args,a,t1,mask,mask_val,givec)))
      {
         fl_show_messages("apply_unary_gf() failed --- see DV console output");
         delete_reg(t1);
         return;
      }
   }
   else
   {
      if (!(r=find_reg(b,1,1)))
      {
         fl_show_messages("Register B not found");
         return;
      }
      FREE_REG(r);
      if (func_table[i].func_type==TYPE_MERGE)
      {
         if (!(merge_gf(a,b,t1,givec)))
         {
            fl_show_messages("merge_gf() failed --- see DV console output");
            delete_reg(t1);
            return;
         }
      }
      else if (!(apply_binary_gf((BINARY_FNC)func_table[i].func,args,a,b,t1,mask,mask_val,givec)))
      {
         fl_show_messages("apply_binary_gf() failed --- see DV console output");
         delete_reg(t1);
         return;
      }
   }

   givec=(char *)fl_get_input(fd_DV_func_window->givec);
   if (strlen(givec) < 250) add_to_hist(givec,fd_DV_func_window->filter_hist);
   if (strlen(args) < 250) add_to_hist(args,fd_DV_func_window->arg_hist);

   delete_reg(new_reg);
   rename_reg(t1,new_reg);
   create_browser_list();
}

void func_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;
   int i;

   IFL printf("in func_cb:\n");

   i=func_build_name();
   fl_set_object_label(fd_DV_func_window->func_info,func_table[i].info);
}

//=============================================================================

//=============================================================================
// differentiate
//=============================================================================
void diff_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;

   IFL printf("in diff_cb:\n");

   fl_show_form(fd_DV_diff_window->DV_diff_window,FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV differentiate");
   fl_set_form_minsize(fd_DV_diff_window->DV_diff_window,560,220);
   XRaiseWindow(fl_display,fd_DV_diff_window->DV_diff_window->window);
}

void diff_browser_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0,found_choice=0;
   int line,i;
   char *sel;
   reg *r;
   FL_OBJECT *wrt;
   char *def_coord[4]={"t","x1","x2","x3"};
   char msg[MAX_NAME_LENGTH+30];
   char *name;

   IFL printf("in diff_browser_cb:\n");

   wrt=fd_DV_diff_window->diff_wrt;

   line=fl_get_browser(obj);

   name=(char *)fl_get_browser_line(obj,line);
   strcpy(diff_reg,&name[2]);
   if (!(r=find_reg(diff_reg,1,1)))
   {
      sprintf(msg,"Register %s not found!",diff_reg);
      fl_show_messages(msg);
      return;
   }

   fl_clear_choice(wrt);
   for (i=0;i<MAX_DIM+1;i++)
   {
      if (r->coord_names[i]) name=r->coord_names[i]; else name=def_coord[i];
      fl_addto_choice(wrt,name);
      if (!strcmp(name,diff_wrt))
      {
         found_choice=1;
         fl_set_choice(wrt,i+1);
      }
   }

   if (!found_choice)
   {
      if (r->coord_names[0]) strcpy(diff_wrt,r->coord_names[0]);
      else strcpy(diff_wrt,def_coord[0]);
      fl_set_choice(wrt,1);
   }

   if ((strlen(diff_wrt)+strlen(diff_reg)) < (MAX_NAME_LENGTH-1))
   {
      sprintf(diff_new_reg,"%s_%s",diff_reg,diff_wrt);
      fl_set_input(fd_DV_diff_window->diff_new_reg,diff_new_reg);
   }

   FREE_REG(r);
   return;
}

void diff_mask_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0,line;
   char *name;

   IFL printf("in diff_mask_cb:\n");

   name=(char *)fl_get_browser_line(obj,line=fl_get_browser(obj));
   if (name && line>1) strcpy(diff_mask_reg,&name[2]);
   else strcpy(diff_mask_reg,"<none>");
}

void diff_go_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0,ret,x;
   reg *r;
   char *t1="__DV_Gui_temporary_register: t1";
   char *t2="__DV_Gui_temporary_register: t2";
   char *cmask,*new_reg,*filter;

   IFL printf("in diff_go_cb:\n");

   if (!strlen(diff_reg)) return;

   new_reg=(char *)fl_get_input(fd_DV_diff_window->diff_new_reg);
   filter=(char *)fl_get_input(fd_DV_diff_window->diff_filter);
   if (strlen(new_reg)>=MAX_NAME_LENGTH)
   {
      fl_show_messages("New register name is longer than the maximum allowed name");
      return;
   }

   strcpy(diff_new_reg,new_reg);
   if (strlen(diff_new_reg)==0) return;

   diff_mask_val=atof(fl_get_input(fd_DV_diff_window->diff_mask_val));

   if (!fl_get_button(fd_DV_diff_window->diff_overwrite) && (r=find_reg(diff_new_reg,1,1)))
   {
      FREE_REG(r);
      if (!(fl_show_question("overwrite existing register?",0))) return;
   }

   if (strlen(diff_mask_reg)==0 || !strcmp(diff_mask_reg,"<none>"))
      cmask=0;  
   else
      cmask=diff_mask_reg;

   //--------------------------------------------------------------------------
   // use a temporary register in case the new register is the same as the old
   //--------------------------------------------------------------------------
   delete_reg(t1);
   if (diff_wrt_ind==1)
   {
      if (cmask)
      {
         ret=apply_unary_gf(gf_clone,"",diff_reg,t1,0,0,filter);
         ret=apply_unary_gf(gf_clone,"",cmask,t2,0,0,filter);
         ret=t_diff(t1,t2,diff_mask_val);
         delete_reg(t2);
      }
      else
      {
         ret=apply_unary_gf(gf_clone,"",diff_reg,t1,0,0,filter);
         ret=t_diff(t1,0,diff_mask_val);
      }
   }
   else
   {
      if (fl_get_choice(fd_DV_diff_window->diff_stencil)==1)
         ret=apply_unary_gf(s_diff,diff_wrt,diff_reg,t1,cmask,diff_mask_val,filter);
      else
         ret=apply_unary_gf(s_diff_CN,diff_wrt,diff_reg,t1,cmask,diff_mask_val,filter);
   }

   if (!ret)
   {
      delete_reg(t1);
      fl_show_messages("diff() failed --- see DV console output");
   }
   else
   {
      delete_reg(diff_new_reg);
      rename_reg(t1,diff_new_reg);
   }

   create_browser_list();
}

void diff_wrt_cb(FL_OBJECT *obj, long obj_arg)
{
   int ltrace=0;

   IFL printf("in diff_wrt_cb:\n");

   if (!(diff_wrt_ind=fl_get_choice(obj))) return;
   strcpy(diff_wrt,fl_get_choice_item_text(obj,diff_wrt_ind));

   if ((strlen(diff_wrt)+strlen(diff_reg)) < (MAX_NAME_LENGTH-1))
   {
      sprintf(diff_new_reg,"%s_%s",diff_reg,diff_wrt);
      fl_set_input(fd_DV_diff_window->diff_new_reg,diff_new_reg);
   }
}

//=============================================================================
// sends selected grids to bbh_xyz
//=============================================================================
void send_to_acceptXYZ_cb(FL_OBJECT *obj, long arg)
{
   int ltrace=0;
   int item;

   IFL printf("in send_to_acceptXYZ_cb:\n");

   // item=fl_get_menu(obj); --- used to be a menu, never used option 1
   item=2; 

   switch(item)
   {
      case 1: 
         IFL printf("item 1 (Selected Grids) selected\n");
         send_bbh_xyz(0);
         break;
      case 2:
         IFL printf("item 2 (Grids of All Selected) selected\n");
         send_bbh_xyz(1);
         break;
      default:
         printf("send_to_acceptXYZ_cb: ERROR --- unknown item %i\n",item);
         return;
   }
   
   return;
}

//=============================================================================
// sends selected grids to AcceptSDF
//=============================================================================
void send_to_acceptSDF_cb(FL_OBJECT *obj, long arg)
{
   int ltrace=0;
   int item;

   IFL printf("in send_to_acceptSDF_cb:\n");

   // item=fl_get_menu(obj); --- used to be a menu, never used option 1
   item=2; 

   switch(item)
   {
      case 1: 
         IFL printf("item 1 (Selected Grids) selected\n");
         send_accept_sdf(0);
         break;
      case 2:
         IFL printf("item 2 (Grids of All Selected) selected\n");
         send_accept_sdf(1);
         break;
      default:
         printf("send_to_acceptSDF_cb: ERROR --- unknown item %i\n",item);
         return;
   }
   
   return;
}

//=============================================================================
// sends selected grids to gl_canvas form
//=============================================================================
void send_to_local_cb(FL_OBJECT *obj, long arg)
{
   int ltrace=0;
   int item;

   IFL printf("in send_to_local_cb:\n");
   
   if (!gl_canvas_open[GL_LOCAL_VIEW]) open_gl_canvas();
   else XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);

   set_gc_opts(0);

   // item=fl_get_menu(obj); --- used to be a menu, never used option 1
   item=2; 
   
   switch(item)
   {
      case 1: 
         IFL printf("item 1 (Selected Grids) selected\n");
         send_gl_canvas(0,0,0,0,1);
         break;
      case 2:
         IFL printf("item 2 (Grids of All Selected) selected\n");
         send_gl_canvas(1,0,0,0,1);
         break;
      default:
         printf("send_to_local_cb: ERROR --- unknown item %i\n",item);
         return;
   }

   set_hvs_thumbwheel();
   
   return;
}

//=============================================================================
// canvas and related cb's
//=============================================================================
void sl_opts_which_canvas_cb(FL_OBJECT *obj, long arg)
{
   char buf[128];
   static int prev_canvas=GL_LOCAL_VIEW;

   switch_gl_canvas(prev_canvas);
   set_gc_opts(1);

   prev_canvas=fl_get_choice(obj)-1;
   switch_gl_canvas(prev_canvas);

   fl_set_input(fd_DV_sl_opts_window->scale,gc_opts.in_zscale);
   fl_set_input(fd_DV_sl_opts_window->cmap_range_minz,gc_opts.in_cmap_minz);
   fl_set_input(fd_DV_sl_opts_window->cmap_range_maxz,gc_opts.in_cmap_maxz);
   fl_set_input(fd_DV_sl_opts_window->givec,gc_opts.givec);
   fl_set_input(fd_DV_sl_opts_window->mask,gc_opts.mask);
   sprintf(buf,"%lf",gc_opts.mask_val);
   fl_set_input(fd_DV_sl_opts_window->mask_val,buf);
   fl_set_input(fd_DV_sl_opts_window->zmin_clip,gc_opts.in_zmin_clip);
   fl_set_input(fd_DV_sl_opts_window->zmax_clip,gc_opts.in_zmax_clip);
   fl_set_choice(fd_DV_sl_opts_window->cmap_type,gc_opts.cmap_type);
   fl_set_choice(fd_DV_sl_opts_window->cmap_scheme,gc_opts.cmap_scheme);
   fl_set_choice(fd_DV_sl_opts_window->coarsen_step,gc_opts.coarsen_step);
   fl_set_choice(fd_DV_sl_opts_window->sync_t,gc_opts.sync_t);
   fl_set_choice(fd_DV_sl_opts_window->concat_grids,gc_opts.concat_grids);
   fl_set_button(fd_DV_sl_opts_window->use_mask,gc_opts.use_mask);
   fl_set_button(fd_DV_sl_opts_window->use_givec,gc_opts.use_givec);
   fl_set_button(fd_DV_sl_opts_window->monotone,gc_opts.monotone);
   fl_set_button(fd_DV_sl_opts_window->levelcolor,gc_opts.levelcolor);
   fl_set_button(fd_DV_sl_opts_window->amr,gc_opts.amr);
   fl_set_button(fd_DV_sl_opts_window->ignore_ccoords,gc_opts.ignore_ccoords);
   fl_set_button(fd_DV_sl_opts_window->bbox_only,gc_opts.bbox_only);
   fl_set_button(fd_DV_sl_opts_window->color_clip,gc_opts.color_clip);
   fl_set_icm_color(FL_FREE_COL1,255*gc_opts.fg_color.r,255*gc_opts.fg_color.g,255*gc_opts.fg_color.b);
   fl_set_object_lcol(fd_DV_sl_opts_window->fg_color,FL_FREE_COL1);
}
   
void sl_opts_concat_grids_cb(FL_OBJECT *obj, long arg)
{
   int i;

   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   i=fl_get_choice(obj);
   switch(i)
   {
      case 1: gc_opts.concat_grids=gc_opts_s[GL_LOCAL_VIEW].concat_grids=
              gc_opts_s[GL_HVS].concat_grids=CONCAT_NONE; break;
      case 2: gc_opts.concat_grids=gc_opts_s[GL_LOCAL_VIEW].concat_grids=
              gc_opts_s[GL_HVS].concat_grids=CONCAT_TIME; break;
      case 3: gc_opts.concat_grids=gc_opts_s[GL_LOCAL_VIEW].concat_grids=
              gc_opts_s[GL_HVS].concat_grids=CONCAT_ALL; break;
   }
}

void sl_opts_monotone_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (gc_opts.monotone) gc_opts.monotone=0; else gc_opts.monotone=1;
}

void sl_opts_cmap_scheme_cb(FL_OBJECT *obj, long arg)
{
   gc_opts.cmap_scheme=fl_get_choice(fd_DV_sl_opts_window->cmap_scheme);
   build_cmap();
}

void sl_3D_bbox_only_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (gc_opts.bbox_only) gc_opts.bbox_only=0; else gc_opts.bbox_only=1;
}

void sl_color_clip_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (gc_opts.color_clip) gc_opts.color_clip=0; else gc_opts.color_clip=1;
}

void sl_opts_levelcolor_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (gc_opts.levelcolor) gc_opts.levelcolor=0; else gc_opts.levelcolor=1;
}

void sl_opts_fg_color_cb(FL_OBJECT *obj, long arg)
{
   static int i=2,r,g,b;

   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   i=fl_show_colormap(i);
   fl_get_icm_color(i,&r,&g,&b);
   gc_opts.fg_color.r=(double)r/255;
   gc_opts.fg_color.g=(double)g/255;
   gc_opts.fg_color.b=(double)b/255;
   fl_set_object_lcol(obj,i);
}


void sl_opts_sync_cb(FL_OBJECT *obj, long arg)
{
   int i;

   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   i=fl_get_choice(obj);
   switch(i)
   {
      case 1: gc_opts.sync_t=gc_opts_s[GL_LOCAL_VIEW].sync_t=
              gc_opts_s[GL_HVS].sync_t=SYNC_T_INDEX; break;
      case 2: gc_opts.sync_t=gc_opts_s[GL_LOCAL_VIEW].sync_t=
              gc_opts_s[GL_HVS].sync_t=SYNC_T_VALUE; break;
   }
}

void sl_opts_amr_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (gc_opts.amr) gc_opts_s[GL_LOCAL_VIEW].amr=gc_opts_s[GL_HVS].amr=gc_opts.amr=0; 
   else gc_opts_s[GL_LOCAL_VIEW].amr=gc_opts_s[GL_HVS].amr=gc_opts.amr=1; 
}

void sl_opts_ignore_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (gc_opts.ignore_ccoords) gc_opts.ignore_ccoords=0; else gc_opts.ignore_ccoords=1;
}

void sl_opts_use_mask_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (gc_opts.use_mask) gc_opts_s[GL_LOCAL_VIEW].use_mask=gc_opts_s[GL_HVS].use_mask=gc_opts.use_mask=0; 
   else gc_opts_s[GL_LOCAL_VIEW].use_mask=gc_opts_s[GL_HVS].use_mask=gc_opts.use_mask=1; 
}

void sl_opts_use_givec_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (gc_opts.use_givec) gc_opts_s[GL_LOCAL_VIEW].use_givec=gc_opts_s[GL_HVS].use_givec=gc_opts.use_givec=0; 
   else gc_opts_s[GL_LOCAL_VIEW].use_givec=gc_opts_s[GL_HVS].use_givec= gc_opts.use_givec=1; 
}

void sl_opts_coarsen_cb(FL_OBJECT *obj, long obj_arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   gc_opts.coarsen_step=fl_get_choice(obj);
}

void sl_opts_scale_hist_cb(FL_OBJECT *obj, long obj_arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (fl_get_choice(obj)==1) return;
   fl_set_input(fd_DV_sl_opts_window->scale,fl_get_choice_item_text(obj,fl_get_choice(obj)));
   fl_set_choice(obj,1);
}

void sl_opts_cmap_hist_cb(FL_OBJECT *obj, long obj_arg)
{
   char buf[256];
   const char *p,*q;
   int i;

   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (fl_get_choice(obj)==1) return;
   p=q=fl_get_choice_item_text(obj,fl_get_choice(obj));
   i=0;
   while(p && *p && *p!=',') { p++; i++; }
   if (i && i<255)
   {
      strncpy(buf,q,i); buf[i]=0;
      fl_set_input(fd_DV_sl_opts_window->cmap_range_minz,buf);
   }
   else fl_set_input(fd_DV_sl_opts_window->cmap_range_minz,"");

   if (*p) p++; 
   if (*p) fl_set_input(fd_DV_sl_opts_window->cmap_range_maxz,p);
   else fl_set_input(fd_DV_sl_opts_window->cmap_range_maxz,"");

   fl_set_choice(obj,1);
}

void sl_opts_givec_hist_cb(FL_OBJECT *obj, long obj_arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_sl_opts_window->which_canvas)-1);
   if (fl_get_choice(obj)==1) return;
   fl_set_input(fd_DV_sl_opts_window->givec,fl_get_choice_item_text(obj,fl_get_choice(obj)));
   fl_set_choice(obj,1);
}

void show_gl_particle_f_select(void)
{
   fl_show_form(fd_DV_f_select_window->DV_f_select_window,
      FL_PLACE_CENTERFREE,FL_FULLBORDER,"Select color function.");
   fl_set_form_minsize(fd_DV_f_select_window->DV_f_select_window,100,100);
   XRaiseWindow(fl_display,fd_DV_f_select_window->DV_f_select_window->window);
}

void show_gl_screen_capture(void)
{
   fl_show_form(fd_DV_capture_window->DV_capture_window,
                FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV screen capture");
   fl_set_form_minsize(fd_DV_capture_window->DV_capture_window,100,100);
   XRaiseWindow(fl_display,fd_DV_capture_window->DV_capture_window->window);
}

void show_gl_annotation_cb(void)
{
   fl_show_form(fd_DV_annotation_window->DV_annotation_window,
                FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV annotation");
   fl_set_form_minsize(fd_DV_annotation_window->DV_annotation_window,300,200);
   XRaiseWindow(fl_display,fd_DV_annotation_window->DV_annotation_window->window);
}

void gl_options_menu_cb(FL_OBJECT *obj, long arg)
{
   int i;
   char zoom[128];
   int ltrace=0;

   switch_gl_canvas(GL_LOCAL_VIEW);

   i=fl_get_menu(obj);

   sprintf(zoom,"%f",1/cam_opts.zoom);

   switch(i)
   {
      case 1: rend_opts.render_mode=RENDER_MODE_POINT; break;
      case 2: rend_opts.render_mode=RENDER_MODE_HIDDEN_LINE; break;
      case 3: rend_opts.render_mode=RENDER_MODE_LINEX; break;
      case 4: rend_opts.render_mode=RENDER_MODE_LINEY; break;
      case 5: rend_opts.render_mode=RENDER_MODE_LINEXY; break;
      case 6: rend_opts.render_mode=RENDER_MODE_SOLID; rend_opts.smooth=0; break;
      case 7: rend_opts.render_mode=RENDER_MODE_SOLID; rend_opts.smooth=1; break;
      case 8: rend_opts.render_mode=RENDER_MODE_PARTICLES;
	      IFL printf("rend_opts.render_mode=RENDER_MODE_PARTICLES\n"); 
	      /* Clear the display lists. */
	      clear_gl_display_lists();
	      /* Initialize to colormap by z values. */
	      show_gl_particle_f_select();
	      /* User must then re-send the data. */
	      break;
      case 10: if (rend_opts.draw_bboxes)
              {
                 rend_opts.draw_bboxes=0; 
                 fl_set_menu_item_mode(obj,6,FL_PUP_RADIO); 
              }
              else
              {
                 rend_opts.draw_bboxes=1; 
                 fl_set_menu_item_mode(obj,6,FL_PUP_RADIO|FL_PUP_CHECK);
              }
              break;
      case 12: show_gl_screen_capture(); break;
      case 13: show_gl_annotation_cb(); break;
      case 15: if (rend_opts.use_key_frames)
              {
                 rend_opts.use_key_frames=0; 
                 fl_set_menu_item_mode(obj,11,FL_PUP_RADIO); 
              }
              else
              {
                 rend_opts.use_key_frames=1; 
                 fl_set_menu_item_mode(obj,11,FL_PUP_RADIO|FL_PUP_CHECK);
              }
              break;
      case 16: set_key_frame(); break;
      case 17: clear_key_frame(); break;
      case 18: clear_all_key_frames(); 
               rend_opts.use_key_frames=0;
               fl_set_menu_item_mode(obj,11,FL_PUP_RADIO);
               break;
   }
   if (i<=8) set_gl_render_mode();
   if (i<=8 || i==9) refresh_gl_display(); 
}

void hvs_options_menu_cb(FL_OBJECT *obj, long arg)
{
   int i;

   switch_gl_canvas(GL_HVS);

   i=fl_get_menu(obj);

   switch(i)
   {
      case 1: rend_opts.render_mode=RENDER_MODE_POINT; break;
      case 2: rend_opts.render_mode=RENDER_MODE_VOLUMETRIC; break;
      case 3: rend_opts.render_mode=RENDER_MODE_NONE; break;
      case 5: if (rend_opts.draw_bboxes)
              {
                 rend_opts.draw_bboxes=0; 
                 fl_set_menu_item_mode(obj,5,FL_PUP_RADIO); 
              }
              else
              {
                 rend_opts.draw_bboxes=1; 
                 fl_set_menu_item_mode(obj,5,FL_PUP_RADIO|FL_PUP_CHECK);
              }
              break;
      case 6: if (rend_opts.draw_slice_box)
              {
                 rend_opts.draw_slice_box=0; 
                 fl_set_menu_item_mode(obj,6,FL_PUP_RADIO); 
              }
              else
              {
                 rend_opts.draw_slice_box=1; 
                 fl_set_menu_item_mode(obj,6,FL_PUP_RADIO|FL_PUP_CHECK);
              }
              break;
      case 8: show_gl_screen_capture(); break;
      case 9: show_gl_annotation_cb(); break;
   }
   if (i<=3) set_gl_render_mode();
   if (i<=3 || i==5 || i==6) refresh_gl_display(); 
}

void refresh_os_text(int i)
{
   if (os_text_list[i].print_str) free(os_text_list[i].print_str);
   os_text_list[i].print_str=0;
   refresh_gl_display();
}

void gl_ann_edit_cb(FL_OBJECT *obj, long arg)
{
   int i;

   i=fl_get_choice(obj)-1;
   fl_set_input(fd_DV_annotation_window->format_str,os_text_list[i].format_str);
   fl_set_choice(fd_DV_annotation_window->font,os_text_list[i].font+1);
   fl_set_object_lcol(fd_DV_annotation_window->color,os_text_list[i].col_ind);
}

void gl_ann_font_cb(FL_OBJECT *obj, long arg)
{
   int i,f;

   i=fl_get_choice(fd_DV_annotation_window->edit)-1;
   f=fl_get_choice(fd_DV_annotation_window->font)-1;
   os_text_list[i].font=f;
   refresh_os_text(i);
}

void gl_ann_color_cb(FL_OBJECT *obj, long arg)
{
   int i,r,g,b;

   i=fl_get_choice(fd_DV_annotation_window->edit)-1;
   os_text_list[i].col_ind=fl_show_colormap(os_text_list[i].col_ind);
   fl_get_icm_color(os_text_list[i].col_ind,&r,&g,&b);
   os_text_list[i].col.r=(double)r/255;
   os_text_list[i].col.g=(double)g/255;
   os_text_list[i].col.b=(double)b/255;
   fl_set_object_lcol(obj,os_text_list[i].col_ind);
   refresh_os_text(i);
}

void gl_ann_bbox_color_cb(FL_OBJECT *obj, long arg)
{
   static int i=2,r,g,b;

   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   i=fl_show_colormap(i);
   fl_get_icm_color(i,&r,&g,&b);
   rend_opts.bbox_color.r=(double)r/255;
   rend_opts.bbox_color.g=(double)g/255;
   rend_opts.bbox_color.b=(double)b/255;
   fl_set_object_lcol(obj,i);
   refresh_gl_display(); 
}

void gl_ann_bg_color_cb(FL_OBJECT *obj, long arg)
{
   static int i=2,r,g,b;

   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   i=fl_show_colormap(i);
   fl_get_icm_color(i,&r,&g,&b);
   rend_opts.bg_color.r=(double)r/255;
   rend_opts.bg_color.g=(double)g/255;
   rend_opts.bg_color.b=(double)b/255;
   fl_set_object_lcol(obj,i);
   refresh_gl_display(); 
}

void gl_ann_anti_alias_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   if (rend_opts.anti_alias) rend_opts.anti_alias=0; else rend_opts.anti_alias=1;
   refresh_gl_display(); 
}

void gl_ann_bbox_width_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   rend_opts.bbox_width=fl_get_choice(obj);
   refresh_gl_display(); 
}

void gl_ann_1d_pt_size_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   rend_opts.pt_size_1d=fl_get_choice(obj);
   refresh_gl_display(); 
}

void gl_ann_1d_line_width_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   rend_opts.line_width_1d=fl_get_choice(obj);
   refresh_gl_display(); 
}

void gl_ann_pos_text_cb(FL_OBJECT *obj, long arg)
{
   if (gl_pos!=POS_TEXT) gl_pos=POS_TEXT; else gl_pos=POS_OFF;
   fl_set_button(fd_DV_annotation_window->pos_cmap,0);

}

void gl_ann_pos_cmap_cb(FL_OBJECT *obj, long arg)
{
   if (gl_pos!=POS_CMAP) gl_pos=POS_CMAP; else gl_pos=POS_OFF;
   fl_set_button(fd_DV_annotation_window->pos_text,0);
}

void gl_ann_disp_cmap_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   if (show_os_cmap) show_os_cmap=0; else show_os_cmap=1;
   refresh_gl_display();
}

void gl_ann_border_width_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   os_cmap_bwidth=fl_get_choice(obj);
   if (show_os_cmap) refresh_gl_display();
}

void gl_ann_border_color_cb(FL_OBJECT *obj, long arg)
{
   int i,r,g,b;

   switch_gl_canvas(fl_get_choice(fd_DV_annotation_window->which_canvas)-1);
   i=fl_show_colormap(1);
   fl_get_icm_color(i,&r,&g,&b);
   os_cmap_bcol.r=(double)r/255;
   os_cmap_bcol.g=(double)g/255;
   os_cmap_bcol.b=(double)b/255;
   fl_set_object_lcol(obj,i);
   if (show_os_cmap) refresh_gl_display();
}

void gl_ann_format_str_cb(FL_OBJECT *obj, long arg)
{
   int i;
   const char *fs;

   fs=fl_get_input(obj);
   i=fl_get_choice(fd_DV_annotation_window->edit)-1;
   replace_ann_edit_fs(fs,i);
   refresh_os_text(i);
}

void gl_ann_which_canvas_cb(FL_OBJECT *obj, long arg)
{
   char buf[128];
   static int prev_canvas=GL_LOCAL_VIEW;

   switch_gl_canvas(prev_canvas); 
   prev_canvas=fl_get_choice(obj)-1;
   switch_gl_canvas(prev_canvas);

   fl_set_button(fd_DV_annotation_window->disp_cmap,show_os_cmap);
   fl_set_choice(fd_DV_annotation_window->border_width,os_cmap_bwidth);
   fl_set_icm_color(FL_FREE_COL1,255*os_cmap_bcol.r,255*os_cmap_bcol.g,255*os_cmap_bcol.b);
   fl_set_object_lcol(fd_DV_annotation_window->border_color,FL_FREE_COL1);
   fl_set_choice(fd_DV_annotation_window->bbox_width,rend_opts.bbox_width);
   fl_set_icm_color(FL_FREE_COL1,255*rend_opts.bbox_color.r,255*rend_opts.bbox_color.g,255*rend_opts.bbox_color.b);
   fl_set_object_lcol(fd_DV_annotation_window->bb_color,FL_FREE_COL1);
   fl_set_choice(fd_DV_annotation_window->point_size,rend_opts.pt_size_1d);
   fl_set_choice(fd_DV_annotation_window->line_width,rend_opts.line_width_1d);
   fl_set_button(fd_DV_annotation_window->anti_alias,rend_opts.anti_alias);
   fl_set_icm_color(FL_FREE_COL1,255*rend_opts.bg_color.r,255*rend_opts.bg_color.g,255*rend_opts.bg_color.b);
   fl_set_object_lcol(fd_DV_annotation_window->bg_color,FL_FREE_COL1);
}


void gl_home_1d_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(GL_LOCAL_VIEW);

   reset_gl_view(1.9,90.0,0.0); 
   refresh_gl_display();
}

void gl_home_cb(FL_OBJECT *obj, long arg)
{
   const char *zoom_c,*theta_c,*phi_c;
   double zoom,theta,phi;

   switch_gl_canvas(GL_LOCAL_VIEW);

   zoom_c=fl_get_input(fd_DV_gl_canvas_window->gl_zoom);
   theta_c=fl_get_input(fd_DV_gl_canvas_window->gl_theta);
   phi_c=fl_get_input(fd_DV_gl_canvas_window->gl_phi);

   zoom=atof(zoom_c); if (zoom==0) zoom=1;
   theta=atof(theta_c);
   phi=atof(phi_c);

   add_to_hist(zoom_c,fd_DV_gl_canvas_window->gl_zoom_hist);
   add_to_hist(theta_c,fd_DV_gl_canvas_window->gl_theta_hist);
   add_to_hist(phi_c,fd_DV_gl_canvas_window->gl_phi_hist);

   reset_gl_view(zoom,theta,phi); 
   refresh_gl_display();
}  

void hvs_home_cb(FL_OBJECT *obj, long arg)
{
   const char *zoom_c,*theta_c,*phi_c;
   double zoom,theta,phi;

   switch_gl_canvas(GL_HVS);

   zoom_c=fl_get_input(fd_DV_gl_canvas_window->gl_zoom);
   theta_c=fl_get_input(fd_DV_gl_canvas_window->gl_theta);
   phi_c=fl_get_input(fd_DV_gl_canvas_window->gl_phi);

   zoom=atof(zoom_c); if (zoom==0) zoom=1;
   theta=atof(theta_c);
   phi=atof(phi_c);

   reset_gl_view(zoom,theta,phi); 
   refresh_gl_display();
}  

void hvs_x_slice_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(GL_LOCAL_VIEW);
   fl_set_button(fd_DV_hvs_window->x_button,1);
   fl_set_button(fd_DV_hvs_window->y_button,0);
   fl_set_button(fd_DV_hvs_window->z_button,0);
   gc_opts.slice=gc_opts_s[GL_HVS].slice=SLICE_X;
   gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v=atof(fl_get_input(fd_DV_hvs_window->x_input));
   set_hvs_thumbwheel();
   set_gc_opts(0);
   if (gl_canvas_open[GL_LOCAL_VIEW]) XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
   send_gl_canvas(1,0,0,0,0);
}

void hvs_y_slice_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(GL_LOCAL_VIEW);
   fl_set_button(fd_DV_hvs_window->x_button,0);
   fl_set_button(fd_DV_hvs_window->y_button,1);
   fl_set_button(fd_DV_hvs_window->z_button,0);
   gc_opts.slice=gc_opts_s[GL_HVS].slice=SLICE_Y;
   gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v=atof(fl_get_input(fd_DV_hvs_window->y_input));
   set_hvs_thumbwheel();
   set_gc_opts(0);
   if (gl_canvas_open[GL_LOCAL_VIEW]) XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
   send_gl_canvas(1,0,0,0,0);
}

void hvs_z_slice_cb(FL_OBJECT *obj, long arg)
{
   switch_gl_canvas(GL_LOCAL_VIEW);
   fl_set_button(fd_DV_hvs_window->x_button,0);
   fl_set_button(fd_DV_hvs_window->y_button,0);
   fl_set_button(fd_DV_hvs_window->z_button,1);
   gc_opts.slice=gc_opts_s[GL_HVS].slice=SLICE_Z;
   gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v=atof(fl_get_input(fd_DV_hvs_window->z_input));
   set_hvs_thumbwheel();
   set_gc_opts(0);
   if (gl_canvas_open[GL_LOCAL_VIEW]) XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
   send_gl_canvas(1,0,0,0,0);
}

void hvs_inc_slice_cb(FL_OBJECT *obj, long arg)
{
   char buf[128];

   switch_gl_canvas(GL_LOCAL_VIEW);
   switch(gc_opts.slice)
   {
      case SLICE_X:
         gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v+=gc_opts_s[GL_HVS].min_dx;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->x_input,buf);
         break;
      case SLICE_Y:
         gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v+=gc_opts_s[GL_HVS].min_dy;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->y_input,buf);
         break;
      case SLICE_Z:
         gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v+=gc_opts_s[GL_HVS].min_dz;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->z_input,buf);
         break;
   }
   set_gc_opts(0);
   if (gl_canvas_open[GL_LOCAL_VIEW]) XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
   send_gl_canvas(1,0,0,0,0);
}

void hvs_dec_slice_cb(FL_OBJECT *obj, long arg)
{
   char buf[128];

   switch_gl_canvas(GL_LOCAL_VIEW);
   switch(gc_opts.slice)
   {
      case SLICE_X:
         gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v-=gc_opts_s[GL_HVS].min_dx;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->x_input,buf);
         break;
      case SLICE_Y:
         gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v-=gc_opts_s[GL_HVS].min_dy;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->y_input,buf);
         break;
      case SLICE_Z:
         gc_opts.slice_v=gc_opts_s[GL_HVS].slice_v-=gc_opts_s[GL_HVS].min_dz;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->z_input,buf);
         break;
   }
   set_gc_opts(0);
   if (gl_canvas_open[GL_LOCAL_VIEW]) XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
   send_gl_canvas(1,0,0,0,0);
}

//-----------------------------------------------------------------------------
// to be called after a send_gl_canvas(...,1);
//-----------------------------------------------------------------------------
void set_hvs_thumbwheel()
{
   FL_OBJECT *obj=fd_DV_hvs_window->slice_slider;
   double Nx,Ny,Nz;
   double thumbwheel_val;

   switch_gl_canvas(GL_HVS);
   switch(gc_opts.slice)
   {
      case SLICE_X:
         Nx=(cam_opts.bbox_ur.x-cam_opts.bbox_ll.x)/gc_opts.min_dx+1;
         thumbwheel_val=(gc_opts.slice_v-cam_opts.bbox_ll.x)/gc_opts.min_dx+1;
         thumbwheel_val=min(thumbwheel_val,Nx);
         fl_set_thumbwheel_bounds(obj,1,Nx);
         break;
      case SLICE_Y:
         Ny=(cam_opts.bbox_ur.y-cam_opts.bbox_ll.y)/gc_opts.min_dy+1;
         thumbwheel_val=(gc_opts.slice_v-cam_opts.bbox_ll.y)/gc_opts.min_dy+1;
         thumbwheel_val=min(thumbwheel_val,Ny);
         fl_set_thumbwheel_bounds(obj,1,Ny);
         break;
      case SLICE_Z:
         Nz=(cam_opts.bbox_ur.z-cam_opts.bbox_ll.z)/gc_opts.min_dz+1;
         thumbwheel_val=(gc_opts.slice_v-cam_opts.bbox_ll.z)/gc_opts.min_dz+1;
         thumbwheel_val=min(thumbwheel_val,Nz);
         fl_set_thumbwheel_bounds(obj,1,Nz);
         break;
   }
   thumbwheel_val=max(thumbwheel_val,1);
   fl_set_thumbwheel_value(obj,thumbwheel_val);
   fl_set_thumbwheel_step(obj,1.0);
}

void hvs_slice_slider_cb(FL_OBJECT *obj, long arg)
{
   char buf[128];
   double i=fl_get_thumbwheel_value(obj)-1;

   switch_gl_canvas(GL_LOCAL_VIEW);

   switch(gc_opts.slice)
   {
      case SLICE_X:
         gc_opts_s[GL_HVS].slice_v=gc_opts.slice_v=cam_opts_s[GL_HVS].bbox_ll.x+i*gc_opts_s[GL_HVS].min_dx;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->x_input,buf);
         break;
      case SLICE_Y:
         gc_opts_s[GL_HVS].slice_v=gc_opts.slice_v=cam_opts_s[GL_HVS].bbox_ll.y+i*gc_opts_s[GL_HVS].min_dy;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->y_input,buf);
         break;
      case SLICE_Z:
         gc_opts_s[GL_HVS].slice_v=gc_opts.slice_v=cam_opts_s[GL_HVS].bbox_ll.z+i*gc_opts_s[GL_HVS].min_dz;
         sprintf(buf,"%lf",gc_opts.slice_v);
         fl_set_input(fd_DV_hvs_window->z_input,buf);
         break;
   }
   set_gc_opts(0);
   if (gl_canvas_open[GL_LOCAL_VIEW]) XRaiseWindow(fl_display,fd_DV_gl_canvas_window->DV_gl_canvas_window->window);
   send_gl_canvas(1,0,0,0,0);
}

void gl_zoom_hist_cb(FL_OBJECT *obj, long obj_arg)
{
   if (fl_get_choice(obj)==1) return;
   fl_set_input(fd_DV_gl_canvas_window->gl_zoom,fl_get_choice_item_text(obj,fl_get_choice(obj)));
   fl_set_choice(obj,1);
}

void gl_theta_hist_cb(FL_OBJECT *obj, long obj_arg)
{
   if (fl_get_choice(obj)==1) return;
   fl_set_input(fd_DV_gl_canvas_window->gl_theta,fl_get_choice_item_text(obj,fl_get_choice(obj)));
   fl_set_choice(obj,1);
}

void gl_phi_hist_cb(FL_OBJECT *obj, long obj_arg)
{
   if (fl_get_choice(obj)==1) return;
   fl_set_input(fd_DV_gl_canvas_window->gl_phi,fl_get_choice_item_text(obj,fl_get_choice(obj)));
   fl_set_choice(obj,1);
}

//=============================================================================
// canvas event handler
//
// emulating explorer's trackball:
//
// left (and default) rotates
// middle or ctr+left moves
// left+middle or crt+middle or shift+left zooms
//=============================================================================
#define ROTATE 1
#define MOVE 2 
#define ZOOM 3

int gl_canvas_event_handler(FL_OBJECT *obj, Window win, int win_width,
                            int win_height, XEvent *xev, void *data)
{
   int ltrace=0;
   FL_FORM *form;
   static int p_mx,p_my,p_action=0;
   static int p_mb=0;
   int mx,my,vx,vy,i,dw,dh;
   unsigned int keymask;
   vect vm,vf;
   double r_mx,r_my,r_mz,norm,rad;
   int lmb=0,mmb=0,rmb=0,shift_mod=0,ctrl_mod=0,action=0;
   int which_canvas;
   double rspeed=0.3,zspeed=0.01,mspeed=0.005,scale;

   IFL printf("in gl_canvas_event_handler:\n");

   if (obj==fd_DV_gl_canvas_window->gl_canvas)
   {
      switch_gl_canvas(GL_LOCAL_VIEW);
      form=fd_DV_gl_canvas_window->DV_gl_canvas_window;
      which_canvas=GL_LOCAL_VIEW;
   }
   else if (obj==fd_DV_hvs_window->gl_canvas)
   {
      switch_gl_canvas(GL_HVS);
      form=fd_DV_hvs_window->DV_hvs_window;
      which_canvas=GL_HVS;
   }
   else
   {
      printf("gl_canvas_event_handler: unknown canvas???\n");
      return 0;
   }

   if ((xev->type)==ResizeRequest || (xev->type)==Expose)
   {
      cam_opts.hw_aspect_ratio=(double)obj->h/obj->w;
      IFL printf("aspect ratio=%f\n",cam_opts.hw_aspect_ratio);
      glViewport(0,0,obj->w,obj->h);
      // simple 'gravity' for text, based upon which edge 'closest' to.
      // doesn't work too well yet, so force user to turn on
      if (fl_get_button(fd_DV_annotation_window->autoscale))
      {
         dw=obj->w-prev_canvas_w[which_canvas];
         dh=obj->h-prev_canvas_h[which_canvas];
         for (i=0; i< MAX_OS_TEXTS;i++)
         {
            if (os_text_list[i].which_canvas==which_canvas)
            {
               if ((os_text_list[i].pos.x > obj->w/4) && 
                   ((os_text_list[i].pos.x+dw) > obj->w/4) ) os_text_list[i].pos.x+=dw;
               if ((os_text_list[i].pos.y > obj->h/4) &&
                   ((os_text_list[i].pos.y+dh) > obj->h/4) ) os_text_list[i].pos.y+=dh;
            }
         }
         if ((os_cmap_p1.x > obj->w/4) && ((os_cmap_p1.x+dw) > obj->w/4)) os_cmap_p1.x+=dw;
         if ((os_cmap_p1.y > obj->h/4) && ((os_cmap_p1.y+dh) > obj->h/4)) os_cmap_p1.y+=dh;
         if ((os_cmap_p2.x > obj->w/4) && ((os_cmap_p2.x+dw) > obj->w/4)) os_cmap_p2.x+=dw;
         if ((os_cmap_p2.y > obj->h/4) && ((os_cmap_p2.y+dh) > obj->h/4)) os_cmap_p2.y+=dh;
      }
      prev_canvas_w[which_canvas]=obj->w;
      prev_canvas_h[which_canvas]=obj->h;
      refresh_gl_display();
      return 1;
   }

   fl_get_mouse(&mx,&my,&keymask);

   if (keymask&Button1Mask) lmb=1;
   if (keymask&Button2Mask) mmb=1;
   if (keymask&Button3Mask) rmb=1;

   if (keymask&ShiftMask) shift_mod=1; 
   if (keymask&ControlMask) ctrl_mod=1; 

   action=0;
   if ((lmb && mmb) || (ctrl_mod && mmb) || (shift_mod && lmb)) action=ZOOM;
   else if (mmb || (lmb && ctrl_mod)) action=MOVE;
   else if (mmb || lmb || rmb) action=ROTATE;

   IFL printf("mx=%i,my=%i,lmb=%i,mmb=%i,rmb=%i,shift_mod=%i,ctrl_mod=%i:\n",mx,my,lmb,mmb,rmb,shift_mod,ctrl_mod);

   if (gl_pos==POS_TEXT)
   {
      r_mx=mx-(obj->x+form->x);
      r_my=-my+(obj->h+obj->y+form->y);  // screen y is 'reversed', hence - sign
      i=fl_get_choice(fd_DV_annotation_window->edit)-1;
      os_text_list[i].pos.x=r_mx;
      os_text_list[i].pos.y=r_my;
      os_text_list[i].which_canvas=which_canvas;
      refresh_os_text(i);
      return 1;
   }
   else if (gl_pos==POS_CMAP)
   {
      r_mx=mx-(obj->x+form->x);
      r_my=-my+(obj->h+obj->y+form->y);  // screen y is 'reversed', hence - sign
      if (lmb)
      {
         os_cmap_p2.x=(os_cmap_p2.x-os_cmap_p1.x)+r_mx;
         os_cmap_p2.y=(os_cmap_p2.y-os_cmap_p1.y)+r_my;
         os_cmap_p1.x=r_mx;
         os_cmap_p1.y=r_my;
      }
      else if (rmb)
      {
         os_cmap_p2.x=r_mx;
         os_cmap_p2.y=r_my;
      }
      refresh_gl_display();
      return 1;
   }

   cam_opts.rot_angle=0;
   cam_opts.dispx=0;
   cam_opts.dispy=0;

   if (action>0 && p_mb>0)
   {
      vx=mx-p_mx;
      vy=my-p_my;

      scale=max(fabs(cam_opts.bbox_ur.x-cam_opts.bbox_ll.x),fabs(cam_opts.bbox_ur.y-cam_opts.bbox_ll.y));
      scale=max(scale,fabs(gc_opts.zscale*(cam_opts.bbox_ur.z-cam_opts.bbox_ll.z)));

      switch(action)
      {
         case ROTATE:
            if (vx==0 && vy==0) return 1;

            // emulate a trackball -- ball sits at the center of the canvas.
           
            // position of mouse relative to center of trackball:
            r_mx=mx-(obj->w/2+obj->x+form->x);
            r_my=-my+(obj->h/2+obj->y+form->y);  // screen y is 'reversed', hence - sign
            rad=(obj->w+obj->h)/4; // radius of track ball
            if ((r_mx*r_mx+r_my*r_my)>(rad*rad)) r_mz=0; else r_mz=sqrt(rad*rad-(r_mx*r_mx+r_my*r_my));
            norm=sqrt(r_mx*r_mx+r_my*r_my+r_mz*r_mz);
            vm.x=r_mx/norm; vm.y=r_my/norm; vm.z=r_mz/norm;
            // vf -- applied 'force' (a better model may be to apply the force
            // tangent to the sphere)
            vf.z=0;
            norm=sqrt(vx*vx+vy*vy);
            vf.x=vx/norm;
            vf.y=-vy/norm;
            // rotation axis is vf X vm :
            cam_opts.rot_axis.x=vf.y*vm.z-vf.z*vm.y;
            cam_opts.rot_axis.y=-(vf.x*vm.z-vf.z*vm.x);
            cam_opts.rot_axis.z=vf.x*vm.y-vf.y*vm.x;
            cam_opts.rot_angle-=norm*rspeed;
            break;
         case ZOOM:
            cam_opts.zoom*=max(0.0001,(1-vy*zspeed));
            break;
         case MOVE: 
            cam_opts.dispx+=mspeed*vx*scale*cam_opts.zoom;
            cam_opts.dispy-=mspeed*vy*scale*cam_opts.zoom;
            break;
      }

      refresh_gl_display();
   }

   p_mx=mx; p_my=my; p_mb=(lmb||mmb||rmb);
   p_action=action;

   return 1;
}

//=============================================================================
// Hiearchy Visualizer and Slicer functions
//=============================================================================
void hvs_cb(FL_OBJECT *obj, long arg)
{
   int ltrace=0;
   int item;

   IFL printf("in hvs_cb:\n");
   
   if (!gl_canvas_open[GL_HVS]) open_hvs();
}

//=============================================================================
// screen capture functions
//=============================================================================

void capture_browse_cb(FL_OBJECT *obj, long arg)
{
   if (fl_show_fselector("Select directory",0,0,0))
      fl_set_input(fd_DV_capture_window->dir,fl_get_directory());
}

void capture_cb(FL_OBJECT *obj, long arg)
{
   if (capture_on)
   {
      capture_on=0;
      fl_set_object_label(obj,"Start capture");
   }
   else
   {
      capture_on=1;
      fl_set_object_label(obj,"Stop capture");
      capture_n=atoi(fl_get_input(fd_DV_capture_window->first_index));
      capture_dir=fl_get_input(fd_DV_capture_window->dir);
      capture_file_name_fs=fl_get_input(fd_DV_capture_window->format);
      capture_jpeg_qf=atoi(fl_get_input(fd_DV_capture_window->jpeg_qf));
      if (capture_jpeg_qf>100) capture_jpeg_qf=100;
      if (capture_jpeg_qf<0) capture_jpeg_qf=0;
   }
}

//=============================================================================
// Select color function callback
//=============================================================================

int is_set_cm_min=0;
int is_set_cm_max=0;
int num_particle_f=0;

void f_select_cb(FL_OBJECT *obj, long arg)
{
   int wf;
   int ltrace=0;
   
   IFL printf("Entering f_select_cb()\n"); 
   wf = get_gl_which_particle_f();
}

void f_select_scmin_cb(FL_OBJECT *obj, long arg)
{ 
   int ltrace=0;
   if(is_set_cm_min){
      is_set_cm_min= 0;
      IFL printf("f_select_scmin_cb: %d\n", is_set_cm_min); 
      fl_hide_object(fd_DV_f_select_window->cm_min);
   }
   else{
      is_set_cm_min= 1;
      IFL printf("f_select_scmin_cb: %d\n", is_set_cm_min); 
      fl_show_object(fd_DV_f_select_window->cm_min);
   }
}

void f_select_scmax_cb(FL_OBJECT *obj, long arg)
{
   int ltrace=0;
   if(is_set_cm_max){
      is_set_cm_max = 0;
      IFL printf("f_select_scmax_cb: %d\n", is_set_cm_max); 
      fl_hide_object(fd_DV_f_select_window->cm_max);
   }
   else{
      is_set_cm_max = 1;
      IFL printf("f_select_scmax_cb: %d\n", is_set_cm_max); 
      fl_show_object(fd_DV_f_select_window->cm_max);
   }
}

void f_select_cm_min_cb(FL_OBJECT *obj, long arg)
{
   int ltrace=0;
   IFL printf("cm_min changed.\n");
   IFL printf("cm_min is: %f\n",atof(fl_get_input(fd_DV_f_select_window->cm_min)) ); 
}

void f_select_cm_max_cb(FL_OBJECT *obj, long arg)
{
   int ltrace=0;
   IFL printf("cm_max changed.\n");
   IFL printf("cm_max is: %f\n",atof(fl_get_input(fd_DV_f_select_window->cm_max)) ); 
}

void get_gl_cm_min(double *cm_min)
{
   /* Sets "cm_min" to the minimum value of the user-defined colormap. */
   int ltrace=0;
   IFL printf("cm_min in: %f\n",*cm_min );
   IFL printf("the min colormap val: %f\n", atof(fl_get_input(fd_DV_f_select_window->cm_min)) );
   *cm_min = atof(fl_get_input(fd_DV_f_select_window->cm_min));
}

void get_gl_cm_max(double *cm_max)
{
   /* Sets "cm_max" to the maximum value of the user-defined colormap. */
   int ltrace=0;
   IFL printf("cm_max in: %f\n",*cm_max );
   IFL printf("the max colormap val: %f\n", atof(fl_get_input(fd_DV_f_select_window->cm_max)) );
   *cm_max = atof(fl_get_input(fd_DV_f_select_window->cm_max));
}

void get_gl_f_min(double *f_min)
{
   /* Sets "f_min" to the minimum value of points that will be shown. */
   int ltrace=0;
   if (strcmp( fl_get_input(fd_DV_f_select_window->show_min), "") == 0 ){
	   IFL printf("You've entered nothing. Should use absolute min.\n");
	   *f_min = -1.0/0.0;
   }
   else{
	   IFL printf("the min input val: %f\n", atof(fl_get_input(fd_DV_f_select_window->show_min)) );
	   *f_min = atof(fl_get_input(fd_DV_f_select_window->show_min));
   }
}

void get_gl_f_max(double *f_max)
{
   /* Sets "f_max" to the maximum value of points that will be shown. */
   int ltrace=0;
   if (strcmp( fl_get_input(fd_DV_f_select_window->show_max), "") == 0 ){
	   IFL printf("You've entered nothing. Should use absolute max.\n");
	   *f_max = 1.0/0.0;
   }
   else{
	   IFL printf("the max input val: %f\n", atof(fl_get_input(fd_DV_f_select_window->show_max)) );
	   *f_max = atof(fl_get_input(fd_DV_f_select_window->show_max));
   }
}

int get_gl_which_particle_f(void)
{
   double f_min, f_max;
   int wf,ltrace=0;

   wf=atoi(fl_get_input(fd_DV_f_select_window->wf));
   if(is_set_cm_min){
	   f_min = atof(fl_get_input(fd_DV_f_select_window->cm_min));
   }
   
   if(is_set_cm_max){
	  f_max = atof(fl_get_input(fd_DV_f_select_window->cm_max));
   }
   IFL printf("Colormap: %d \tf_min: %f \tf_max: %f\n", wf, f_min, f_max);

   if ( wf < 0)
   {
      printf("f_select_cb(): cannot choose a negative function index to color by.\n Choosing 0.");
      fl_set_input(fd_DV_f_select_window->wf,"0");
      wf=0;
   }
   if ( wf > num_particle_f)
   {
      printf("f_select_cb(): %d exceed number of existing functions %d.\n Choosing 0.\n",wf, num_particle_f);
      fl_set_input(fd_DV_f_select_window->wf,"0");
      wf=0;
   }

   return wf;
}

//=============================================================================
// data browser functions
//=============================================================================

int db_i0=0;
int db_j0=0;
grid *db_g=0;

void db_new_grid(reg *r,time_str *ts,grid *g)
{
   char buf[1024];
   double dx;
   char *c1,*c2;
   char def_c1[4]="x1";
   char def_c2[4]="x2";

   c1=r->coord_names[1];
   c2=r->coord_names[2];

   if (!c1) c1=def_c1;
   if (!c2) c2=def_c2;

   dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);

   db_i0=0;
   db_j0=0;
   fl_set_slider_value(fd_DV_dbrowser_window->h_slider,1.0);
   fl_set_slider_value(fd_DV_dbrowser_window->v_slider,1.0);

   if (g->dim==1)
   {
      sprintf(buf,"register: %s\ncoordinate: %s\nshape: [%i], dx=%f, t=%f\n",
              r->name,c1,g->shape[0],dx,ts->time);
      fl_set_slider_bounds(fd_DV_dbrowser_window->v_slider,1.0,(double)g->shape[0]);
   }
   else if (g->dim==2)
   {
      sprintf(buf,"register: %s\ncoordinates: %s,%s\nshape: [%i,%i], dx=%f, t=%f\n",
              r->name,c1,c2,g->shape[0],g->shape[1],dx,ts->time);
      fl_set_slider_bounds(fd_DV_dbrowser_window->v_slider,1.0,(double)g->shape[1]);
      fl_set_slider_bounds(fd_DV_dbrowser_window->h_slider,1.0,(double)g->shape[0]);
   }
   else
   {
      if(db_g) gfree(db_g); db_g=0;
      sprintf(buf,"3D grids not yet supported");
      refresh_db();
      return;
   }

   fl_set_object_label(fd_DV_dbrowser_window->info,buf);

   if (db_g) gfree(db_g);
   if (!(db_g=galloc(g->dim,g->coord_type,g->shape,g->time,g->coords,g->ccoords,g->data)))
      printf("db_new_grid: error, out of memory\n");

   if (g->coord_type!=COORD_UNIFORM && g->dim==1) 
      printf("db_new_grid: WARNING ... only uniform 2D grids will display correct coordinate info\n");

   refresh_db();

   return;
}

void refresh_db(void)
{
   const char *d_fs,*c_fs;
   int width,nx,ny,n,tbuf_size;
   char buf[256];
   char *tbuf,*p,*nl;
   char spaces[31]="                              ";
   double f;

   FL_Coord tx,ty,tw,th;
   double dx,dy,x0,y0,x,y;

   int ascend,descend,cw,ch,nr,nc,i,j;

   FD_DV_dbrowser_window *fd=fd_DV_dbrowser_window;   

   if (!db_g)
   {
      fl_set_object_label(fd->info,"<no data>");
      fl_set_object_label(fd->data," ");
      fl_set_object_label(fd->i_info," ");
      fl_set_object_label(fd->j_info," ");
      return;
   }

   c_fs=fl_get_input(fd->coord_fs);
   d_fs=fl_get_input(fd->data_fs);

   width=0;

   if (db_g->dim==2)
   {
      nx=db_g->shape[0];
      x0=db_g->coords[0];
      dx=(db_g->coords[1]-db_g->coords[0])/(nx-1);
      ny=db_g->shape[1];
      y0=db_g->coords[2];
      dy=(db_g->coords[3]-db_g->coords[2])/(ny-1);
      if (db_reverse_y)
      {
         y0=y0+(ny-1)*dy;
         dy=-dy;
      }
      if (db_i0 >= db_g->shape[0] || db_i0<0) db_i0=0;
      if (db_j0 >= db_g->shape[1] || db_j0 <0) db_j0=0;
   }
   else
   {
      nx=1;
      x0=0;
      dx=0;
      ny=db_g->shape[0];
      y0=db_g->coords[0];
      dy=(db_g->coords[1]-db_g->coords[0])/(ny-1);
      if (db_j0 >= db_g->shape[0] || db_j0 <0) db_j0=0;
      db_i0=0;
   }

   if (db_reverse_y && db_g->dim==2) f=db_g->data[db_i0+(ny-1-db_j0)*nx]; else f=db_g->data[db_i0+db_j0*nx];
   sprintf(buf,d_fs,f);
   width=max(width,strlen(buf));
   sprintf(buf,c_fs,x0+dx);
   width=max(width,strlen(buf));
   width=width+3;
   width=min(width,30);
//   test[width]=0;
//   fl_set_tabstop(test); I couldn't get tabstops to work
   fl_get_object_geometry(fd->data,&tx,&ty,&tw,&th);
   ch=fl_get_char_height(4,12,&ascend,&descend);
   cw=fl_get_char_width(4,12);
   nr=th/ch+1;
   nc=tw/width/cw+1;

//   printf("nr,nc,th,tw,ch,cw,width:%i,%i,%i,%i,%i,%i,%i\n",nr,nc,th,tw,ch,cw,width);

   tbuf_size=nr*nc*width*4;
   if (!(tbuf=(char *)malloc(tbuf_size)))
   {
      printf("refresh_db: out of memory\n"); return;
   }

   p=tbuf;
   if (db_screen_dump) printf("\n");
   for (j=db_j0; j<(nr+db_j0) && j<ny && ((p-tbuf)<tbuf_size/2); j++)
   {
      if (db_screen_dump)
      {
         printf("i1,j=[%5i,%5i], x1,y=[",db_i0+1,j+1);
         printf(c_fs,db_i0*dx+x0);
         printf(",");
         if (db_g->dim==1 && db_g->coord_type==COORD_CURVILINEAR) 
            printf(c_fs,db_g->coords[j]);
         else
            printf(c_fs,j*dy+y0);
         printf("] : ");
      }
      for (i=db_i0; i<(nc+db_i0) && i<nx && ((p-tbuf)<tbuf_size/2); i++)
      {
         if (db_reverse_y && db_g->dim==2) f=db_g->data[i+(ny-1-j)*nx]; else f=db_g->data[i+j*nx];
         sprintf(p,d_fs,f);
         n=strlen(p); p+=n;
         strncpy(p,spaces,abs(width-n)); 
         p+=abs(width-n); *p=0;
         if (db_screen_dump)
         {
            printf(d_fs,f); printf("\t");
         }

      }
      if (db_screen_dump) printf("\n");
      *p++='\n';
      *p=0;
   }

   fl_set_object_label(fd->data,tbuf);

   p=tbuf;
   for (i=db_i0; i<(nc+db_i0) && i<nx;i++)
   {
      sprintf(p,"%5i",i+1);
      n=strlen(p); p+=n;
      strncpy(p,spaces,abs(width-n));
      p+=abs(width-n); *p=0;
   }
   *p++='\n';
   *p=0;
   for (i=db_i0,x=db_i0*dx+x0; i<(nc+db_i0) && i<nx; i++, x+=dx)
   {
      sprintf(p,c_fs,x);
      n=strlen(p); p+=n;
      strncpy(p,spaces,abs(width-n));
      p+=abs(width-n); *p=0;
   }

   fl_set_object_label(fd->i_info,tbuf);

   p=tbuf;
   for (j=db_j0,y=db_j0*dy+y0; j<(nr+db_j0) && j<ny;j++, y+=dy)
   {
      if (db_g->dim==1 && db_g->coord_type==COORD_CURVILINEAR) y=db_g->coords[j];
      sprintf(p,"%5i  ",j+1);
      p+=strlen(p);
      sprintf(p,c_fs,y);
      p+=strlen(p);
      *p++='\n';
      *p=0;
   }

   fl_set_object_label(fd->j_info,tbuf);

   free(tbuf);

   return;
}

void db_h_slider_cb(FL_OBJECT *obj, long arg)
{
   db_i0=fl_get_slider_value(obj)-1;
   refresh_db();
}

void db_v_slider_cb(FL_OBJECT *obj, long arg)
{
   db_j0=fl_get_slider_value(obj)-1;
   refresh_db();
}

void db_data_fs_cb(FL_OBJECT *obj, long arg)
{
   refresh_db();
}

void db_coord_fs_cb(FL_OBJECT *obj, long arg)
{
   refresh_db();
}

void db_reverse_y_cb(FL_OBJECT *obj, long arg)
{
   if (db_reverse_y==1) db_reverse_y=0; else db_reverse_y=1;
   refresh_db();
}

void db_screen_dump_cb(FL_OBJECT *obj, long arg)
{
   if (db_screen_dump)
      db_screen_dump=0;
   else
   {
      db_screen_dump=1;
      refresh_db();
   }
}
void db_sm_save_cb(FL_OBJECT *obj, long obj_arg)
{
 
   char *file_name;
   FILE *stream;
   float *fdata,*f;
   int size,i;
   double *d;
 
   if (!db_g) return;
 
   if (file_name=(char *)fl_show_fselector("Enter file name",0,"*",0))
   {
      if (stream=fopen(file_name,"r"))
      {
         fclose(stream);
         if (!(fl_show_question("Overwrite existing file?",0))) return;
      }
      if (stream=fopen(file_name,"w"))
      {
         size=db_g->shape[0]*db_g->shape[1];
         if (!(f=fdata=(float *)malloc(size*sizeof(float)))) 
         {
            printf("db_sm_save_cb: out of memory\n"); fclose(stream); return;
         }
         d=db_g->data;
         for (i=0;i<size;i++) *f++=*d++;
         fwrite(db_g->shape,sizeof(int),2,stream);
         fwrite(fdata,sizeof(float),size,stream);
         fclose(stream);
         free(fdata);
      }
   }
   return;
}   

void data_browser_cb(FL_OBJECT *obj, long obj_arg)
{
   fl_show_form(fd_DV_dbrowser_window->DV_dbrowser_window,FL_PLACE_CENTERFREE,FL_FULLBORDER,"DV data browser");
   fl_set_form_minsize(fd_DV_dbrowser_window->DV_dbrowser_window,300,200);
   XRaiseWindow(fl_display,fd_DV_dbrowser_window->DV_dbrowser_window->window);
   db_open=1;

   refresh_db();
}

/****************************************
 * This part was added by Martin Snajdr *
 ****************************************/

void mstest_menu_cb(FL_OBJECT *ob, long data)
{

   int ltrace=0;
   int item;
   char *file_name;

   IFL printf("in mstest_menu_cb:\n");

   item=fl_get_menu(ob);

   switch(item)
   {
      case 1:
         fl_show_form(fd_DV_mstest_item1_window->DV_mstest_item1_window,FL_PLACE_CENTERFREE,FL_FULLBORDER,"User functions");
         fl_set_form_minsize(fd_DV_mstest_item1_window->DV_mstest_item1_window,500,400);
         XRaiseWindow(fl_display,fd_DV_mstest_item1_window->DV_mstest_item1_window->window);
         break;
      case 2:
         IFL printf("item 2 (Load usef functions) selected\n");
         if (file_name=(char *)fl_show_fselector("Select file",0,"*",0)) {
            read_userf_file(file_name);
         }
         break;
      case 3:
        IFL printf("item 3 (Save usef functions) selected\n");
         if (file_name=(char *)fl_show_fselector("Select file",0,"*",0)) {
            write_userf_file(file_name);
         }
         break;
   }
}

int write_userf_file(char *file_name)
{
  int i,j;
  userf_table_entry *ute;
  userf_macro_line *ufml;
  FILE *stream;
  if (!(stream=fopen(file_name,"w")))
    {
      printf("Save user functions: unable to open file %s\n",file_name);
      return 0;
    }
  fprintf(stream,"default_settings\n");
  fprintf(stream,"reg_a = %s\n",def_AB_etc.reg_a);
  fprintf(stream,"reg_b = %s\n",def_AB_etc.reg_b);
  fprintf(stream,"mask = %s\n",def_AB_etc.reg_b);
  fprintf(stream,"mvalue = %s\n",def_AB_etc.mvalue);
  fprintf(stream,"args = %s\n",def_AB_etc.args);
  fprintf(stream,"new_name = %s\n\n",def_AB_etc.new_name);
  
  for (i=0;i<=userf_table.last;i++) {
    ute=userf_table.entry[i];
    fprintf(stream,"name = %s\n",ute->name); 
    for (j=0;j<=ute->last;j++) {
      ufml = ute->macro_line[j];
      fprintf(stream,"  func = %s\n",ufml->func); 
      fprintf(stream,"    reg_a = %s\n",ufml->reg_a); 
      fprintf(stream,"    reg_b = %s\n",ufml->reg_b); 
      fprintf(stream,"    mask = %s\n",ufml->mask); 
      fprintf(stream,"    mvalue = %s\n",ufml->mvalue); 
      fprintf(stream,"    args = %s\n",ufml->args); 
      fprintf(stream,"    new_name = %s\n",ufml->new_name); 
    }
    fprintf(stream,"\n",ufml->new_name); 
  }
  fprintf(stream,"name = END\n");
  fflush(stream);
  fclose(stream);
}

int read_userf_file(char *file_name)
{
  FILE *stream;
  int ltrace=0,i,j,nufunc,flag,mline;
  int input_pos[MAX_USERF_INPUT_PARAMS];
  int input_len[MAX_USERF_INPUT_PARAMS];
  int input_num[MAX_USERF_INPUT_PARAMS],ninp,ninput,maxinp;
  char *line;
  char *str1;
  char *str2;
  char ch;
  char *str,*tstr,*ufname;
  userf_table_entry *ute;

  if (!(stream=fopen(file_name,"r")))
   {
      printf("Load user functions: unable to open file %s\n",file_name);
      return 0;
   }
  tstr = calloc(MAX_USERF_INFO_LENGTH,sizeof(char));
  if (!tstr) {
    printf("mstest read_userf_file: CANNOT ALLOCATE MEMORY!!!\n");
    exit(-1);
  }
  str1=calloc(MAX_USERF_LINE_LENGTH,sizeof(char));
  if (!str1) {
    printf("mstest read_userf_file: CANNOT ALLOCATE MEMORY!!!\n");
    exit(-1);
  }
  str2=calloc(MAX_USERF_LINE_LENGTH,sizeof(char));
  if (!str2) {
    printf("mstest read_userf_ CANNOT ALLOCATE MEMORY!!!\n");
    exit(-1);
  }
  line=calloc(MAX_USERF_LINE_LENGTH,sizeof(char));
  if (!line) {
    printf("mstest read_userf_ CANNOT ALLOCATE MEMORY!!!\n");
    exit(-1);
  }
  nufunc = -1; // user function counter
  flag = 0; 
  while(fgets(line,MAX_USERF_LINE_LENGTH,stream)){
    IFL printf("mstest %s",line);
    i = 0;
    j = 0;
    str1[0] = '\0';
    str2[0] = '\0';
    str = str1;
    while(ch=line[i++]){
      if (ch != '\n') 
         if (ch == '=')  {
           str[j] = '\0';
           j = 0;
           str = &str2[0];
         }
         else
           str[j++]=ch;
    }
    str[j] = '\0';
    cut_space(str1,str1);
    cut_space(str2,str2);
    IFL printf("mstest  str1=!%s!  str2=!%s!\n",str1,str2);


    if (!strcmp(str1,"default_settings\0") && !strcmp(str2,"\0")) {
       flag = 1;
       IFL printf("read_user_file: Reading defaul settings\n");
    }
    else if (!strcmp(str1,"name\0")){ // start of new userfun definition (unless END)
      flag = 0;
      if (nufunc>=0) {
         ute->last = mline;
         ute->ninput_regs = get_ninput(nufunc);
         sprintf(tstr,"Number of input registers: %d",ute->ninput_regs);
         ute->info = strdup(tstr);
      }
      if (!strcmp(str2,"END\0")) {
         IFL printf("mstest END reached %d %d\n",nufunc,mline);
         break;
      }
      if (!(ute = malloc(sizeof(userf_table_entry)))) {
        printf("mstest can not allocate memory!!!\n");
        exit(-1);
      }
      nufunc++;
      userf_table.entry[nufunc] = ute;
      mline = -1;
      ute->name = strdup(str2);
    }
    else if (!strcmp(str1,"func\0")){ // next macro line
      mline++;
      if (!(ute->macro_line[mline] = malloc(sizeof(userf_macro_line)))) {
        printf("mstest can not allocate memory!!!\n");
        exit(-1);
      }
      i = get_func_indx(str2);
      ute->macro_line[mline]->func = strdup((char *)func_table[i].name);
      ute->macro_line[mline]->ifunc = i;
      ute->macro_line[mline]->reg_a = strdup(def_AB_etc.reg_a);
      ute->macro_line[mline]->reg_b = strdup(def_AB_etc.reg_b);
      ute->macro_line[mline]->mask = strdup(def_AB_etc.mask);
      ute->macro_line[mline]->mvalue = strdup(def_AB_etc.mvalue);
      ute->macro_line[mline]->args = strdup(def_AB_etc.args);
      ute->macro_line[mline]->new_name = strdup(def_AB_etc.new_name);
    } 
    else if(!strcmp(str1,"reg_a\0")){
      if (flag==0) {
        ute->macro_line[mline]->reg_a = strdup(str2);
      }
      else if (flag==1) def_AB_etc.reg_a = strdup(str2);
    }
    else if(!strcmp(str1,"reg_b\0")){
      if (flag==0) {
        ute->macro_line[mline]->reg_b = strdup(str2);
      }
      else if (flag==1) def_AB_etc.reg_b = strdup(str2);
    }
    else if(!strcmp(str1,"mask\0")){
      if (flag==0) {
        ute->macro_line[mline]->mask = strdup(str2);
      }
      else if (flag==1) def_AB_etc.mask = strdup(str2);
    }
    else if(!strcmp(str1,"mvalue\0")){
      if (flag==0) {
        ute->macro_line[mline]->mvalue = strdup(str2);
      }
      else if (flag==1) def_AB_etc.mvalue = strdup(str2);
    }
    else if(!strcmp(str1,"args\0")){
      if (flag==0) {
        ute->macro_line[mline]->args = strdup(str2);
      }
      else if (flag==1) def_AB_etc.args = strdup(str2);
    }
    else if(!strcmp(str1,"new_name\0")){
      if (flag==0) {
        ute->macro_line[mline]->new_name = strdup(str2);
      }
      else if (flag==1) def_AB_etc.new_name = strdup(str2);
    }
  }
  free(tstr);
  free(str1);
  free(str2);
  fclose(stream);
  userf_table.last = nufunc;
  fill_userf_list();
  if (nufunc>=0) {
    fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,1);
    update_userf(0); 
    backup_userf(0);
  }
  
  return 1;
}

void free_userf_macro_line(userf_macro_line *ufml)
{
  free(ufml->func);
  free(ufml->reg_a);
  free(ufml->reg_b);
  free(ufml->mask);
  free(ufml->mvalue);
  free(ufml->args);
  free(ufml->new_name);

}

void free_userf_table_entry(userf_table_entry *ute)
{
  int i;
  free(ute->name);
  free(ute->info);
  for (i=0;i<=ute->last;i++)  {
    free_userf_macro_line(ute->macro_line[i]);
  }
}

void backup_userf(int line)
{
  userf_table_entry *ute;
  int i;
  int ltrace=0;

  IFL printf("mstest backup_userf ******* %d\n",line);
  ute = userf_table.entry[line];
  if (last_backed_up>=0) free_userf_table_entry(&backup_entry);
  backup_entry.name = strdup(ute->name);
  backup_entry.info = strdup(ute->info);
  backup_entry.ninput_regs = ute->ninput_regs;
  backup_entry.last = ute->last;
  IFL printf("mstest backup_userf %d\n",ute->last);
  for (i=0;i<=ute->last;i++) {
    backup_entry.macro_line[i] = malloc(sizeof(userf_macro_line));
    if (!backup_entry.macro_line[i]) {
           printf("mstest backup_userf:CANNOT ALLOCATE MEMORY!!!\n");
           exit(-1);
         }
    backup_entry.macro_line[i]->func = strdup(ute->macro_line[i]->func);
    backup_entry.macro_line[i]->ifunc = ute->macro_line[i]->ifunc;
    backup_entry.macro_line[i]->reg_a = strdup(ute->macro_line[i]->reg_a);
    backup_entry.macro_line[i]->reg_b = strdup(ute->macro_line[i]->reg_b);
    backup_entry.macro_line[i]->mask = strdup(ute->macro_line[i]->mask);
    backup_entry.macro_line[i]->mvalue = strdup(ute->macro_line[i]->mvalue);
    backup_entry.macro_line[i]->args = strdup(ute->macro_line[i]->args);
    backup_entry.macro_line[i]->new_name = strdup(ute->macro_line[i]->new_name);
  }
  last_backed_up = line;
}
void restore_userf(int line)
{
  userf_table_entry *ute;
  int i;
  int ltrace=0;
  IFL printf("mstest restore_userf *********** %d\n",line);
  if (line<0) return;
  ute = userf_table.entry[line];
  free_userf_table_entry(ute);
  ute->name = strdup(backup_entry.name);
  ute->info = strdup(backup_entry.info);
  ute->ninput_regs = backup_entry.ninput_regs;
  ute->last = backup_entry.last;
  for (i=0;i<=ute->last;i++) {
    ute->macro_line[i] = malloc(sizeof(userf_macro_line));
    if (!ute->macro_line[i]) {
           printf("mstest restore_userf:CANNOT ALLOCATE MEMORY!!!\n");
           exit(-1);
         }
    ute->macro_line[i]->func = strdup(backup_entry.macro_line[i]->func); 
    ute->macro_line[i]->ifunc = backup_entry.macro_line[i]->ifunc; 
    ute->macro_line[i]->reg_a= strdup(backup_entry.macro_line[i]->reg_a); 
    ute->macro_line[i]->reg_b= strdup(backup_entry.macro_line[i]->reg_b); 
    ute->macro_line[i]->mask = strdup(backup_entry.macro_line[i]->mask); 
    ute->macro_line[i]->mvalue = strdup(backup_entry.macro_line[i]->mvalue); 
    ute->macro_line[i]->args = strdup(backup_entry.macro_line[i]->args); 
    ute->macro_line[i]->new_name = strdup(backup_entry.macro_line[i]->new_name); 
  }

}

int contains_input(char *str,int *input_pos,int *input_len,int *input_num,int *maxinput)
/*
 searches for pattern "$ddd" in a string str (ddd are digits)
 return position of each pattern in inpur_pos,its length in inpur_len,
 the corresponding integer value of ddd in input_numb
 and the maximum of ddd
*/
{
  char *nstr,ch,ch2;
  int i,j,ninp,i1,i2;
  int ltrace=0;
  i = 0;
  ninp = 0;
  IFL printf("mstest contains_input processes %s\n",str);
  while (ch=str[i]) {
    if (ch == '$') {
       i1 = ++i;
       i2 = i1-1;
       while (ch2=str[i]) {
         if (isdigit(ch2))
           i2 = i;
         else
           break;
         i++;
       }
       if (i2>=i1) {
         nstr = calloc(i2-i1+2,sizeof(char));
         if (!nstr) {
           printf("mstest contains_input :CANNOT ALLOCATE MEMORY!!!\n");
           exit(-1);
         }
         input_pos[ninp] = i1-1;
         input_len[ninp] = i2-i1+2;
         for (j=i1;j<=i2;j++)
           nstr[j-i1] = str[j];
         nstr[i2-i1+1] = '\0';
         input_num[ninp] = atoi(nstr);
         if (input_num[ninp]>*maxinput)
            *maxinput = input_num[ninp];
         ninp++;
         free(nstr);
       }
    }
    else
      i++;
  }
  IFL printf("mstest contains_input maxinp=%d\n",*maxinput);
  return(ninp);
}

int get_ninput(int func)
{
  int input_pos[MAX_USERF_INPUT_PARAMS];
  int input_len[MAX_USERF_INPUT_PARAMS];
  int input_num[MAX_USERF_INPUT_PARAMS],ninp,i,maxinp,ninput;
  userf_table_entry *ute;
  ute = userf_table.entry[func];
  maxinp = 0;
  for (i=0;i<=ute->last;i++) {
    ninp = contains_input(ute->macro_line[i]->reg_a,input_pos,input_len,input_num,&maxinp);
    ninp = contains_input(ute->macro_line[i]->reg_b,input_pos,input_len,input_num,&maxinp);
    ninp = contains_input(ute->macro_line[i]->mask,input_pos,input_len,input_num,&maxinp);
    ninp = contains_input(ute->macro_line[i]->new_name,input_pos,input_len,input_num,&maxinp);
  }
  return(maxinp);
}

void initialize_userf()
{
  userf_table.last = -1; // initialize the user function table
  def_AB_etc.func = strdup("default");
  def_AB_etc.ifunc = -1;
  def_AB_etc.reg_a = strdup("");
  def_AB_etc.reg_b = strdup("");
  def_AB_etc.mask = strdup("");
  def_AB_etc.mvalue = strdup("1");
  def_AB_etc.args = strdup("");
  def_AB_etc.new_name = strdup("");
}


void create_new_userf()
{
  userf_table_entry *ute;

  userf_table.last++;
  ute = malloc(sizeof(userf_table_entry));
  if (!ute) {
           printf("mstest create_new_userf:CANNOT ALLOCATE MEMORY!!!\n");
           exit(-1);
         }
  userf_table.entry[userf_table.last] = ute; 
  ute->name = strdup("new");
  ute->info = strdup("Not implemented yet");
  ute->last = -1;
  update_userf(userf_table.last);
  fl_addto_browser(fd_DV_mstest_item1_window->mstest_item1_func,ute->name);
  fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,userf_table.last+1);
}

void cut_space(char *str,char *rstr)
{
  int i,i1,i2,flag;
  char ch;
  int ltrace=0;
  
  flag = 0;
  i1=-1;
  i2=-1;
  IFL printf("cut_space !%s!\n",str);
  for(i=0;i<strlen(str);i++){
    ch = str[i];
    if (isspace(ch) && (flag == 0))  {
      i1=i;
    }
    else if (!isspace(ch)){
      i2=i;
      flag = 1;
    }
  }
  IFL printf("cut_space %d %d\n",i1,i2);
  for (i=i1+1;i<=i2;i++) {
    rstr[i-i1-1] = str[i];
  }
  if (i2<0) 
    rstr[0] = '\0';
  else 
    rstr[i2-i1] = '\0';
}


void save_AB_etc(int ufunc,int mline,int field)
/*
 saves the values of the field (AB_etc) into the structure
 -1 means save all
*/
{
  userf_table_entry *ute;

  if (ufunc<0 || mline<0) 
     return;
  ute = userf_table.entry[ufunc];
  switch (field) {
    case 0: ute->macro_line[mline]->reg_a = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_a));
            break; 
    case 1: ute->macro_line[mline]->reg_b = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_b));
            break;
    case 2: ute->macro_line[mline]->mask = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mask));
            break;
    case 3: ute->macro_line[mline]->mvalue = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mvalue));
            break;
    case 4: ute->macro_line[mline]->args = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_args));
            break;
    case 5: ute->macro_line[mline]->new_name = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_new));
            break;
    case 6: ute->name = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_name));
            break;
    case -1:ute->macro_line[mline]->reg_a = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_a));
            ute->macro_line[mline]->reg_b = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_b));
            ute->macro_line[mline]->mask = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mask));
            ute->macro_line[mline]->mvalue = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mvalue));
            ute->macro_line[mline]->args = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_args));
            ute->macro_line[mline]->new_name = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_new));
            ute->name = strdup((char *)fl_get_input(fd_DV_mstest_userfdef_window->mstest_userfdef_name));
            break;
  }
}

int get_func_indx(char *str)
{
  int i;
  i = 0;
  while(strlen(func_table[i].name)) {
    if (!strcmp(str,func_table[i].name)) {
      return(i);
    }
    i++;
  }
  printf("error get_func_indx: function %s is not in the table!!!\n");
}

void update_userf(int ufunc)
/*
updates all 
*/
{
  userf_table_entry *ute;

  ute = userf_table.entry[ufunc];
  fl_set_object_label(fd_DV_mstest_item1_window->mstest_item1_info,ute->info);
  fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_name,ute->name);
  fill_macro_code(ufunc);
  if (ute->last>=0) fill_AB_etc(ufunc,0);
  else fill_AB_etc(ufunc,-1);
}

void fill_userf_list()
{ 
  int last,i;
  int ltrace=0;
  userf_table_entry *ute;
  last = userf_table.last;
  IFL printf("mstest fill_userf: last= %d\n",last);
  fl_clear_browser(fd_DV_mstest_item1_window->mstest_item1_func);
  for (i=0;i<=last;i++){
    ute = userf_table.entry[i];
    fl_add_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,ute->name); 
  }
}

void fill_macro_code(int ufunc)
{
  userf_table_entry *ute;
  int i;
  int ltrace=0;
  ute = userf_table.entry[ufunc];
  IFL printf("mstest fill_macro_code: %d\n",ute->last);
  if (ute->last<0) {
     fl_clear_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro);
     return;
  }
  fl_clear_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro);
  for (i=0;i<=ute->last;i++) {
    fl_addto_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,ute->macro_line[i]->func);
  }
  fl_select_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,1);
}

void fill_AB_etc(int ufunc,int mline)
{
  int ltrace=0;
  userf_macro_line *ufml;
  IFL printf("mstest fill_AB_etc ufunc=%d mline=%d\n",ufunc,mline);
  if (mline>=0) {
    ufml = userf_table.entry[ufunc]->macro_line[mline];
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_a,ufml->reg_a);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_b,ufml->reg_b);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mask,ufml->mask);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mvalue,ufml->mvalue);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_args,ufml->args);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_new,ufml->new_name);
  }
  else {
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_a,def_AB_etc.reg_a);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_b,def_AB_etc.reg_b);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mask,def_AB_etc.mask);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mvalue,def_AB_etc.mvalue);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_args,def_AB_etc.args);
    fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_new,def_AB_etc.new_name);
  }
}

/*** callbacks and freeobj handles for form DV_mstest_item1_window ***/

void mstest_item1_browser_cb(FL_OBJECT *obj, long data)
{

 int ltrace=0;
   int mbutton,line;
   char *name;

   IFL printf("in mstest_item1_browser_cb:\n");

   line=abs(fl_get_browser(obj));
   name=(char *)fl_get_browser_line(obj,line);

   mbutton=fl_mouse_button();
   IFL printf("mouse button line=%d %d\n",line,mbutton);
   if (mbutton==FL_RIGHT_MOUSE) {
      fl_deselect_browser(obj);
      fl_select_browser_line(obj,line);
      fl_addto_browser(fd_DV_mstest_item1_window->mstest_item1_input,name);
   }
   if (fl_get_browser_maxline(fd_DV_mstest_item1_window->mstest_item1_input)==1)
     fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_input,1);
}

void mstest_item1_add_cb(FL_OBJECT *ob, long data)
{
  int i,lline,lline2,added;
  FL_OBJECT *bl=fd_DV_mstest_item1_window->mstest_item1_browser;

  added = 0;
  lline = fl_get_browser_maxline(bl);
  for (i=1;i<=lline;i++) {
    if (fl_isselected_browser_line(bl,i)) {
      fl_addto_browser(fd_DV_mstest_item1_window->mstest_item1_input,(char *)fl_get_browser_line(bl,i));
      added = 1;
    }
  }
  if (added)
    if (!fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_input))
      fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_input,1);
}


int  mstest_item1_close_cb(FL_FORM* form, void *data)
{
   int ltrace=0;
   IFL printf("in mstest_item1_close_cb\n");
   fl_hide_form(form);
   return 1;
}

void mstest_item1_go_cb(FL_OBJECT *ob, long data)
{
  int fline,line,lline,ninp_regs,gomode,del,j;
  char *mssg;
  int ltrace=0;
  mssg = calloc(100,sizeof(char));
  fline = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func); 
  line = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_input); 
  lline = fl_get_browser_maxline(fd_DV_mstest_item1_window->mstest_item1_input);
  ninp_regs = userf_table.entry[fline-1]->ninput_regs;
  IFL printf("mstest mstest_item1_go_cb %d %d %d\n",fline,line,lline);
  if ((line==0) && (lline>0)) {
    fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_input,1); 
    line = 1;
  }
  if (lline<(line+ninp_regs-1)) {
     if (ninp_regs==1)
       sprintf(mssg,"1 input register is needed\n");
     else
       sprintf(mssg,"%d input registers are needed\n",ninp_regs);
     fl_show_messages(mssg);
     return;
  }
  IFL printf("mstest mstest_item1_go_cb %d %d %d\n",fline,line,lline);
  execute_userf(fline,line,ninp_regs); 

  del = fl_get_button(fd_DV_mstest_item1_window->mstest_item1_godelmode);
  gomode = fl_get_button(fd_DV_mstest_item1_window->mstest_item1_gomode);

  if (del) {
   for (j=0;j<ninp_regs;j++) 
     delete_item1_input_line(line-1); 
  }
  if (del) 
    lline -= ninp_regs;
  else 
    line += ninp_regs;

  if (gomode) {
    while (lline>=(line+ninp_regs-1)) {
      fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_input,line);
      execute_userf(fline,line,ninp_regs);
      if (del) {
        for (j=0;j<ninp_regs;j++) 
         delete_item1_input_line(line-1);
      }
      if (del) 
        lline -= ninp_regs;
      else 
        line += ninp_regs;
    }
  }
  if (lline>=line)
    fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_input,line);
    
  free(mssg); 
}

void execute_userf(int fline,int line,int ninp_regs)
{
  userf_table_entry *ute;
  userf_macro_line *ufml;
  int i,dummy;
  char *regstr,*backup_filter;
  int ltrace=0;

  ute = userf_table.entry[fline-1];
  regstr = calloc(MAX_USERF_LINE_LENGTH,sizeof(char));
  backup_filter = calloc(sizeof(char),256);
  strcpy(backup_filter,fl_get_input(fd_DV_func_window->givec));
  fl_set_input(fd_DV_func_window->givec,fl_get_input(fd_DV_mstest_item1_window->filter));
  for (i=0;i<=ute->last;i++) {
    ufml = ute->macro_line[i];
    fl_select_browser_line(fd_DV_func_window->func,ufml->ifunc+1);
    subs_regs(regstr,ufml->reg_a,line);
    fl_set_input(fd_DV_func_window->func_reg_a,regstr);
    subs_regs(regstr,ufml->reg_b,line);
    fl_set_input(fd_DV_func_window->func_reg_b,regstr);
    subs_regs(regstr,ufml->mask,line);
    fl_set_input(fd_DV_func_window->func_mask,regstr);
    fl_set_input(fd_DV_func_window->func_mask_val,ufml->mvalue);
    fl_set_input(fd_DV_func_window->func_args,ufml->args);
    subs_regs(regstr,ufml->new_name,line);
    IFL printf("mstest ------------ !%s!\n",regstr);
    if (strcmp(regstr,"\0")==0) {
      IFL  printf("mstest default new register name\n");
    }
    else 
      fl_set_input(fd_DV_func_window->func_new_reg,regstr);
    func_go_cb(fd_DV_func_window->func_go,(long) 0);
  }
  IFL printf("mstest resetting filter to %s\n",backup_filter);
  fl_set_input(fd_DV_func_window->givec,backup_filter);
  free(backup_filter);
   
}

void subs_regs(char *regstr,char *str,int line)
{
  char *rg;
  int input_pos[MAX_USERF_INPUT_PARAMS];
  int input_len[MAX_USERF_INPUT_PARAMS];
  int input_num[MAX_USERF_INPUT_PARAMS],ninp,i,j,k,l,pos,maxinp;
  int ltrace=0;
  maxinp = 0;
  ninp = contains_input(str,input_pos,input_len,input_num,&maxinp);
  pos=0;
  l = 0;
  for (j=0;j<ninp;j++) {
    for (k=pos;k<input_pos[j];k++)
      regstr[l++] = str[k];
    rg = &((char *)fl_get_browser_line(fd_DV_mstest_item1_window->mstest_item1_input,line+input_num[j]-1))[2];
    IFL printf("mstest subs_regs %s line=%d\n",rg,line);
    for (k=0;k<strlen(rg);k++)
      regstr[l++] = rg[k];
    pos = input_pos[j]+input_len[j];
  }
  for (k=pos;k<strlen(str);k++)
    regstr[l++] = str[k];
  regstr[l] = '\0';
}



void mstest_item1_new_cb(FL_OBJECT *ob, long data)
/* 
data=0 edit
data=1 new
*/
{
  int ltrace=0;

   IFL printf("in mstest_item1_new_cb:\n");
// backup the function so it can be restored if changes are not applied
   if (data) {// new user function
      IFL printf("mstest Creating new user function\n");
      restore_userf(last_backed_up);
      create_new_userf();
      backup_userf(userf_table.last);
      userf_mode = USERF_NEW;
   }
   else
      userf_mode = USERF_NEW;

   fl_show_form(fd_DV_mstest_userfdef_window->DV_mstest_userfdef_window,FL_PLACE_CENTERFREE,FL_FULLBORDER,"User function definitions");
   fl_set_form_minsize(fd_DV_mstest_userfdef_window->DV_mstest_userfdef_window,500,400);
   XRaiseWindow(fl_display,fd_DV_mstest_userfdef_window->DV_mstest_userfdef_window->window);
}



void mstest_item1_delete_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
  int line,selected_line;
  line = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
  IFL printf("mstest in mstest_item1_delete_cb: line=%d\n",line);
  if (line) {
    selected_line = delete_userf_line(line-1);
    if (selected_line>=0) {
      update_userf(selected_line);
      backup_userf(selected_line);
    }
    else {
      last_backed_up = -1;
      mstest_item1_close_cb(fd_DV_mstest_userfdef_window->DV_mstest_userfdef_window,(long) 0);
    }
  }

}

void mstest_item1_up_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
  int line;
  line = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
  IFL printf("mstest in mstest_item1_up_cb: line=%d\n",line);
  if (line>1) {
    last_backed_up--;
    switch_userf_lines(line-2,line-1,line-2);
  }


}

void mstest_item1_down_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
  int line,maxline;
  line = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
  maxline = fl_get_browser_maxline(fd_DV_mstest_item1_window->mstest_item1_func);
  if (line<maxline) {
    last_backed_up++;
    switch_userf_lines(line,line-1,line);
  }

}

void mstest_item1_input_cb(FL_OBJECT *ob, long data)
{
  /* fill-in code for callback */
}



void mstest_item1_input_remove_cb(FL_OBJECT *ob, long data)
{
  int delmode,fline,i,ninp_regs,lline,line;
  line = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_input);
  lline = fl_get_browser_maxline(fd_DV_mstest_item1_window->mstest_item1_input);
  delmode = fl_get_button(fd_DV_mstest_item1_window->mstest_item1_delmode); 
  if (delmode==0) {
    if (line>0) delete_item1_input_line(line-1);
    return;
  }
  else {
    fline = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
    if (fline==0) return;
    if (lline>=(line+userf_table.entry[fline-1]->ninput_regs-1))
      for (i=0;i<userf_table.entry[fline-1]->ninput_regs;i++)
        delete_item1_input_line(line-1);
    else
      for (i=line;i<=lline;i++)
        delete_item1_input_line(line-1);
  }
}

void mstest_item1_input_up_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
  int line;
  line = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_input);
  if (line>1)
    switch_item1_input_lines(line-2,line-1,line-2);

}

void mstest_item1_input_down_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
  int line,maxline;
  line = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_input);
  maxline = fl_get_browser_maxline(fd_DV_mstest_item1_window->mstest_item1_input);
  if (line<maxline && line>0)
    switch_item1_input_lines(line,line-1,line);

}




void mstest_item1_func_cb(FL_OBJECT *ob, long data)
{
  int line,ltrace=0,mbutton;
  mbutton=fl_mouse_button();
  line = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
  IFL printf("in mstest_item1_func_cb line=%d:\n",line);
//  if (mbutton==FL_LEFT_MOUSE) {
    restore_userf(last_backed_up);
    update_userf(line-1);
    backup_userf(line-1);
/*  }
  else if (mbutton==FL_RIGHT_MOUSE) {
    if (line) 
      fl_deselect_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line);
  }
*/
}







/*** callbacks and freeobj handles for form DV_mstest_userfdef_window ***/

int  mstest_userfdef_close_cb(FL_FORM* form, void *data)
{
   int ltrace=0;
   IFL printf("in mstest_item1_close_cb\n");
   fl_hide_form(form);
   return 1;
}


void mstest_userfdef_browser_cb(FL_OBJECT *obj, long data)
{
   int ltrace=0;
   int mbutton,line;
   char *name;

   IFL printf("in mstest_userfdef_browser_cb:\n");

   line=fl_get_browser(obj);
   name=&((char *)fl_get_browser_line(obj,line))[2];

   mbutton=fl_mouse_button();
   if (mbutton==FL_LEFT_MOUSE)
      fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_a,name);
   else if (mbutton==FL_MIDDLE_MOUSE)
      fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_reg_b,name);
   else
      fl_set_input(fd_DV_mstest_userfdef_window->mstest_userfdef_mask,name);

   //func_build_name();

}

void mstest_userfdef_AB_etc_cb(FL_OBJECT *ob, long data)
{
  int fline,mline;
  int ltrace=0;
  IFL printf("mstest mstest_userfdef_AB_etc_cb %d\n",data);
  fline = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
  if (data == 6)
     save_AB_etc(fline-1,0,(int) 6);  // second argument is irrelevant
  mline = fl_get_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro);
  save_AB_etc(fline-1,mline-1,(int) data); 
}

void mstest_userfdef_apply_cb(FL_OBJECT *ob, long data)
{
  int fline,mline;
  char *tstr;
  userf_table_entry *ute;
  tstr = calloc(MAX_USERF_INFO_LENGTH,sizeof(char));
  if (!tstr) {
    printf("mstest mstest_userfdef_apply_cb CANNOT ALLOCATE MEMORY!!!\n");
    exit(-1);
  }
  fline = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
  ute = userf_table.entry[fline-1];
  ute->ninput_regs = get_ninput(fline-1);
  sprintf(tstr,"Number of input registers: %d",ute->ninput_regs);
  ute->info = strdup(tstr);
  fl_replace_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,fline,ute->name);
  fl_set_object_label(fd_DV_mstest_item1_window->mstest_item1_info,tstr);
  backup_userf(fline-1); 
  free(tstr);
}

void mstest_userfdef_func_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
   int mbutton,line,i;
   char *name;

   IFL printf("in mstest_userfdef_func_cb:\n");

   line=fl_get_browser(ob);
   name=((char *)fl_get_browser_line(ob,line));
   i = get_func_indx(name);
   IFL printf("in mstest_userfdef_func_cb: line=%d name=%s\n",line,name);

   mbutton=fl_mouse_button();
   if (mbutton==FL_LEFT_MOUSE)
   add_macro_line(i);
}

void mstest_userfdef_macro_cb(FL_OBJECT *ob, long data)
{
  int fline,mline,ltrace=0,mbutton;
  IFL printf("in mstest_userf_macro_cb\n");
  mbutton=fl_mouse_button();
//   if (mbutton==FL_LEFT_MOUSE) {
     fline = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
     mline = fl_get_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro);
     fill_AB_etc(fline-1,mline-1);
//   }

}


void mstest_userfdef_delete_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
  int line;
  line = fl_get_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro);
  IFL printf("mstest in mstest_userfdef_delete_cb: line=%d\n",line);
  if (line) 
    delete_macro_line(line-1);
}

void mstest_userfdef_up_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
  int line;

  line = fl_get_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro);
  IFL printf("in mstest_userfdef_up_cb line=%d:\n",line);
  if (line>1) {
    switch_macro_lines(line-2,line-1,line-2);
  }

}

void mstest_userfdef_down_cb(FL_OBJECT *ob, long data)
{
  int ltrace=0;
  int line,maxline;

  line = fl_get_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro);
  maxline = fl_get_browser_maxline(fd_DV_mstest_userfdef_window->mstest_userfdef_macro);
  IFL printf("in mstest_userfdef_down_cb line=%d:\n",line);
  if (line<maxline) {
    switch_macro_lines(line,line-1,line);
  }
}

void switch_macro_lines(int line1,int line2, int select)
/* switches two macro lines line1 and line2 and updates
the internal structure. It selectes the line select
Note that line1,line2,select point to the macro_line
which starts at 0 so they must be incremented to 
*/
{
  int ltrace,fline;
  userf_macro_line *temp;
  userf_table_entry *ute;
  char *name1,*name2;

  fline = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
  ute = userf_table.entry[fline-1];
  temp = ute->macro_line[line1];
  ute->macro_line[line1] = ute->macro_line[line2];
  ute->macro_line[line2] = temp;
  name1=((char *)fl_get_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,line1+1));
  name2=((char *)fl_get_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,line2+1));
  fl_freeze_form(fd_DV_mstest_userfdef_window->mstest_userfdef_macro->form);
// insert line2 before line1 and delete line1
  fl_insert_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,line1+1,name2);
  fl_delete_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,line1+2);
// insert line1 before line2 and delete line2
  fl_insert_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,line2+1,name1);
  fl_delete_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,line2+2);
// select select line
  fl_select_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,select+1);
  fl_unfreeze_form(fd_DV_mstest_userfdef_window->mstest_userfdef_macro->form);
  fill_AB_etc(fline-1,select);
}

void delete_macro_line(int line)
/* deletes a line from macro 
and updates the structure
*/
{
  int ltrace,fline,lline,i;
  userf_table_entry *ute;

  fline = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
// update the structures (shift the pointers) 
  ute = userf_table.entry[fline-1];
  free_userf_macro_line(ute->macro_line[line]);
  for (i=line+1;i<=ute->last;i++) {
    ute->macro_line[i-1] = ute->macro_line[i];
  }
  ute->last--;
  fl_freeze_form(fd_DV_mstest_userfdef_window->mstest_userfdef_macro->form);
  fl_delete_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,line+1);
  lline = fl_get_browser_maxline(fd_DV_mstest_userfdef_window->mstest_userfdef_macro)-1;
  if (lline>=line) {
    fl_select_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,line+1);
    fill_AB_etc(fline-1,line);
  }
  else {
    fl_select_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,lline+1);
    fill_AB_etc(fline-1,lline);
  }
  fl_unfreeze_form(fd_DV_mstest_userfdef_window->mstest_userfdef_macro->form);
}

int delete_userf_line(int line)
/*
returns the position of selected userf
*/
{
  int ltrace,lline,i,sline;

// update the structures (shift the pointers)
  free_userf_table_entry(userf_table.entry[line]);
  for (i=line+1;i<=userf_table.last;i++) {
    userf_table.entry[i-1] = userf_table.entry[i];
  }
  userf_table.last--;
  fl_delete_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line+1);
  lline = fl_get_browser_maxline(fd_DV_mstest_item1_window->mstest_item1_func)-1;
  sline = -1;
  if (lline>=line) {
    fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line+1);
    sline = line;
  }
  else if (lline>=0) {
    fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,lline+1);
    sline = lline;
  }
  return(sline);
}

void switch_userf_lines(int line1,int line2, int select)
{
  int ltrace,fline;
  userf_table_entry *temp;
  char *name1,*name2;

  temp = userf_table.entry[line1];
  userf_table.entry[line1] = userf_table.entry[line2];
  userf_table.entry[line2] = temp;
  name1=(char *)fl_get_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line1+1);
  name2=(char *)fl_get_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line2+1);
  fl_freeze_form(fd_DV_mstest_item1_window->mstest_item1_func->form);
// insert line2 before line1 and delete line1
  fl_insert_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line1+1,name2);
  fl_delete_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line1+2);
// insert line1 before line2 and delete line2
  fl_insert_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line2+1,name1);
  fl_delete_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,line2+2);
// select select line
  fl_select_browser_line(fd_DV_mstest_item1_window->mstest_item1_func,select+1);
  fl_unfreeze_form(fd_DV_mstest_item1_window->mstest_item1_func->form);
}

void delete_item1_input_line(int line)
// line numbering starts from 0 - to be compatible with
// other delete calls that update some internal structures as well
{
  int ltrace,lline;
  FL_OBJECT *bl=fd_DV_mstest_item1_window->mstest_item1_input; 
  fl_delete_browser_line(bl,line+1);
  lline = fl_get_browser_maxline(bl)-1;
  if (lline>=line)
    fl_select_browser_line(bl,line+1);
  else if (lline>=0)
    fl_select_browser_line(bl,lline+1);
}

void switch_item1_input_lines(int line1,int line2, int select)
{
  int ltrace;
  char *name1,*name2;
  FL_OBJECT *bl=fd_DV_mstest_item1_window->mstest_item1_input;

  name1=(char *)fl_get_browser_line(bl,line1+1);
  name2=(char *)fl_get_browser_line(bl,line2+1);
  fl_freeze_form(bl->form);
// insert line2 before line1 and delete line1
  fl_insert_browser_line(bl,line1+1,name2);
  fl_delete_browser_line(bl,line1+2);
// insert line1 before line2 and delete line2
  fl_insert_browser_line(bl,line2+1,name1);
  fl_delete_browser_line(bl,line2+2);
// select select line
  fl_select_browser_line(bl,select+1);
  fl_unfreeze_form(bl->form);
}


void add_macro_line(int i)
/* adds a line to macro
and updates the structure
*/
{
  int fline,mline;
  userf_table_entry *ute;

  fline = fl_get_browser(fd_DV_mstest_item1_window->mstest_item1_func);
// update the structures (shift the pointers)
  ute = userf_table.entry[fline-1];
  ute->last++;
  mline = ute->last;
  ute->macro_line[mline] = malloc(sizeof(userf_macro_line));
  if (!ute) {
    printf("mstest add_macro_line:CANNOT ALLOCATE MEMORY!!!\n");
    exit(-1);
  }
 
  fl_addto_browser(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,func_table[i].name);
  fl_select_browser_line(fd_DV_mstest_userfdef_window->mstest_userfdef_macro,mline+1);
  fill_AB_etc(fline-1,-1); // fill with default
  save_AB_etc(fline-1,mline,-1);//save all defaults
  ute->macro_line[mline]->func=strdup((char *)func_table[i].name);
  ute->macro_line[mline]->ifunc=i;
}

