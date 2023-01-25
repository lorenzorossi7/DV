#ifndef _SEND_GL_CANVAS_H
#define _SEND_GL_CANVAS_H

#include "fncs.h"

typedef struct
{
   double x,y,z;
} vect;

typedef struct
{
   double r,g,b;
} color;


// NOTE: all the const char *'s below point to xform's internal buffers. 
// so do *not* modify the text that they point to.

typedef struct
{
   double zscale;
   const char *in_zscale; 
   double cmap_minz,cmap_maxz;
   const char *in_cmap_minz,*in_cmap_maxz; 
   double zmin_clip,zmax_clip;
   const char *in_zmin_clip,*in_zmax_clip; 
   int use_zmin_clip,use_zmax_clip;
   int concat_grids;
   int amr;
   int use_mask;
   const char *mask;
   double mask_val;
   int use_filter;
   const char *filter_args; 
   UNARY_FNC *filter;
   int coarsen_step;
} grid_conversion_params;

extern grid_conversion_params gc_opts;

#define RENDER_MODE_POINT 1
#define RENDER_MODE_WIRE_FRAME 2
#define RENDER_MODE_HIDDEN_LINE 3
#define RENDER_MODE_SOLID 4

typedef struct
{
   int render_mode;
   int smooth;      // smoothing on or off
   int draw_bboxes;
   int freeze_lens; // freeze current perspective transform options
   color bbox_color; 
   int bbox_width;
} render_params;

extern render_params rend_opts;

typedef struct
{
   vect home_vp; // home view point -- center of object (model coords)
   vect bbox_ll,bbox_ur; // object's bounding box (object coords)

   double dispx,dispy; // for interactive displacement
   vect rot_axis;      // for interactive rotation
   double rot_angle;
   double zoom;       

   double z_translate;
   double proj_w,hw_aspect_ratio,proj_near,proj_far;

   double t;         // save info from most recent grid, if user wants to print it.
   int shapex,shapey;
} camera_params;

extern camera_params cam_opts;

void send_gl_canvas(int send_children, grid *only_g, char *r1_name);
void refresh_gl_display(void);
void set_gl_render_mode(void);
void reset_gl_view(void);

#define MAX_DISPLAY_LISTS 1000
#define GRID_DL_OFFSET 1
#define BBOX_DL_OFFSET 1001

#define FONT_DL_OFFSET 10000
#define NUM_FONTS 20
#define FONT_SIZE 256
extern char *xfont_names[NUM_FONTS];

typedef struct
{
   char *format_str;
   char *print_str;
   vect pos;
   color col;
   int col_ind;
   int font;
} os_text;

#define MAX_OS_TEXTS 32
extern os_text os_text_list[MAX_OS_TEXTS];
#define MAX_OS_TEXT_LENGTH 256

extern int n_dl;

extern int capture_on;
extern int capture_n;
extern int capture_jpeg_qf;
extern const char *capture_dir;
extern const char *capture_file_name_fs;

// for on-screen colormap scale:
extern vect os_cmap_p1,os_cmap_p2; 
extern color os_cmap_bcol;
extern int os_cmap_bwidth,show_os_cmap;

#endif // _SEND_GL_CANVAS_H
