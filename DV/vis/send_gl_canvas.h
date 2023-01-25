#ifndef _SEND_GL_CANVAS_H
#define _SEND_GL_CANVAS_H

#include "fncs.h"
#include <forms.h>

typedef struct
{
   double x,y,z;
} vect;

typedef struct
{
   double r,g,b;
} color;

/* Note: the following 3 sets of #define where chosen
         to be consistent with xforms numbering of choice items,
         and are assumed to be thus in DV_gui.c */

#define CONCAT_NONE 1
#define CONCAT_TIME 2
#define CONCAT_ALL 3

#define SYNC_T_INDEX 1
#define SYNC_T_VALUE 2

#define CMAP_LIN 1
#define CMAP_LOGPM 2
#define CMAP_LOGP 3
#define CMAP_LOGM 4
#define CMAP_BW_CONTOUR 5

#define CMAP_STANDARD 1
#define CMAP_BW 2
#define CMAP_WB 3

#define SLICE_X 1
#define SLICE_Y 2
#define SLICE_Z 3

typedef struct grid_conversion_params
{
   double zscale;
   char *in_zscale; 
   double cmap_minz,cmap_maxz;
   int cmap_type;
   int cmap_scheme;
   char *in_cmap_minz,*in_cmap_maxz; 
   double zmin_clip,zmax_clip;
   char *in_zmin_clip,*in_zmax_clip; 
   int use_zmin_clip,use_zmax_clip;
   int concat_grids;
   int sync_t;
   int amr;
   int use_mask;
   char *mask;
   double mask_val;
   int use_givec;
   char *givec; 
   int coarsen_step;
   int ignore_ccoords;
   int monotone;
   int levelcolor;
   color fg_color;
   int do_slice;
   int slice;
   double slice_v;
   double min_dx,min_dy,min_dz;
   int bbox_only; /* for 3D */
   int color_clip; /* so far for 3D only */
   int spin_y;
} grid_conversion_params;


#define RENDER_MODE_POINT 1
#define RENDER_MODE_WIRE_FRAME 2
#define RENDER_MODE_LINEX 3
#define RENDER_MODE_LINEY 4
#define RENDER_MODE_LINEXY 5
#define RENDER_MODE_HIDDEN_LINE 6
#define RENDER_MODE_SOLID 7
#define RENDER_MODE_NONE 8
#define RENDER_MODE_VOLUMETRIC 9
#define RENDER_MODE_PARTICLES 10

typedef struct render_params
{
   int render_mode;
   int smooth;      /* smoothing on or off */
   int draw_bboxes;
   color bbox_color; 
   int bbox_width;
   int draw_slice_box;
   int use_spec;
   int pt_size_1d;
   int line_width_1d;
   float shininess;
   int use_key_frames;
   int anti_alias;
   color bg_color;
   int first;
} render_params;

typedef struct camera_params
{
   vect home_vp; /* home view point -- center of object (model coords) */
   vect bbox_ll,bbox_ur; /* object's bounding box (object coords) */
   double min_data,max_data; /* data extremes */

   double dispx,dispy; /* for interactive displacement */
   vect rot_axis;      /* for interactive rotation */
   double rot_angle; 
   double zoom;       

   double z_translate;
   double proj_w,hw_aspect_ratio,proj_near,proj_far;
   double new_z_translate;
   double new_proj_w,new_proj_near,new_proj_far;

   double t;         /* save info from most recent grid, if user wants to print it. */
   int shapex,shapey,shapez;
} camera_params;

void send_gl_canvas(int send_children, grid *only_g, char *r1_name, int to_explorer, int do_3d);
void refresh_gl_display(void);
void set_gl_render_mode(void);
void reset_gl_view(double zoom, double theta, double phi);

#define MAX_DISPLAY_LISTS 5000
#define GRID_DL_OFFSET 1
#define BBOX_DL_OFFSET 5001

#define FONT_DL_OFFSET 20001
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
   int which_canvas;
} os_text;

#define MAX_OS_TEXTS 32
extern os_text os_text_list[MAX_OS_TEXTS];
#define MAX_OS_TEXT_LENGTH 256

extern int capture_on;
extern int capture_n;
extern int capture_jpeg_qf;
extern const char *capture_dir;
extern const char *capture_file_name_fs;

typedef struct key_frame
{
   struct key_frame *next;
   struct key_frame *prev;
   double t;
   double zoom;
   double view_mat[16];
} key_frame;

extern key_frame *key_frames;

void set_key_frame(void);
void clear_key_frame(void);
void clear_all_key_frames(void);
int is_key_frame(void);

void build_cmap(void);

/*-----------------------------------------------------------------------------
   the following set of global variables control the rendering
   options, canvas selection, etc.
  -----------------------------------------------------------------------------*/

extern grid_conversion_params gc_opts;
extern render_params rend_opts;
extern camera_params cam_opts;
extern int *grid_dl,*bbox_dl;
extern int n_dl,dl_view_offset;
extern double last_model_mat[16];
extern vect os_cmap_p1,os_cmap_p2;
extern color os_cmap_bcol;
extern int os_cmap_bwidth,show_os_cmap,os_cmap_image_first;
extern char *os_cmap_image;
extern int os_cmap_size_x,os_cmap_size_y;
extern int prev_os_bwidth;
extern color prev_os_bcol;
extern double prev_os_x1,prev_os_y1;
extern int prev_os_cmap_type;
extern int prev_os_cmap_scheme;

#endif /* _SEND_GL_CANVAS_H */
