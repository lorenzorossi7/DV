/*
   send_gl_canvas.c: to send output to the GL canvas

   NOTE: these routines assume that the GL canvas is open
*/

#include "DVault.h"
#include "reg.h"
#include "../misc.h"
#include <stdio.h>
#include "sdf.h"
#include "sdf_priv.h"
#include "cliser.h"
#ifndef DARWIN
#include <malloc.h>
#endif 
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
//#include "send_gl_canvas.h"
#include "DV_gui.h"
#include "ncar_out.h"
#include <math.h>

//-----------------------------------------------------------------------------
// NOTE: any additional global variables added here that affect rendering,
//       grid conversion, etc., must be added to the corresponding '_s'
//       variables/routines/etc in DV_gui.c, so that it can properly
//       handle multiple canvas objects.
//-----------------------------------------------------------------------------

grid_conversion_params gc_opts;
render_params rend_opts;
camera_params cam_opts;

int *grid_dl=0; // calling routine must allocate MAX_DISPLAY_LISTS of these
int *bbox_dl=0; // "
int n_dl=0;   // number of current display lists
int dl_view_offset=0;   // for multiple views

double last_model_mat[16];
//-----------------------------------------------------------------------------
// on-screen colormap variables
//-----------------------------------------------------------------------------
vect os_cmap_p1={10,10,0},os_cmap_p2={20,100,0};
color os_cmap_bcol={0,0,0};
int os_cmap_bwidth=1,show_os_cmap=0,os_cmap_image_first=1; 
char *os_cmap_image=0;
int os_cmap_size_x,os_cmap_size_y;
int prev_os_bwidth;
color prev_os_bcol;
double prev_os_x1,prev_os_y1;
int prev_os_cmap_type;
int prev_os_cmap_scheme;

//-----------------------------------------------------------------------------
// for screen capture
//-----------------------------------------------------------------------------
int capture_on=0;
int capture_n=0;
int capture_jpeg_qf=100;
const char *capture_dir=0;
const char *capture_file_name_fs=0; 

//-----------------------------------------------------------------------------
// send_gl_canvas_1g mode arguments:
//-----------------------------------------------------------------------------
#define SEND_GL_NEW 0
#define SEND_GL_APPEND 1 

//-----------------------------------------------------------------------------
// for speed we pre-calculate the colormap, and store the results in a
// look-up table.
// for the look-up functions, f is required to be between 0 and 1
//-----------------------------------------------------------------------------
#define CMAP_SIZE 1024
double cmap_lu_r[CMAP_SIZE],cmap_lu_g[CMAP_SIZE],cmap_lu_b[CMAP_SIZE];

#define cmap_r(f) (cmap_lu_r[(int)((f)*(CMAP_SIZE-1))])
#define cmap_g(f) (cmap_lu_g[(int)((f)*(CMAP_SIZE-1))])
#define cmap_b(f) (cmap_lu_b[(int)((f)*(CMAP_SIZE-1))])

//-----------------------------------------------------------------------------
// cheesy key-frame based animation system
//-----------------------------------------------------------------------------
key_frame *key_frames=0;
extern FD_DV_gl_canvas_window *fd_DV_gl_canvas_window;

void set_key_frame()
{
   key_frame *p,*q,*pq;
   int i;

   if (!(p=(key_frame *)malloc(sizeof(struct key_frame))))
   {
      printf("set_key_frame: malloc error\n"); return;
   }

   fl_set_object_label(fd_DV_gl_canvas_window->key_frame_ind,"*");

   p->next=p->prev=0;
   p->t=cam_opts.t;
   p->zoom=cam_opts.zoom;
   for (i=0;i<16;i++) p->view_mat[i]=last_model_mat[i];

   // 1 KF
   if (!key_frames) { key_frames=p; return; }

   q=key_frames;
   while(q && fuzz_lt(q->t,p->t,1.0e-10)) {pq=q; q=q->next;}

   // KF comes at end of list
   if (!q) { pq->next=p; p->prev=pq; return; } 

   // KF replaces exising one
   if (fuzz_eq(q->t,p->t,1.0e-10)) 
   {
      p->next=q->next;
      p->prev=q->prev;
      if (p->next) p->next->prev=p;
      if (p->prev) p->prev->next=p;
      if (key_frames==q) key_frames=p;
      free(q);
      return;
   }

   // KF is inserted into list before q
   p->next=q;
   p->prev=q->prev;
   if (q->prev) q->prev->next=p; else key_frames=p;
   q->prev=p;
}

void clear_all_key_frames()
{
   key_frame *q,*p=key_frames;

   while(p) {q=p; p=p->next; free(q);}

   key_frames=0;
   fl_set_object_label(fd_DV_gl_canvas_window->key_frame_ind," ");
}

void clear_key_frame()
{
   key_frame *p=key_frames;

   while(p && (!(fuzz_eq(p->t,cam_opts.t,1.0e-10)))) p=p->next;

   if (p)
   {
      if (p->prev) p->prev->next=p->next; else key_frames=p->next;
      if (p->next) p->next->prev=p->prev;
      free(p);
   }
   fl_set_object_label(fd_DV_gl_canvas_window->key_frame_ind," ");
}

int is_key_frame()
{
   key_frame *p=key_frames;

   while(p && (!(fuzz_eq(p->t,cam_opts.t,1.0e-10)))) p=p->next;

   if (p) return 1; else return 0;
}

//-----------------------------------------------------------------------------
// simple colormap functions --- f is between zero and 1
//-----------------------------------------------------------------------------
void colormap(double f,double *r,double *g,double *b)
{
   switch(gc_opts.cmap_scheme)
   {
      case CMAP_STANDARD:
         if(f<0.6) *r=-atan(20*f-6)/3+0.5; else *r=1+2*atan(10*(f-1))/3;
         *b=pow(atan(10*f-5)/2.8+0.5,2);
         if(f<0.4) *g=sqrt(fabs(atan(10*f-1.5)/2.2+0.5)); else *g=1-(atan(10*(f-.75))/3.0+0.45);
         *r=min(1,*r); *r=max(0,*r);
         *g=min(1,*g); *g=max(0,*g);
         *b=min(1,*b); *b=max(0,*b);
         break;
      case CMAP_BW:
         *r=*g=*b=f;
         *r=min(1,*r); *r=max(0,*r);
         *g=min(1,*g); *g=max(0,*g);
         *b=min(1,*b); *b=max(0,*b);
         break;
      case CMAP_WB:
         *r=*g=*b=1-f;
         *r=min(1,*r); *r=max(0,*r);
         *g=min(1,*g); *g=max(0,*g);
         *b=min(1,*b); *b=max(0,*b);
         break;
   }
}

void build_cmap()
{
   int i;

   for (i=0;i<CMAP_SIZE;i++) 
      colormap(((double)i)/(CMAP_SIZE-1),&cmap_lu_r[i],&cmap_lu_g[i],&cmap_lu_b[i]);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void set_gl_render_mode(void)
{
   float spec_on[4]={1,1,1,1},spec_off[4]={0,0,0,1};
   float zero=0;

   glEnable(GL_COLOR_MATERIAL); // so that setting the vertex color also sets material color
   glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_DEPTH_TEST);
   glDisable(GL_LIGHTING);
   glDisable(GL_BLEND);
   switch(rend_opts.render_mode)
   {
      case RENDER_MODE_PARTICLES:
      case RENDER_MODE_POINT:
         glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
         break;
      case RENDER_MODE_WIRE_FRAME:
         glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
         glDisable(GL_DEPTH_TEST);
         break;
      case RENDER_MODE_HIDDEN_LINE:
         glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
         break;
      case RENDER_MODE_VOLUMETRIC:
         glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
         glEnable(GL_LINE_SMOOTH); 
         glEnable(GL_BLEND); 
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
         break;
      case RENDER_MODE_SOLID:
         glEnable(GL_LIGHTING);
         glEnable(GL_LIGHT0);
         glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
         if (rend_opts.use_spec)
         {
            glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,spec_on);
            glMaterialfv(GL_FRONT,GL_SHININESS,&rend_opts.shininess);
         }
         else
         {
            glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,spec_off);
            glMaterialfv(GL_FRONT,GL_SHININESS,&zero);
         }
         break;
   }

   if (rend_opts.smooth)
      glShadeModel(GL_SMOOTH);
   else
      glShadeModel(GL_FLAT);
}

//-----------------------------------------------------------------------------
// creates on-screen colormap image. 
//-----------------------------------------------------------------------------
void create_os_cmap()
{
   int x1,y1,x2,y2,i,j,size_i,size_x,size_y; 
   char *p;
   double f;

   x1=min(os_cmap_p1.x,os_cmap_p2.x);
   x2=max(os_cmap_p1.x,os_cmap_p2.x);
   y1=min(os_cmap_p1.y,os_cmap_p2.y);
   y2=max(os_cmap_p1.y,os_cmap_p2.y);
   size_x=x2-x1;
   size_y=y2-y1;

   if (!os_cmap_image_first && size_x==os_cmap_size_x && size_y==os_cmap_size_y && 
       prev_os_bwidth==os_cmap_bwidth && prev_os_bcol.r==os_cmap_bcol.r &&
       prev_os_bcol.g==os_cmap_bcol.g && prev_os_bcol.b==os_cmap_bcol.b &&
       prev_os_x1==x1 && prev_os_y1==y1 &&
       os_cmap_image && prev_os_cmap_type==gc_opts.cmap_type &&
       prev_os_cmap_scheme==gc_opts.cmap_scheme) return;

   prev_os_cmap_type=gc_opts.cmap_type;
   prev_os_cmap_scheme=gc_opts.cmap_scheme;

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   os_cmap_image_first=0;
   prev_os_bwidth=os_cmap_bwidth;
   prev_os_bcol.r=os_cmap_bcol.r;
   prev_os_bcol.g=os_cmap_bcol.g;
   prev_os_bcol.b=os_cmap_bcol.b;
   prev_os_x1=x1;
   prev_os_y1=y1;
   os_cmap_size_x=x2-x1;
   os_cmap_size_y=y2-y1;
   if (os_cmap_image) free(os_cmap_image);
   size_i=size_x*size_y*3;
   if (!(os_cmap_image=(char *)malloc(sizeof(char)*size_i)))
   {
      printf("create_os_cmap: ERROR ... out of memory\n");
      return;
   }

   p=os_cmap_image;
   for(j=0;j<size_y;j++)
      for(i=0;i<size_x;i++)
      {
         if (j<os_cmap_bwidth || i<os_cmap_bwidth || 
             (size_x-i)<=os_cmap_bwidth || (size_y-j)<=os_cmap_bwidth)
         {
            *p++=os_cmap_bcol.r*255;
            *p++=os_cmap_bcol.g*255;
            *p++=os_cmap_bcol.b*255;
         }
         else
         {
            if ((y2-y1)>(x2-x1))
            {
               f=(double)(j-os_cmap_bwidth)/(size_y-2*os_cmap_bwidth);
               if (os_cmap_p1.y>os_cmap_p2.y) f=1-f;
            }
            else
            {
               f=(double)(i-os_cmap_bwidth)/(size_x-2*os_cmap_bwidth);
               if (os_cmap_p1.x>os_cmap_p2.x) f=1-f;
            }
            switch(gc_opts.cmap_type)
            {
               case CMAP_LOGPM:
                  if (f>0.5) f=0.5+log10((f-0.5)*198+1)/4;
                  else f=0.5-log10((0.5-f)*198+1)/4;
                  break;
               case CMAP_LOGP:
                  f=log10(f*99+1)/2;
                  break;
               case CMAP_LOGM:
                  f=log10((1-f)*99+1)/2;
                  break;
               case CMAP_BW_CONTOUR:
                  if (f<0.5) f=0; else f=1;
                  break;
            }
            *p++=cmap_r(f)*255;
            *p++=cmap_g(f)*255;
            *p++=cmap_b(f)*255;
         }
      }
}

//-----------------------------------------------------------------------------
// on-screen text support
//-----------------------------------------------------------------------------
os_text os_text_list[MAX_OS_TEXTS];

// use command line 'xlsfonts' to see available fonts,
// and 'xfd' to view a font

char *xfont_names[NUM_FONTS]=
{
   "*Times*medium-r*12*",
   "*Times*medium-r*18*",
   "*Times*medium-r*24*",
   "*Times*medium-r*34*",
   "*Times*bold-r*12*",
   "*Times*bold-r*18*",
   "*Times*bold-r*24*",
   "*Times*bold-r*34*",
   "*Times*medium-i*12*",
   "*Times*medium-i*18*",
   "*Times*medium-i*24*",
   "*Times*medium-i*34*",
   "*Times*bold-i*12*",
   "*Times*bold-i*18*",
   "*Times*bold-i*24*",
   "*Times*bold-i*34*",
   "*symbol*12*",
   "*symbol*18*",
   "*symbol*24*",
   "*symbol*34*",
};

//-----------------------------------------------------------------------------
// converts a format string(format_s) to the actual print string(print_s)
// The size of ps is assumed to be large enough to contain the final string.
//-----------------------------------------------------------------------------
void parse_os_text(char *format_s, char *print_s)
{
   char *fs=format_s,*ps=print_s,*b;
   char sbuf[256];
   char pbuf[256];
   int done=0,ret,n=1,slice;
   double var;

   while(!done)
   {
      while(*fs && *fs!='$' && n<MAX_OS_TEXT_LENGTH) { *ps++=*fs++; n++; }
      if (*fs && n<MAX_OS_TEXT_LENGTH)
      {
         fs++;
         pbuf[0]=0;
         sbuf[0]='%'; b=&sbuf[1];

         slice=0;

         if (!(strncmp(fs,"t",1)))
         { fs++; var=cam_opts.t; }
         else if (!(strncmp(fs,"zscale",6)))
         { fs+=6; var=gc_opts.zscale; }
         else if (!(strncmp(fs,"xmin",4)))
         { fs+=4; var=cam_opts.bbox_ll.x; }
         else if (!(strncmp(fs,"ymin",4)))
         { fs+=4; var=cam_opts.bbox_ll.y; }
         else if (!(strncmp(fs,"zmin",4)))
         { fs+=4; var=cam_opts.bbox_ll.z; }
         else if (!(strncmp(fs,"xmax",4)))
         { fs+=4; var=cam_opts.bbox_ur.x; }
         else if (!(strncmp(fs,"ymax",4)))
         { fs+=4; var=cam_opts.bbox_ur.y; }
         else if (!(strncmp(fs,"zmax",4)))
         { fs+=4; var=cam_opts.bbox_ur.z; }
         else if (!(strncmp(fs,"cmin",4)))
         { fs+=4; var=gc_opts.cmap_minz; }
         else if (!(strncmp(fs,"cmax",4)))
         { fs+=4; var=gc_opts.cmap_maxz; }
         else if (!(strncmp(fs,"clip_zmin",9)))
         { fs+=9; var=gc_opts.zmin_clip; }
         else if (!(strncmp(fs,"clip_zmax",9)))
         { fs+=9; var=gc_opts.zmax_clip; }
         else if (!(strncmp(fs,"shapex",6)))
         { fs+=6; var=cam_opts.shapex; }
         else if (!(strncmp(fs,"shapey",6)))
         { fs+=6; var=cam_opts.shapey; }
         else if (!(strncmp(fs,"shapez",6)))
         { fs+=6; var=cam_opts.shapez; }
         else if (!(strncmp(fs,"slice",5)))
         { fs+=5; slice=1;}
         else
         {
            printf("parse_os_text: undefined variable $%s\n",fs);
            var=0;
         }

         if (!slice)
         {
            while((*fs>='0' && *fs<='9') || *fs=='.') *b++=*fs++;
            if (*fs=='f' || *fs=='e') *b++=*fs++; 
            *b++=0;
            sprintf(pbuf,sbuf,var);
         }
         else 
         {
            if (gc_opts.slice == SLICE_X) sprintf(pbuf,"X");
            else if (gc_opts.slice == SLICE_Y) sprintf(pbuf,"Y");
            else sprintf(pbuf,"Z");
         }
         if ((strlen(pbuf)+n)<MAX_OS_TEXT_LENGTH)
         {
            strcpy(ps,pbuf);
            ps+=strlen(pbuf);
            n+=strlen(pbuf);
         }
      }
      else
      {
         done=1;
         *ps++=0;
      }
   }
}

//-----------------------------------------------------------------------------
// renders the string ost->print_str to the gl canvas
//-----------------------------------------------------------------------------
void render_string(os_text *ost)
{
   char *p;

   glColor3d(ost->col.r,ost->col.g,ost->col.b);
   glRasterPos2d(ost->pos.x,ost->pos.y);
   p=ost->print_str; if (!p) return;
   while(*p) glCallList(ost->font*FONT_SIZE+FONT_DL_OFFSET+*p++);
} 

//-----------------------------------------------------------------------------
// renders all text and the colormap to the canvas
//-----------------------------------------------------------------------------
void render_os_stuff()
{
   int i,vp[4];
   char *fs,*ps;

   glDisable(GL_LIGHTING); 
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CLIP_PLANE0);
   glDisable(GL_CLIP_PLANE1);
   glPushMatrix();

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glGetIntegerv(GL_VIEWPORT,vp); // [vp]=[x,y,width,height] 
   glOrtho(vp[0],vp[0]+vp[2],vp[1],vp[1]+vp[3],-1,1);

   for(i=0;i<MAX_OS_TEXTS;i++)
   {
      if ((fs=os_text_list[i].format_str) && strlen(fs) && 
          (os_text_list[i].which_canvas==curr_gl_canvas))
      {
         if (!(ps=os_text_list[i].print_str))
         {
            if (!(ps=(char *)malloc(sizeof(char)*MAX_OS_TEXT_LENGTH)))
            {
               printf("render_os_stuff: ERROR ... out of memory\n");
               goto cleanup;
            }
            os_text_list[i].print_str=ps;
            parse_os_text(fs,ps);
         }
         render_string(&os_text_list[i]);
      }
   }

   if (show_os_cmap)
   {
      create_os_cmap();
      if (os_cmap_image)
      {
         glRasterPos2d(min(os_cmap_p1.x,os_cmap_p2.x),min(os_cmap_p1.y,os_cmap_p2.y));
         glDrawPixels(os_cmap_size_x,os_cmap_size_y,GL_RGB,GL_UNSIGNED_BYTE,os_cmap_image);
      }
   }

cleanup:
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   set_gl_render_mode();
}

void init_xfonts(void)
{
   int i;
   XFontStruct *xfs;

   for (i=0;i<NUM_FONTS;i++)
   {
      if (!(xfs=XLoadQueryFont(fl_display,xfont_names[i])))
      {
         printf("init_xfonts: WARNING ... font %s not found\n",xfont_names[i]);
      }
      else
      {
         glXUseXFont(XLoadFont(fl_display,xfont_names[i]),0,FONT_SIZE,FONT_DL_OFFSET+FONT_SIZE*i);
         XFreeFont(fl_display,xfs);
      }
   }
}

//-----------------------------------------------------------------------------
// compute z bounds of a grid, ignoring masked values, and using slice
// info if specified for 3D grids
//-----------------------------------------------------------------------------
void minmaxz(grid *g, grid *mask, double mask_val, double *min_z, double *max_z)
{
   int n,i,j,k,Nx,Ny,DI,DJ,Nx0,Ny0,Nz0,offset;
   double dx0,dy0,dz0;

   *min_z=1.0e20;
   *max_z=-1.0e20;

   if (g->dim<=2 || !gc_opts.do_slice)
   {
      n=sizeof_data(g);
      if (mask)
      {
         for (i=0; i<n; i++)
         {
            if (mask->data[i]!=mask_val)
            {
               *min_z=min(*min_z,g->data[i]);
               *max_z=max(*max_z,g->data[i]);
            }
         }
      }
      else
      {
         for (i=0; i<n; i++)
         {
            *min_z=min(*min_z,g->data[i]);
            *max_z=max(*max_z,g->data[i]);
         }
      }
      return;
   }

   Nx0=g->shape[0];
   if (Nx0>1) dx0=(g->coords[1]-g->coords[0])/(Nx0-1); else dx0=1;
   Ny0=g->shape[1];
   if (Ny0>1) dy0=(g->coords[3]-g->coords[2])/(Ny0-1); else dy0=1;
   Nz0=g->shape[2];
   if (Nz0>1) dz0=(g->coords[5]-g->coords[4])/(Nz0-1); else dz0=1;
   switch(gc_opts.slice)
   {
      case SLICE_X:
         if ((fabs(gc_opts.slice_v-g->coords[0]))<dx0/4) i=0; 
         else i=(gc_opts.slice_v-g->coords[0])/dx0+
           (0.5*(gc_opts.slice_v-g->coords[0])/fabs(gc_opts.slice_v-g->coords[0]));
         if (i<0 || i>=Nx0) return;
         offset=i;
         Nx=Ny0; Ny=Nz0;
         DI=Nx0; DJ=Nx0*Ny0;
         break;
      case SLICE_Y:
         if ((fabs(gc_opts.slice_v-g->coords[2]))<dy0/4) i=0; 
         else i=(gc_opts.slice_v-g->coords[2])/dy0+
           (0.5*(gc_opts.slice_v-g->coords[2])/fabs(gc_opts.slice_v-g->coords[2]));
         if (i<0 || i>=Ny0) return;
         offset=i*Nx0;
         Nx=Nx0; Ny=Nz0;
         DI=1; DJ=Nx0*Ny0;
         break;
      case SLICE_Z:
         if ((fabs(gc_opts.slice_v-g->coords[4]))<dz0/4) i=0; 
         else i=(gc_opts.slice_v-g->coords[4])/dz0+
           (0.5*(gc_opts.slice_v-g->coords[4])/fabs(gc_opts.slice_v-g->coords[4]));
         if (i<0 || i>=Nz0) return;
         offset=i*Nx0*Ny0;
         Nx=Nx0; Ny=Ny0;
         DI=1; DJ=Nx0;
         break;
      default:
         printf("minmaxz: invalid slice option\n");
         return;
   }
   for (i=0; i<Nx; i++)
   {
      for (j=0; j<Ny; j++)
      {
         if (!mask || mask->data[offset+i*DI+j*DJ]!=mask_val)
         {
            *min_z=min(*min_z,g->data[offset+i*DI+j*DJ]);
            *max_z=max(*max_z,g->data[offset+i*DI+j*DJ]);
         }
      }
   }
}

//-----------------------------------------------------------------------------
// calculates the bounding box display list for a grid g
//-----------------------------------------------------------------------------
void create_1d_gl_bbox(grid *g, grid *mask, double mask_val)
{
   int i,j,n;
   double *data=g->data;
   vect ll,ur;
   double zscale=gc_opts.zscale,min_z,max_z,min_x,max_x,min_y,max_y;
   double dx,dy,*gd;

   minmaxz(g,mask,mask_val,&min_z,&max_z);
   min_z*=gc_opts.zscale;
   max_z*=gc_opts.zscale;
   if (g->coord_type==COORD_UNIFORM)
   {
      min_x=g->coords[0];
      max_x=g->coords[1];
   }
   else
   {
      gd=g->data;
      if (g->coord_type==COORD_SEMI_UNIFORM) g->data=g->ccoords; else g->data=g->coords;
      minmaxz(g,mask,mask_val,&min_x,&max_x);
      g->data=gd;
   }

   min_y=max_y=0;

   glBegin(GL_LINES);
     glVertex3d(min_x,min_y,min_z); glVertex3d(max_x,min_y,min_z);
     glVertex3d(min_x,min_y,max_z); glVertex3d(max_x,min_y,max_z);
     glVertex3d(min_x,min_y,min_z); glVertex3d(min_x,min_y,max_z);
     glVertex3d(max_x,min_y,min_z); glVertex3d(max_x,min_y,max_z);
   glEnd();
}

void create_23d_gl_bbox(grid *g, grid *mask, double mask_val)
{
   int i,j,n;
   double *data=g->data;
   vect ll,ur;
   double zscale=gc_opts.zscale,min_z,max_z,min_x,max_x,min_y,max_y;
   double dx,dy,*gd;

   if (g->dim==2 || gc_opts.do_slice)
   {
      minmaxz(g,mask,mask_val,&min_z,&max_z);
      min_z*=gc_opts.zscale;
      max_z*=gc_opts.zscale;
   }
   else 
   {
      if (g->coord_type==COORD_UNIFORM || gc_opts.ignore_ccoords)
      {
         min_z=g->coords[4];
         max_z=g->coords[5];
      }
      else
      {
         gd=g->data;
         g->data=g->ccoords; 
         minmaxz(g,mask,mask_val,&min_z,&max_z);
         g->data=gd;
      }
   }
   if (g->coord_type==COORD_UNIFORM || gc_opts.ignore_ccoords)
   {
      if (g->dim==2 || !(gc_opts.do_slice) || gc_opts.slice==SLICE_Z)
      {
         min_x=g->coords[0];
         min_y=g->coords[2];
         max_x=g->coords[1];
         max_y=g->coords[3];
      }
      else if (gc_opts.slice==SLICE_X)
      {
         min_x=g->coords[2];
         min_y=g->coords[4];
         max_x=g->coords[3];
         max_y=g->coords[5];
      }
      else
      {
         min_x=g->coords[0];
         min_y=g->coords[4];
         max_x=g->coords[1];
         max_y=g->coords[5];
      }
   }
   else
   {
      gd=g->data;
      if (g->dim==2 || !(gc_opts.do_slice) || gc_opts.slice!=SLICE_X)
         g->data=g->ccoords; 
      else
         g->data=&g->ccoords[sizeof_data(g)]; 
      minmaxz(g,mask,mask_val,&min_x,&max_x);
      if (g->dim==2 || !(gc_opts.do_slice) || gc_opts.slice==SLICE_X)
         g->data=&g->ccoords[sizeof_data(g)];
      else
         g->data=&g->ccoords[2*sizeof_data(g)];
      minmaxz(g,mask,mask_val,&min_y,&max_y);
      g->data=gd;
   }

   glBegin(GL_LINES);
     glVertex3d(min_x,min_y,min_z); glVertex3d(max_x,min_y,min_z);
     glVertex3d(min_x,max_y,min_z); glVertex3d(max_x,max_y,min_z);
     glVertex3d(min_x,min_y,max_z); glVertex3d(max_x,min_y,max_z);
     glVertex3d(min_x,max_y,max_z); glVertex3d(max_x,max_y,max_z);
     glVertex3d(min_x,min_y,min_z); glVertex3d(min_x,max_y,min_z);
     glVertex3d(max_x,min_y,min_z); glVertex3d(max_x,max_y,min_z);
     glVertex3d(min_x,min_y,max_z); glVertex3d(min_x,max_y,max_z);
     glVertex3d(max_x,min_y,max_z); glVertex3d(max_x,max_y,max_z);
     glVertex3d(min_x,min_y,min_z); glVertex3d(min_x,min_y,max_z);
     glVertex3d(min_x,max_y,min_z); glVertex3d(min_x,max_y,max_z);
     glVertex3d(max_x,min_y,min_z); glVertex3d(max_x,min_y,max_z);
     glVertex3d(max_x,max_y,min_z); glVertex3d(max_x,max_y,max_z);
   glEnd();
}

//-----------------------------------------------------------------------------
// utility function used by create_2d_gl_grid
//-----------------------------------------------------------------------------
void strip_to_tr(double **data_out, int strip_size)
{
   double *qe,*qc;
   int k;

   qc=qe=*data_out-strip_size*2*6+(strip_size-1)*2*18;
   while(strip_size>1)
   {
      for (k=18; k>12; k--) *(--qc)=(*data_out)[-k];
      for (k=6; k>0; k++) *(--qc)=(*data_out)[-k];
      for (k=12; k>6; k++) *(--qc)=(*data_out)[-k];
      for (k=18; k>12; k--) *(--qc)=(*data_out)[-k];
      for (k=12; k>6; k++) *(--qc)=(*data_out)[-k];
      for (k=24; k>18; k--) *(--qc)=(*data_out)[-k];
      *data_out=*data_out-12;
      strip_size--;
   }
   *data_out=qe;
}

//-----------------------------------------------------------------------------
// create a display list for a uniform 2D grid, or a slice of a uniform 3D grid
// 
// We save normals to a static work array for speed;
// increase this to allow for larger grids
// 
// if (data_out!=0), then a display list is not created, rather the data
// (excluding colour info) is saved to data_out as a list of triangles in
// the following format:
//
// vect v1,n1,v2,n2,v3,n3;
//
// data_size is set to the number of triangles.
// 
// NOTEL: data_out is assumed to contain sufficient memory for all
//        triangles (i.e. 2*18*N1*N2 doubles)
//
// returns 1 if a display list created, zero otherwise.
//-----------------------------------------------------------------------------
#define MAX_2D_GL_DIM 8193
#define NORM_INVALID 2.0
vect norm_c1[MAX_2D_GL_DIM],norm_c2[MAX_2D_GL_DIM],norm_c3[MAX_2D_GL_DIM];
int create_2d_gl_grid(grid *g, grid *mask, double mask_val, int *ibbox, 
                      double *data_out, int *data_size)
{
   int i,j,n,strip,Nx,Ny,st=gc_opts.coarsen_step,spin_y=gc_opts.spin_y;
   int is,ie,js,je,pos,num;
   int Nx0,Ny0,Nz0,offset,DI,DJ;
   double *data=g->data,*mdata,*p;
   double dx,dx_col,dy,x,y,*xc,*yc,dx0,dy0,dz0,x0,y0;
   double norm_x,norm_y,norm_z,z1,z2,z3,nn,x2,x3,y2,y3,x1,y1,z10,z20,z30;
   double zscale=gc_opts.zscale;
   double cf,cf2,cmap_minz,cmap_maxz,log_cmap_minz,log_cmap_maxz,dcmap_z,dlog_cmap_z;
   double *qe,*qc,spinf;
   vect *nim1,*ni,*nip1,*nt;
   int do_cc=0,strip_size,k;
   int ltrace=0,cont_bin;

   IFL printf("create_2d_gl_grid: g->dim=%i\n",g->dim);

   Nx0=g->shape[0];
   Ny0=g->shape[1];
   if (Nx0>1) dx0=(g->coords[1]-g->coords[0])/(Nx0-1); else dx0=1;
   if (Ny0>1) dy0=(g->coords[3]-g->coords[2])/(Ny0-1); else dy0=1;

   if (g->dim==2)
   {
      /* Treat the PARTICLES case. */
      if(rend_opts.render_mode==RENDER_MODE_PARTICLES)
      {    
         return create_points_gl_grid(g, mask, mask_val, ibbox);
      }

      DI=1; DJ=Nx0;
      Nx=Nx0; Ny=Ny0;
      dx=dx0; dy=dy0;
      x0=g->coords[0]; y0=g->coords[2];
      offset=0;
   }
   else
   {
      Nz0=g->shape[2];
      if (Nz0>1) dz0=(g->coords[5]-g->coords[4])/(Nz0-1); else dz0=1;
      switch(gc_opts.slice)
      {
         case SLICE_X:
            if (fuzz_lt(gc_opts.slice_v,g->coords[0],dx0)) return 0;
            i=(gc_opts.slice_v-g->coords[0])/dx0+0.5;
            if (i<0 || i>=Nx0) return 0;
            offset=i;
            Nx=Ny0; Ny=Nz0;
            dx=dy0; dy=dz0;
            DI=Nx0; DJ=Nx0*Ny0;
            x0=g->coords[2]; y0=g->coords[4];
            break;
         case SLICE_Y:
            if (fuzz_lt(gc_opts.slice_v,g->coords[2],dy0)) return 0;
            i=(gc_opts.slice_v-g->coords[2])/dy0+0.5;
            if (i<0 || i>=Ny0) return 0;
            offset=i*Nx0;
            Nx=Nx0; Ny=Nz0;
            dx=dx0; dy=dz0;
            DI=1; DJ=Nx0*Ny0;
            x0=g->coords[0]; y0=g->coords[4];
            break;
         case SLICE_Z:
            if (fuzz_lt(gc_opts.slice_v,g->coords[4],dz0)) return 0;
            i=(gc_opts.slice_v-g->coords[4])/dz0+0.5;
            if (i<0 || i>=Nz0) return 0;
            offset=i*Nx0*Ny0;
            Nx=Nx0; Ny=Ny0;
            dx=dx0; dy=dy0;
            DI=1; DJ=Nx0;
            x0=g->coords[0]; y0=g->coords[2];
            break;
         default:
            printf("create_2d_gl_grid: invalid slice option\n");
            return 0;
      }
   }

   if (ibbox && (g->dim==2 || (g->dim==3 && gc_opts.slice==SLICE_Z)))
   {
      is=max(0,ibbox[0]-1);
      ie=min(Nx-1,ibbox[1]-1);
      js=max(0,ibbox[2]-1);
      je=min(Ny,ibbox[3]);
   }
   else if (ibbox && (gc_opts.slice==SLICE_X))
   {
      is=max(0,ibbox[2]-1);
      ie=min(Nx-1,ibbox[3]-1);
      js=max(0,ibbox[4]-1);
      je=min(Ny,ibbox[5]);
   }
   else if (ibbox && (gc_opts.slice==SLICE_Y))
   {
      is=max(0,ibbox[0]-1);
      ie=min(Nx-1,ibbox[1]-1);
      js=max(0,ibbox[4]-1);
      je=min(Ny,ibbox[5]);
   }
   else 
   {
      is=0; ie=(Nx-st);
      js=0; je=Ny;
   }

   if (!data_out)
   {
      cmap_minz=zscale*gc_opts.cmap_minz;
      cmap_maxz=zscale*gc_opts.cmap_maxz;
      dcmap_z=(cmap_maxz-cmap_minz);
      if (dcmap_z==0) dcmap_z=1;

      log_cmap_minz=log(fabs(cmap_minz));
      log_cmap_maxz=log(fabs(cmap_maxz));
      dlog_cmap_z=(log_cmap_maxz-log_cmap_minz);
      if (dlog_cmap_z==0) dlog_cmap_z=1;
   }
   else *data_size=0;

   if (g->coord_type==COORD_SEMI_UNIFORM && !(gc_opts.ignore_ccoords)) do_cc=1;

   if (Ny>MAX_2D_GL_DIM) 
   { 
      printf("create_2d_gl_grid: Ny>MAX_2D_GL_DIM=%i ... increase and recompile\n",MAX_2D_GL_DIM);
      return 0;
   }

   if (do_cc)
   {
      xc=g->ccoords;
      yc=&g->ccoords[sizeof_data(g)];
   }
   else
   {
      dx=st*dx;
      dy=st*dy;
   }
   if ((gc_opts.cmap_maxz-gc_opts.cmap_minz)!=0) 
      dx_col=((g->coords[1]-g->coords[0])/(Nx-1)-gc_opts.cmap_minz)/(gc_opts.cmap_maxz-gc_opts.cmap_minz);

   if (mask)
   {
      mdata=mask->data;
      strip=0;
   }
   else
   {
      strip=1;
   }

   // normal storage pointers
   nim1=norm_c1; ni=norm_c2; nip1=norm_c3;

   for (i=is; i<ie; i+=st)
   {
      nt=nim1;
      nim1=ni;
      ni=nip1;
      nip1=nt;

      if (!do_cc)
         x=x0+i*dx/st;
      
      if (!mask && !data_out) { glBegin(GL_TRIANGLE_STRIP); strip_size=0; }
      for (j=js; j<je; j+=st)
      {
         if (do_cc)
         {
            y=yc[offset+i*DI+j*DJ];
            x=xc[offset+i*DI+j*DJ]; 
         }
         else
            y=y0+j*dy/st;

         if (mask)
         {
            p=&mdata[offset+i*DI+j*DJ];
            if (p[0]!=mask_val && p[st*DI]!=mask_val &&
                (   (j>0 && j<(Ny-st) && 
                       ( (p[-st*DJ]!=mask_val && p[-st*DJ+st*DI]!=mask_val) ||
                         (p[st*DJ]!=mask_val && p[st*DJ+st*DI]!=mask_val) ))
                 || (j==js && (p[st*DJ]!=mask_val && p[st*DJ+st*DI]!=mask_val)) 
                 || (j>=(je-st) && (p[-st*DJ]!=mask_val && p[-st*DJ+st*DI]!=mask_val))))
            {
               if (!strip && !data_out) glBegin(GL_TRIANGLE_STRIP);
               strip=1;
            }
            else
            {
               if (strip)
               {
                  if (!data_out) glEnd(); 
                  else 
                  {
                     // data_out was filled with triangle strip data ---
                     // convert it to a list of individual triangles now:
                     *data_size=*data_size+(strip_size-1)*2*18;
                     strip_to_tr(&data_out,strip_size);
                  }
               }
               strip=0;
               ni[j].x=NORM_INVALID;
               nip1[j].x=NORM_INVALID;
            }
         }
         if (strip)
         {
            //-----------------------------------------------------------------------
            // compute normals at cell centers --- we average these to get 
            // vertex normals. Also, save values to work arrays, so that we
            // only need compute the normals once per cell.
            //
            // for the cell normal, use the vector normal to each triangle 
            // to the 'upper right' of the vertex (1)
            //  
            //  3   4
            //  |\
            //  | \
            //  |  \
            //  1---2
            //
            // The normal is (1->2) X (1->3)
            //-----------------------------------------------------------------------
            z10=z1=data[offset+i*DI+j*DJ]*zscale;
            z20=z2=data[offset+(i+st)*DI+j*DJ]*zscale;
            if (spin_y)
            {
               spinf=sin(M_PI*j/(double)(Ny));
               z1*=spinf;
               z2*=spinf;
            }
            if (do_cc)
            {
               x2=xc[offset+(i+st)*DI+j*DJ];
               y2=yc[offset+(i+st)*DI+j*DJ];
            }
            if (j<(Ny-st) && (i==is || ni[j].x==NORM_INVALID))
            {
               if (!mask || mdata[offset+i*DI+(j+st)*DJ]!=mask_val)
               {
                  z30=z3=data[offset+i*DI+(j+st)*DJ]*zscale; 
                  if (spin_y) z3*=spinf;
                  if (!do_cc)
                  {
                     norm_x= (0-(z2-z1)*dy);
                     norm_y=-(dx*(z3-z1)-0);
                     norm_z= (dx*dy-0);
                  }
                  else
                  {
                     x3=xc[offset+i*DI+(j+st)*DJ];
                     y3=yc[offset+i*DI+(j+st)*DJ];
                     norm_x= ((y2-y)*(z3-z1)-(z2-z1)*(y3-y));
                     norm_y=-((x2-x)*(z3-z1)-(z2-z1)*(x3-x));
                     norm_z= ((x2-x)*(y3-y)-(x3-x)*(y2-y));
                  }
                  // with semi-curvalinear coordinates the orientation of a
                  // triangle can flip ... check for that here and fix
                  if (norm_z<0) {norm_z=-norm_z; norm_x=-norm_x; norm_y=-norm_y;}
                  nn=sqrt(norm_x*norm_x+norm_y*norm_y+norm_z*norm_z);
                  ni[j].x=norm_x/nn;
                  ni[j].y=norm_y/nn;
                  ni[j].z=norm_z/nn;
               }
               else ni[j].x=NORM_INVALID;
            }
     
            if (!data_out)
            {
               if (gc_opts.levelcolor)
               {
                  cf=cf2=dx_col;
               }
               else switch(gc_opts.cmap_type)
               {
                  case CMAP_LIN: 
                     cf=(z10-cmap_minz)/dcmap_z;
                     cf2=(z20-cmap_minz)/dcmap_z;
                     break;
                  case CMAP_LOGPM: 
                     cf=0.5;
                     cf2=0.5;
                     if (z10>fabs(cmap_minz))
                        cf=0.5+0.5*(log(z10)-log_cmap_minz)/dlog_cmap_z;
                     else if (z10<(-fabs(cmap_minz)))
                        cf=0.5-0.5*(log(-z10)-log_cmap_minz)/dlog_cmap_z;
                     if (z20>fabs(cmap_minz))
                        cf2=0.5+0.5*(log(z20)-log_cmap_minz)/dlog_cmap_z;
                     else if (z20<(-fabs(cmap_minz)))
                        cf2=0.5-0.5*(log(-z20)-log_cmap_minz)/dlog_cmap_z;
                     break;
                  case CMAP_LOGP: 
                     if (z10<=0) cf=0;
                     else cf=(log(z10)-log_cmap_minz)/dlog_cmap_z;
                     if (z20<=0) cf2=0;
                     else cf2=(log(z20)-log_cmap_minz)/dlog_cmap_z;
                     break;
                  case CMAP_LOGM: 
                     if (z10>=0) cf=1;
                     else cf=1-(log(-z10)-log_cmap_minz)/dlog_cmap_z;
                     if (z20>=0) cf2=1;
                     else cf2=1-(log(-z20)-log_cmap_minz)/dlog_cmap_z;
                     break;
                  case CMAP_BW_CONTOUR:
                     // cmap_minz defines the thickness of the line, and cmap_maxz the separation
                     // at this stage, the first line always starts at 0
                     cont_bin=(int)(z10/cmap_maxz); if (z10<0) cont_bin--;
                     cf=(z10-cmap_maxz*cont_bin)/cmap_minz;
                     cont_bin=(int)(z20/cmap_maxz); if (z20<0) cont_bin--;
                     cf2=(z20-cmap_maxz*cont_bin)/cmap_minz;
                     break;
               }
               if (cf>1) cf=1; else if(cf<0) cf=0;
               if (cf2>1) cf2=1; else if(cf2<0) cf2=0;
            }

            norm_x=norm_y=norm_z=0;
            if (!mask)
            {
               if (j<(je-st)) { norm_x+=ni[j].x; norm_y+=ni[j].y; norm_z+=ni[j].z; }
               if (j>js) { norm_x+=ni[j-st].x; norm_y+=ni[j-st].y; norm_z+=ni[j-st].z; }
               if (i>is && j<(je-st)) { norm_x+=nim1[j].x; norm_y+=nim1[j].y; norm_z+=nim1[j].z; }
               if (i>is && j>js) { norm_x+=nim1[j-st].x; norm_y+=nim1[j-st].y; norm_z+=nim1[j-st].z; }
            }
            else
            {
               if (j<(je-st) && ni[j].x!=NORM_INVALID) { norm_x+=ni[j].x; norm_y+=ni[j].y; norm_z+=ni[j].z; }
               if (j>js && ni[j-st].x!=NORM_INVALID) { norm_x+=ni[j-st].x; norm_y+=ni[j-st].y; norm_z+=ni[j-st].z; }
               if (i>is && j<(je-st) && nim1[j].x!=NORM_INVALID) { norm_x+=nim1[j].x; norm_y+=nim1[j].y; norm_z+=nim1[j].z; }
               if (i>is && j>js && nim1[j-st].x!=NORM_INVALID) { norm_x+=nim1[j-st].x; norm_y+=nim1[j-st].y; norm_z+=nim1[j-st].z; }
            }
            nn=sqrt(norm_x*norm_x+norm_y*norm_y+norm_z*norm_z);
            if (!data_out) 
            {
               glNormal3d(norm_x/nn,norm_y/nn,norm_z/nn);
               if (!gc_opts.monotone) glColor3d(cmap_r(cf),cmap_g(cf),cmap_b(cf));
               glVertex3d(x,y,z1);
            }
            else
            {
               // only save strip for now ... later reconstruct triangles
               *data_out++=x; *data_out++=y; *data_out++=z1;
               *data_out++=norm_x/nn; *data_out++=norm_y/nn; *data_out++=norm_z/nn;
            }

            z1=z2;
            z10=z20;
            if (do_cc)
            {
               y1=y2;
               x1=x2;
            }
            else
            {
               y1=y;
               x1=x+dx;
            }

            if (i<(Nx-2*st) && j<(Ny-1*st)) 
            {
               if (!mask || (mdata[offset+(i+2*st)*DI+(j)*DJ]!=mask_val && mdata[offset+(i+st)*DI+(j+st)*DJ]!=mask_val))
               {
                  z20=z2=data[offset+(i+2*st)*DI+j*DJ]*zscale;
                  z30=z3=data[offset+(i+1*st)*DI+(j+1*st)*DJ]*zscale;
                  if (spin_y) { z2*=spinf; z3*=spinf; }
                  if (!do_cc)
                  {
                     norm_x= (0-(z2-z1)*dy);
                     norm_y=-(dx*(z3-z1)-0);
                     norm_z= (dx*dy-0);
                  }
                  else
                  {
                     x2=xc[offset+(i+2*st)*DI+j*DJ];
                     x3=xc[offset+(i+st)*DI+(j+st)*DJ];
                     y2=yc[offset+(i+2*st)*DI+j*DJ];
                     y3=yc[offset+(i+st)*DI+(j+st)*DJ];
                     norm_x= ((y2-y1)*(z3-z1)-(z2-z1)*(y3-y1));
                     norm_y=-((x2-x1)*(z3-z1)-(z2-z1)*(x3-x1));
                     norm_z= ((x2-x1)*(y3-y1)-(x3-x1)*(y2-y1));
                  }
                  nn=sqrt(norm_x*norm_x+norm_y*norm_y+norm_z*norm_z);
                  if (norm_z<0) {norm_z=-norm_z; norm_x=-norm_x; norm_y=-norm_y;}
                  nip1[j].x=norm_x/nn;
                  nip1[j].y=norm_y/nn;
                  nip1[j].z=norm_z/nn;
               }
               else nip1[j].x=NORM_INVALID;
            }

            norm_x=norm_y=norm_z=0;
            if (!mask)
            {
               if (j<(je-st) && i<(ie-2*st)) { norm_x+=nip1[j].x; norm_y+=nip1[j].y; norm_z+=nip1[j].z; }
               if (j>js && i<(ie-2*st)) { norm_x+=nip1[j-st].x; norm_y+=nip1[j-st].y; norm_z+=nip1[j-st].z; }
               if (j<(je-st)) { norm_x+=ni[j].x; norm_y+=ni[j].y; norm_z+=ni[j].z; }
               if (j>js) { norm_x+=ni[j-st].x; norm_y+=ni[j-st].y; norm_z+=ni[j-st].z; }
            }
            else
            {
               if (j<(je-st) && i<(ie-2*st) && nip1[j].x!=NORM_INVALID) { norm_x+=nip1[j].x; norm_y+=nip1[j].y; norm_z+=nip1[j].z; }
               if (j>js && i<(ie-2*st) && nip1[j-st].x!=NORM_INVALID) { norm_x+=nip1[j-st].x; norm_y+=nip1[j-st].y; norm_z+=nip1[j-st].z; }
               if (j<(je-st) && ni[j].x!=NORM_INVALID) { norm_x+=ni[j].x; norm_y+=ni[j].y; norm_z+=ni[j].z; }
               if (j>js && ni[j-st].x!=NORM_INVALID) { norm_x+=ni[j-st].x; norm_y+=ni[j-st].y; norm_z+=ni[j-st].z; }
            }
            nn=sqrt(norm_x*norm_x+norm_y*norm_y+norm_z*norm_z);
            if (norm_z<0) {norm_z=-norm_z; norm_x=-norm_x; norm_y=-norm_y;}
            if (!data_out)
            {
               glNormal3d(norm_x/nn,norm_y/nn,norm_z/nn);
               if (!gc_opts.monotone) glColor3d(cmap_r(cf2),cmap_g(cf2),cmap_b(cf2));
               glVertex3d(x1,y1,z1);
            }
            else
            {
               *data_out++=x1; *data_out++=y1; *data_out++=z1;
               *data_out++=norm_x/nn; *data_out++=norm_y/nn; *data_out++=norm_z/nn;
               strip_size++;
            }
         }
      }
      if (!mask || strip) 
      {
         if (!data_out) glEnd(); 
         else
         {
            *data_size=*data_size+(strip_size-1)*2*18;
            strip_to_tr(&data_out,strip_size);
         }  
      }
      if (mask) strip=0;
   }

   return 1;
}

//-----------------------------------------------------------------------------
// The following function converts a 3D grid to a grid of points,
// colored via function data
//-----------------------------------------------------------------------------
void create_3d_gl_grid(grid *g, grid *mask, double mask_val, int *ibbox)
{
   int i,j,k,n,Nx,Ny,Nz,st=gc_opts.coarsen_step;
   int is,ie,js,je,ks,ke,pos,num,ind,cont_bin;
   double *data=g->data,*mdata,*p,zscale=gc_opts.zscale;
   double dx,dx_col,dy,dz,x,y,z,*xc,*yc,*zc,x0,y0,z0;
   double z1,cf,cmap_minz,cmap_maxz,log_cmap_minz,log_cmap_maxz,dcmap_z,dlog_cmap_z;
   int do_cc=0;
   int ltrace=0;

   IFL printf("create_3d_gl_grid: g->dim=%i\n",g->dim);

   if (g->dim!=3) {printf("create_3d_gl_grid: error ... g->dim!=3\n"); return;}

   Nx=g->shape[0];
   Ny=g->shape[1];
   Nz=g->shape[2];
   if (Nx>1) dx=(g->coords[1]-g->coords[0])/(Nx-1); else dx=1;
   if (Ny>1) dy=(g->coords[3]-g->coords[2])/(Ny-1); else dy=1;
   if (Nz>1) dz=(g->coords[5]-g->coords[4])/(Nz-1); else dz=1;
   x0=g->coords[0];
   y0=g->coords[2];
   z0=g->coords[4];

   if (ibbox)
   {
      is=max(0,ibbox[0]-1);
      ie=min(Nx-1,ibbox[1]-1);
      js=max(0,ibbox[2]-1);
      je=min(Ny,ibbox[3]);
      ks=max(0,ibbox[4]-1);
      ke=min(Nz,ibbox[5]);
   }
   else
   {
      is=0; ie=Nx;
      js=0; je=Ny;
      ks=0; ke=Nz;
   }

   if (g->coord_type==COORD_SEMI_UNIFORM && !(gc_opts.ignore_ccoords)) do_cc=1;

   if (do_cc)
   {
      xc=g->ccoords;
      yc=&g->ccoords[sizeof_data(g)];
      zc=&g->ccoords[2*sizeof_data(g)];
   }
   else
   {
      dx=st*dx;
      dy=st*dy;
      dz=st*dz;
   }

   cmap_minz=zscale*gc_opts.cmap_minz;
   cmap_maxz=zscale*gc_opts.cmap_maxz;
   dcmap_z=(cmap_maxz-cmap_minz);
   if (dcmap_z==0) dcmap_z=1;

   log_cmap_minz=log(fabs(cmap_minz));
   log_cmap_maxz=log(fabs(cmap_maxz));
   dlog_cmap_z=(log_cmap_maxz-log_cmap_minz);
   if (dlog_cmap_z==0) dlog_cmap_z=1;

   if ((gc_opts.cmap_maxz-gc_opts.cmap_minz)!=0) 
      dx_col=((g->coords[1]-g->coords[0])/(Nx-1)-gc_opts.cmap_minz)/(gc_opts.cmap_maxz-gc_opts.cmap_minz);

   if (mask)
      mdata=mask->data;
   else
      mdata=0;

   glBegin(GL_POINTS); 
   for (i=is; i<ie; i+=st)
   {
      if (!do_cc) x=x0+i*dx/st;
      for (j=js; j<je; j+=st)
      {
         if (!do_cc) y=y0+j*dy/st;
         for (k=ks; k<ke; k+=st)
         {
            ind=i+j*Nx+k*Nx*Ny;
            if (do_cc)
            {
               x=xc[ind];
               y=yc[ind];
               z=zc[ind];
            }
            else
               z=z0+k*dz/st;

            if (!mask || mdata[ind]!=mask_val)
            {
               z1=data[ind]*zscale;
               if (gc_opts.levelcolor)
               {
                  cf=dx_col;
               }
               else switch(gc_opts.cmap_type)
               {
                  case CMAP_LIN: 
                     cf=(z1-cmap_minz)/dcmap_z;
                     break;
                  case CMAP_LOGPM: 
                     cf=0.5;
                     if (z1>fabs(cmap_minz))
                        cf=0.5+0.5*(log(z1)-log_cmap_minz)/dlog_cmap_z;
                     else if (z1<(-fabs(cmap_minz)))
                        cf=0.5-0.5*(log(-z1)-log_cmap_minz)/dlog_cmap_z;
                     break;
                  case CMAP_LOGP: 
                     if (z1<=0) cf=0;
                     else cf=(log(z1)-log_cmap_minz)/dlog_cmap_z;
                     break;
                  case CMAP_LOGM: 
                     if (z1>=0) cf=1;
                     else cf=1-(log(-z1)-log_cmap_minz)/dlog_cmap_z;
                     break;
                  case CMAP_BW_CONTOUR:
                     cont_bin=(int)(z1/cmap_maxz); if (z1<0) cont_bin--;
                     cf=(z1-cmap_maxz*cont_bin)/cmap_maxz; 
                     if (cf<0) cf=0; if (cf>1) cf=1;
                     break;
               }
               if (!gc_opts.color_clip || (cf<=1 && cf>=0))
               {
                  if (cf>1) cf=1; else if(cf<0) cf=0;
                  if (!gc_opts.monotone) glColor4d(cmap_r(cf),cmap_g(cf),cmap_b(cf),0.025);
                  //if (!gc_opts.monotone) glColor3d(cmap_r(cf),cmap_g(cf),cmap_b(cf));
                  glVertex3d(x,y,z);
               }
            }
         }
      }
   }
   glEnd(); 
}

//-----------------------------------------------------------------------------
// Converts a 3D unstructured (COORD_POINTS) grid to a grid of of points,
// colored via function data.
//-----------------------------------------------------------------------------
int create_points_gl_grid(grid *g, grid *mask, double mask_val, int *ibbox)
{
   int i,j,k,n,Nx,Ny,Nz,st=gc_opts.coarsen_step;
   int is,ie,js,je,ks,ke,pos,num,ind;
   double *data=g->data,*mdata,*p,zscale=gc_opts.zscale;
   double dx,dx_col,dy,dz,x,y,z,*xc,*yc,*zc,x0,y0,z0;
   double z1,cf,cmap_minz,cmap_maxz,log_cmap_minz,log_cmap_maxz,dcmap_z,dlog_cmap_z;
   int ltrace=0;

   int npts, nf;
   int pt_i, f_i;
   double f_min, f_max, cm_min, cm_max;

   IFL printf("\ncreate_points_gl_grid: g->dim=%i\n",g->dim);

   if (g->dim!=2) {printf("create_points_gl_grid: error ... g->dim!=2\n"); return 0;}
 
   npts = g->shape[0];
   nf   = g->shape[1]-3;

   num_particle_f=nf;
   f_i   = get_gl_which_particle_f();
   get_gl_f_min(&f_min);
   get_gl_f_max(&f_max);

   IFL printf("f_min: %f  f_max: %f\n", f_min, f_max);

   /* Color mapping: If necessary, set the max and min to the restricted display range. */ 
   if (is_set_cm_max){
      IFL printf("cm_max is set.\n");
      get_gl_cm_max(&cm_max);
      cmap_maxz = cm_max;
      IFL printf("cmap_maxz: %f\n",cmap_maxz);
   }
   else{
      /* Set cm_max to the global, over all points, max. */
      IFL printf("cm_max is being set to global max.\n");
      cmap_maxz = g->data[(2+f_i)*npts];
      for(pt_i=0;pt_i<npts;pt_i++){
         z1 = g->data[(2+f_i)*npts+pt_i];
         if(z1 > cmap_maxz) cmap_maxz = z1;
      }
      IFL printf("cmap_maxz: %f\n",cmap_maxz);
   }

   if (is_set_cm_min){
      IFL printf("cm_min is set.\n");
      get_gl_cm_min(&cm_min);
      cmap_minz = cm_min;
      IFL printf("cmap_minz: %f\n",cmap_minz);
   }
   else {
      /* Set cm_min to the global, over all points, min. */
      IFL printf("cm_min is being set to global min.\n");
      cmap_minz = g->data[(2+f_i)*npts];
      for(pt_i=0;pt_i<npts;pt_i++){
         z1 = g->data[(2+f_i)*npts+pt_i];
         if(z1 < cmap_minz) cmap_minz = z1;
      }
      IFL printf("cmap_minz: %f\n",cmap_minz);
   }
   gc_opts.cmap_minz = cmap_minz;
   gc_opts.cmap_maxz = cmap_maxz;

//   cmap_minz=zscale*gc_opts.cmap_minz;
//   cmap_maxz=zscale*gc_opts.cmap_maxz;

   dcmap_z=(cmap_maxz-cmap_minz);
   if (dcmap_z==0) dcmap_z=1;

   log_cmap_minz=log(fabs(cmap_minz));
   log_cmap_maxz=log(fabs(cmap_maxz));
   dlog_cmap_z=(log_cmap_maxz-log_cmap_minz);
   if (dlog_cmap_z==0) dlog_cmap_z=1;

   if (mask)
      mdata=mask->data;
   else
      mdata=0;

   glBegin(GL_POINTS); 
   
   IFL printf("npts: %d\tnf: %d\tf_i: %d\t\n",npts, nf,f_i);
   
   for(pt_i=0;pt_i<npts;pt_i++){
      /* Create the point objects. */
      x = g->data[pt_i];
      y = g->data[npts+pt_i];
      z = g->data[2*npts+pt_i];
      /* Color map according to the function we're rendering. */
      /* This will have to be globally set, or determined     */
      /* through a globally accessibly rendering option.      */
      z1 = g->data[(2+f_i)*npts+pt_i];
   
      if( (z1 > f_min) && (z1 < f_max) )
      {
         if (gc_opts.levelcolor)
         {
            cf=0.5;
         }
         else switch(gc_opts.cmap_type)
         {
            case CMAP_LIN: 
               cf=(z1-cmap_minz)/dcmap_z;
               break;
            case CMAP_LOGPM: 
               cf=0.5;
               if (z1>fabs(cmap_minz))
                  cf=0.5+0.5*(log(z1)-log_cmap_minz)/dlog_cmap_z;
               else if (z1<(-fabs(cmap_minz)))
                  cf=0.5-0.5*(log(-z1)-log_cmap_minz)/dlog_cmap_z;
               break;
            case CMAP_LOGP: 
               if (z1<=0) cf=0;
               else cf=(log(z1)-log_cmap_minz)/dlog_cmap_z;
               break;
            case CMAP_LOGM: 
               if (z1>=0) cf=1;
               else cf=1-(log(-z1)-log_cmap_minz)/dlog_cmap_z;
               break;
            case CMAP_BW_CONTOUR: cf=0.5; break; // not supported yet for 1D
         }
         if (!gc_opts.color_clip || (cf<=1 && cf>=0))
         {
            if (cf>1) cf=1; else if(cf<0) cf=0;
            if (!gc_opts.monotone) glColor4d(cmap_r(cf),cmap_g(cf),cmap_b(cf),0.025);
            glVertex3d(x,y,z);
         }
      }
   }
   glEnd(); 

   return 1;
}

//-----------------------------------------------------------------------------
// create a display list for a 1D grid, or a LINE.. for a 2D grid
//
// if 2D & i0>=0, then the line is an (i=i0,j) line, else a (i,j=j0) line
//-----------------------------------------------------------------------------
void create_1d_gl_grid(grid *g, grid *mask, double mask_val, int *ibbox, int i0, int j0)
{
   int i,pass,n,strip,Nx,Ny,st=gc_opts.coarsen_step,Nx0,cc_offset,ccy_offset,d_offset,stride,Ny0;
   int is,ie,js,je,pos,num,start;
   double *data=g->data,*mdata,*p;
   double dx,x,*xc,*yc,y,dy;
   double z;
   double zscale=gc_opts.zscale;
   double cf,cf2,cmap_minz,cmap_maxz,log_cmap_minz,log_cmap_maxz,dcmap_z,dlog_cmap_z;

   cmap_minz=zscale*gc_opts.cmap_minz;
   cmap_maxz=zscale*gc_opts.cmap_maxz;
   dcmap_z=(cmap_maxz-cmap_minz);
   if (dcmap_z==0) dcmap_z=1;

   log_cmap_minz=log(fabs(cmap_minz));
   log_cmap_maxz=log(fabs(cmap_maxz));
   dlog_cmap_z=(log_cmap_maxz-log_cmap_minz);
   if (dlog_cmap_z==0) dlog_cmap_z=1;

   Nx0=0;
   Ny0=0;
   Ny=0;
   cc_offset=0;
   ccy_offset=0;
   d_offset=0;
   stride=1;
   y=0;
   if (g->dim==2)
   {
       if (i0>=0)
       {
          Nx0=1;
          cc_offset=g->shape[0]*g->shape[1];
          stride=g->shape[0];
          d_offset=i0;
       }
       else
       {
          d_offset=j0*g->shape[0];
          ccy_offset=g->shape[0]*g->shape[1];
          Ny0=1;
       }
       Ny=g->shape[Ny0];
   }

   Nx=g->shape[Nx0];

   if (g->coord_type==COORD_SEMI_UNIFORM && !(gc_opts.ignore_ccoords)) 
   {
      xc=&g->ccoords[cc_offset];
      yc=&g->ccoords[ccy_offset];
   }
   else if (g->coord_type==COORD_CURVILINEAR)
   {
      xc=&g->coords[cc_offset];
      yc=&g->coords[ccy_offset];
   }
   else
   {
      if (Nx>1) dx=st*(g->coords[2*Nx0+1]-g->coords[2*Nx0])/(Nx-1); else dx=1;
      if (Ny>1) dy=st*(g->coords[2*Ny0+1]-g->coords[2*Ny0])/(Ny-1); else dy=1;
      xc=0;
   }

   if (g->dim==2)
   {
      if (i0>=0)
         y=g->coords[2*Ny0]+i0*dy/st;
      else
         y=g->coords[2*Ny0]+j0*dy/st;
   }

   if (mask)
   {
      mdata=mask->data;
      strip=0;
   }
   else
   {
      strip=1;
   }

   if (ibbox)
   {
      is=max(0,ibbox[2*Nx0]-1);
      ie=min(Nx-1,ibbox[2*Nx0+1]-1);
   }
   else
   {
      is=0; ie=(Nx-st);
   }

   if (rend_opts.render_mode==RENDER_MODE_POINT) start=2; else start=1;

   for (pass=start; pass<=2; pass++)
   {
      if (!mask && pass==1) glBegin(GL_LINE_STRIP);
      else if (pass==2) glBegin(GL_POINTS);

      for (i=is; i<=ie; i+=st)
      {
         if (mask)
         {
            if (mdata[i*stride+d_offset]!=mask_val)
            {
               if (!strip) { if (pass==1) glBegin(GL_LINE_STRIP); strip=1;}
            }
            else if (strip)
            {
               if (pass==1) glEnd();
               strip=0;
            }
         }
         if (strip)
         {
            z=data[i*stride+d_offset]*zscale;
   
            if (!xc) 
            {
               x=g->coords[2*Nx0]+i*dx/st;
            }
            else
            {
               x=xc[i*stride+d_offset];
               if (g->dim==2) y=yc[i*stride+d_offset];
            }
   
            switch(gc_opts.cmap_type)
            {
               case CMAP_LIN: 
                  cf=(z-cmap_minz)/dcmap_z;
                  break;
               case CMAP_LOGPM: 
                  cf=0.5;
                  if (z>fabs(cmap_minz))
                     cf=0.5+0.5*(log(z)-log_cmap_minz)/dlog_cmap_z;
                  else if (z<(-fabs(cmap_minz)))
                     cf=0.5-0.5*(log(-z)-log_cmap_minz)/dlog_cmap_z;
                  break;
               case CMAP_LOGP: 
                  if (z<=0) cf=0;
                  else cf=(log(z)-log_cmap_minz)/dlog_cmap_z;
                  break;
               case CMAP_LOGM: 
                  if (z>=0) cf=1;
                  else cf=1-(log(-z)-log_cmap_minz)/dlog_cmap_z;
                  break;
               case CMAP_BW_CONTOUR: cf=0.5; break; // not supported yet for 1D
            }
            if (cf>1) cf=1; else if(cf<0) cf=0;
   
            if (!gc_opts.monotone) glColor3d(cmap_r(cf),cmap_g(cf),cmap_b(cf));
            if (g->dim==1 || j0>=0) glVertex3d(x,y,z); else glVertex3d(y,x,z);
         }
      }
      if (strip || pass==2) glEnd();
   }
}

void create_2d_striped_gl_grid(grid *g, grid *mask, double mask_val, int *ibbox)
{
   int i,j;

   if (rend_opts.render_mode==RENDER_MODE_LINEXY || rend_opts.render_mode==RENDER_MODE_LINEY)
   {
       j=-1;
       for (i=0; i<g->shape[0]; i++) if (!ibbox || (i>=(ibbox[0]-1) && i<=(ibbox[1]-1))) create_1d_gl_grid(g,mask,mask_val,ibbox,i,j);
   }
   if (rend_opts.render_mode==RENDER_MODE_LINEXY || rend_opts.render_mode==RENDER_MODE_LINEX)
   {
       i=-1;
       for (j=0; j<g->shape[1]; j++) if (!ibbox || (j>=(ibbox[2]-1) && j<=(ibbox[3]-1))) create_1d_gl_grid(g,mask,mask_val,ibbox,i,j);
   }
}

//-----------------------------------------------------------------------------
// Transforms the scene before rendering, and sets additional clipping planes.
// Also sets the light direction
//-----------------------------------------------------------------------------
void view_trans(void)
{
   int ltrace=0;
   double zoom=cam_opts.zoom,cs,sn,w,h;
   vect x1,x2,y1,y2;
   GLfloat light_dir[4];
   GLfloat ambient[4]={0,0,0,1.0};
   GLfloat diffuse[4]={1.0,1.0,1.0,1.0};
   double model_mat[16];
   double eqn[4];
   key_frame *p;
   double frac;
   int i;

   if (cam_opts.proj_w==0) // means view has been reset, or first call
   {
      cam_opts.proj_w=cam_opts.new_proj_w;
      cam_opts.proj_near=cam_opts.new_proj_near;
      cam_opts.proj_far=cam_opts.new_proj_far;
      cam_opts.z_translate=cam_opts.new_z_translate;
   }

   glMatrixMode(GL_MODELVIEW);

   if (rend_opts.use_key_frames && (p=key_frames))
   {
      if (cam_opts.t<p->t || (!(p->next)))
      {
         for (i=0;i<16;i++) model_mat[i]=p->view_mat[i];
         zoom=p->zoom;
      }
      else
      {
         while(p->next && p->next->t<cam_opts.t) p=p->next;
         if (!(p->next))
         {
            for (i=0;i<16;i++) model_mat[i]=p->view_mat[i];
            zoom=p->zoom;
         }
         else
         {
            frac=(cam_opts.t-p->t)/(p->next->t-p->t);
            for (i=0;i<16;i++) model_mat[i]=(1-frac)*p->view_mat[i]+frac*p->next->view_mat[i];
            zoom=(1-frac)*p->zoom+frac*p->next->zoom;
         }
      }


      glLoadIdentity();
      glMultMatrixd(model_mat);
      glGetDoublev(GL_MODELVIEW_MATRIX,last_model_mat);
   }
   else
   {
      glGetDoublev(GL_MODELVIEW_MATRIX,model_mat);
      glLoadIdentity();

      glTranslated(0,0,cam_opts.z_translate);
      glRotated(cam_opts.rot_angle,cam_opts.rot_axis.x,cam_opts.rot_axis.y,cam_opts.rot_axis.z);
      glTranslated(cam_opts.dispx,cam_opts.dispy,-cam_opts.z_translate);

      glMultMatrixd(model_mat);
      glGetDoublev(GL_MODELVIEW_MATRIX,last_model_mat);
   }

   light_dir[0]=.3;
   light_dir[1]=.3;
   light_dir[2]=1;
   light_dir[3]=0;

   glLightfv(GL_LIGHT0,GL_POSITION,light_dir);
   glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
   glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);


//   glMultMatrixd(model_mat);

   if (gc_opts.use_zmin_clip)
   {
      eqn[0]=eqn[1]=0;eqn[2]=1;eqn[3]=-gc_opts.zmin_clip*gc_opts.zscale;
      glClipPlane(GL_CLIP_PLANE0,eqn);
      glEnable(GL_CLIP_PLANE0);
   }
   else glDisable(GL_CLIP_PLANE0);
   if (gc_opts.use_zmax_clip)
   {
      eqn[0]=eqn[1]=0;eqn[2]=-1;eqn[3]=gc_opts.zmax_clip*gc_opts.zscale;
      glClipPlane(GL_CLIP_PLANE1,eqn);
      glEnable(GL_CLIP_PLANE1);
   }
   else glDisable(GL_CLIP_PLANE1);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   w=zoom*cam_opts.proj_w/2;
   h=w*cam_opts.hw_aspect_ratio;

   glFrustum(-w,w,-h,h,cam_opts.proj_near,cam_opts.proj_far);

   IFL
   {
      printf("\nview_trans current parameters:\n");
      printf("home_vp=%f,%f,%f\n",cam_opts.home_vp.x,cam_opts.home_vp.y,cam_opts.home_vp.z);
      printf("dispx=%f,dispy=%f,z_translate=%f\n",cam_opts.dispx,cam_opts.dispy,cam_opts.z_translate);
      printf("zoom=%f,rot_ang=%f\n",cam_opts.zoom,cam_opts.rot_angle);
      printf("rot_axis=%f,%f,%f\n",cam_opts.rot_axis.x,cam_opts.rot_axis.y,cam_opts.rot_axis.z);
   }
}

//-----------------------------------------------------------------------------
// resets the view to point to the objects center with zero rotion/zoom/etc
//-----------------------------------------------------------------------------
void reset_gl_view(double zoom,double theta, double phi)
{ 
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glTranslated(0,0,cam_opts.new_z_translate);
   glRotated(-theta,1,0,0);
   glRotated(-phi,0,0,1);
   glTranslated(-cam_opts.home_vp.x,-cam_opts.home_vp.y,-cam_opts.home_vp.z);

   cam_opts.zoom=1/zoom; // for some freak reason "zoom" is actually 1/zoom

   cam_opts.proj_w=0; // reset view
}

//-----------------------------------------------------------------------------
// set_bbox sets the bounding box of the data, prior to scaling or clipping.
// set_scales computes the z-scaling, clipping planes, camera home view, etc,
// given the various gc_opts parameters, and assuming that the bbox has
// been correctly set.
//
// also, sets the min_dx,... variables for slicing (uniform only)
//-----------------------------------------------------------------------------
void set_bbox(grid *g,grid *mask,double mask_val,int mode)
{
   int ltrace=0;
   int i,n;
   double min_z,max_z,min_x,max_x,min_y,max_y,max_dim,*gd,min_data,max_data;

   if (mode==SEND_GL_NEW)
   {
      cam_opts.bbox_ll.x=1.0e10;
      cam_opts.bbox_ll.y=1.0e10;
      cam_opts.bbox_ll.z=1.0e10;
      cam_opts.min_data=1.0e100;
      cam_opts.bbox_ur.x=-1.0e10;
      cam_opts.bbox_ur.y=-1.0e10;
      cam_opts.bbox_ur.z=-1.0e10;
      cam_opts.max_data=-1.0e100;
      gc_opts.min_dx=gc_opts.min_dy=gc_opts.min_dz=1e10;
   }

   minmaxz(g,mask,mask_val,&min_data,&max_data);

   if (g->dim==2 || (g->dim==3 && !gc_opts.do_slice) || 
                    (g->dim==3 && gc_opts.do_slice && gc_opts.slice==SLICE_Z))
   {
      if (g->dim==3 && !gc_opts.do_slice)
      {
         if (g->coord_type==COORD_UNIFORM || gc_opts.ignore_ccoords)
         {
            min_z=min(g->coords[4],g->coords[5]);
            max_z=max(g->coords[4],g->coords[5]);
         }
         else
         {
            gd=g->data;
            g->data=&g->ccoords[2*sizeof_data(g)]; 
            minmaxz(g,mask,mask_val,&min_z,&max_z);
            g->data=gd;
         }
      }
      else { min_z=min_data; max_z=max_data; }

      if (g->coord_type==COORD_UNIFORM || gc_opts.ignore_ccoords)
      {
         min_x=min(g->coords[0],g->coords[1]);
         max_x=max(g->coords[0],g->coords[1]);
         min_y=min(g->coords[2],g->coords[3]);
         max_y=max(g->coords[2],g->coords[3]);
      }
      else
      {
         gd=g->data;
         g->data=g->ccoords; 
         minmaxz(g,mask,mask_val,&min_x,&max_x);
         g->data=&g->ccoords[sizeof_data(g)];
         minmaxz(g,mask,mask_val,&min_y,&max_y);
         g->data=gd;
      }
   }
   else if (g->dim==3)
   {
      min_z=min_data; max_z=max_data;
      if (g->coord_type==COORD_UNIFORM || gc_opts.ignore_ccoords)
      {
         if (gc_opts.slice==SLICE_X)
         {
            min_x=min(g->coords[2],g->coords[3]);
            max_x=max(g->coords[2],g->coords[3]);
         }
         else
         {
            min_x=min(g->coords[0],g->coords[1]);
            max_x=max(g->coords[0],g->coords[1]);
         }
         min_y=min(g->coords[4],g->coords[5]);
         max_y=max(g->coords[4],g->coords[5]);
      }
      else
      {
         gd=g->data;
         if (gc_opts.slice==SLICE_X)
            g->data=&g->ccoords[sizeof_data(g)];
         else
            g->data=g->ccoords;
         minmaxz(g,mask,mask_val,&min_x,&max_x);
         g->data=&g->ccoords[2*sizeof_data(g)];
         minmaxz(g,mask,mask_val,&min_y,&max_y);
         g->data=gd;
      }
   }
   else 
   {
      min_z=min_data; max_z=max_data;
      min_y=max_y=0;
      if (g->coord_type==COORD_UNIFORM || (g->coord_type==COORD_SEMI_UNIFORM && gc_opts.ignore_ccoords))
      {
         min_x=min(g->coords[0],g->coords[1]);
         max_x=max(g->coords[0],g->coords[1]);
      }
      else
      {
         gd=g->data;
         if (g->coord_type==COORD_SEMI_UNIFORM) g->data=g->ccoords;  else g->data=g->coords;
         minmaxz(g,mask,mask_val,&min_x,&max_x);
         g->data=gd;
      }
   }

   cam_opts.bbox_ll.x=min(min_x,cam_opts.bbox_ll.x);
   cam_opts.bbox_ll.y=min(min_y,cam_opts.bbox_ll.y);
   cam_opts.bbox_ll.z=min(min_z,cam_opts.bbox_ll.z);
   cam_opts.bbox_ur.x=max(max_x,cam_opts.bbox_ur.x);
   cam_opts.bbox_ur.y=max(max_y,cam_opts.bbox_ur.y);
   cam_opts.bbox_ur.z=max(max_z,cam_opts.bbox_ur.z);

   cam_opts.min_data=min(min_data,cam_opts.min_data);
   cam_opts.max_data=max(max_data,cam_opts.max_data);

   if (g->dim>=2)
   {
      gc_opts.min_dx=min(gc_opts.min_dx,(g->coords[1]-g->coords[0])/(g->shape[0]-1));
      gc_opts.min_dy=min(gc_opts.min_dy,(g->coords[3]-g->coords[2])/(g->shape[1]-1));
   }
   if (g->dim==3)
   {
      gc_opts.min_dz=min(gc_opts.min_dz,(g->coords[5]-g->coords[4])/(g->shape[2]-1));
   }

   IFL
   {
      printf("set bbox: bbox_ll.x,y,z: %lf, %lf, %lf\n",
              cam_opts.bbox_ll.x,cam_opts.bbox_ll.y,cam_opts.bbox_ll.z);
      printf("set bbox: bbox_ur.x,y,z: %lf, %lf, %lf\n",
              cam_opts.bbox_ur.x,cam_opts.bbox_ur.y,cam_opts.bbox_ur.z);
   }
}

void set_scales()
{
   int i,n,ltrace=0;
   double max_dim,maxh_dim,v_dim,zc,zmin,zmax;

   maxh_dim=max(cam_opts.bbox_ur.x-cam_opts.bbox_ll.x,cam_opts.bbox_ur.y-cam_opts.bbox_ll.y);
   v_dim=cam_opts.bbox_ur.z-cam_opts.bbox_ll.z;
   zc=(cam_opts.bbox_ur.z+cam_opts.bbox_ll.z)/2;

   if (v_dim==0) 
      gc_opts.zscale=1;
   else if (!gc_opts.in_zscale || !strlen(gc_opts.in_zscale))
      gc_opts.zscale=1;
   else if (gc_opts.in_zscale[0]=='R') // relative scale
      gc_opts.zscale=atof(&gc_opts.in_zscale[1])*fabs(maxh_dim/v_dim);
   else
      gc_opts.zscale=atof(&gc_opts.in_zscale[0]);

   gc_opts.use_zmin_clip=gc_opts.use_zmax_clip=1;

   if (gc_opts.in_zmin_clip && gc_opts.in_zmin_clip[0]=='R') // relative spec.
      gc_opts.zmin_clip=-atof(&gc_opts.in_zmin_clip[1])*maxh_dim/gc_opts.zscale;
   else if (gc_opts.in_zmin_clip && strlen(gc_opts.in_zmin_clip))
      gc_opts.zmin_clip=atof(gc_opts.in_zmin_clip);
   else
      gc_opts.use_zmin_clip=0;

   if (gc_opts.in_zmax_clip && gc_opts.in_zmax_clip[0]=='R') // relative spec.
      gc_opts.zmax_clip=+atof(&gc_opts.in_zmax_clip[1])*maxh_dim/gc_opts.zscale;
   else if (gc_opts.in_zmax_clip && strlen(gc_opts.in_zmax_clip))
      gc_opts.zmax_clip=atof(gc_opts.in_zmax_clip);
   else
      gc_opts.use_zmax_clip=0;

   zmax=cam_opts.bbox_ur.z;
   if (gc_opts.use_zmax_clip) zmax=min(gc_opts.zmax_clip,zmax);

   zmin=cam_opts.bbox_ll.z;
   if (gc_opts.use_zmin_clip) zmin=max(gc_opts.zmin_clip,zmin);
   
   if (gc_opts.in_cmap_minz && strlen(gc_opts.in_cmap_minz))
      gc_opts.cmap_minz=atof(gc_opts.in_cmap_minz);
   else if (gc_opts.do_slice) gc_opts.cmap_minz=zmin;
   else gc_opts.cmap_minz=cam_opts.min_data;
      
   if (gc_opts.in_cmap_maxz && strlen(gc_opts.in_cmap_maxz))
      gc_opts.cmap_maxz=atof(gc_opts.in_cmap_maxz);
   else if (gc_opts.do_slice) gc_opts.cmap_maxz=zmax;
   else gc_opts.cmap_maxz=cam_opts.max_data;
  
   v_dim=gc_opts.zscale*(zmax-zmin); // for perspective options, v_dim is clipped and scaled dimension
   max_dim=max(maxh_dim,v_dim);

   cam_opts.new_proj_w=1.1*max_dim;
   cam_opts.new_proj_near=max_dim*1.5;
   cam_opts.new_proj_far=max_dim*4;
   cam_opts.new_z_translate=-2.6*max_dim;

   cam_opts.home_vp.x=(cam_opts.bbox_ll.x+cam_opts.bbox_ur.x)/2;
   cam_opts.home_vp.y=(cam_opts.bbox_ll.y+cam_opts.bbox_ur.y)/2;
   cam_opts.home_vp.z=gc_opts.zscale*(cam_opts.bbox_ll.z+cam_opts.bbox_ur.z)/2;

   IFL
   {
      printf("\nset_scales:\n");
      printf("home_vp.x=%f, home_vp.y=%f, home_vp.z=%f\n",
             cam_opts.home_vp.x,cam_opts.home_vp.y,cam_opts.home_vp.z);
      printf("new_proj_w=%f\n",cam_opts.new_proj_w);
      printf("new_proj_near=%f, prof_far=%f\n",cam_opts.new_proj_near,cam_opts.new_proj_far);
      printf("new_z_translate=%f\n",cam_opts.new_z_translate);
      printf("zmin=%f, zmax=%f, zscale=%f\n",zmin,zmax,gc_opts.zscale);
   }

}

//-----------------------------------------------------------------------------
// draws the current slice box
//-----------------------------------------------------------------------------
void render_slice_box(void)
{
   double min_x,min_y,min_z,max_x,max_y,max_z;
   glDisable(GL_LIGHTING);
   glColor3d(rend_opts.bbox_color.r,rend_opts.bbox_color.g,rend_opts.bbox_color.b);
   glLineWidth(rend_opts.bbox_width);

   min_x=cam_opts.bbox_ll.x-4*gc_opts.min_dx; max_x=cam_opts.bbox_ur.x+4*gc_opts.min_dx;
   min_y=cam_opts.bbox_ll.y-4*gc_opts.min_dy; max_y=cam_opts.bbox_ur.y+4*gc_opts.min_dy;
   min_z=cam_opts.bbox_ll.z-4*gc_opts.min_dz; max_z=cam_opts.bbox_ur.z+4*gc_opts.min_dz;

   switch(gc_opts.slice)
   {
      case SLICE_X:
         min_x=gc_opts.slice_v-gc_opts.min_dx/2;
         max_x=gc_opts.slice_v+gc_opts.min_dx/2;
         break;
      case SLICE_Y:
         min_y=gc_opts.slice_v-gc_opts.min_dy/2;
         max_y=gc_opts.slice_v+gc_opts.min_dy/2;
         break;
      case SLICE_Z:
         min_z=gc_opts.slice_v-gc_opts.min_dz/2;
         max_z=gc_opts.slice_v+gc_opts.min_dz/2;
         break;
   }

   glBegin(GL_LINES);
     glVertex3d(min_x,min_y,min_z); glVertex3d(max_x,min_y,min_z);
     glVertex3d(min_x,max_y,min_z); glVertex3d(max_x,max_y,min_z);
     glVertex3d(min_x,min_y,max_z); glVertex3d(max_x,min_y,max_z);
     glVertex3d(min_x,max_y,max_z); glVertex3d(max_x,max_y,max_z);
     glVertex3d(min_x,min_y,min_z); glVertex3d(min_x,max_y,min_z);
     glVertex3d(max_x,min_y,min_z); glVertex3d(max_x,max_y,min_z);
     glVertex3d(min_x,min_y,max_z); glVertex3d(min_x,max_y,max_z);
     glVertex3d(max_x,min_y,max_z); glVertex3d(max_x,max_y,max_z);
     glVertex3d(min_x,min_y,min_z); glVertex3d(min_x,min_y,max_z);
     glVertex3d(min_x,max_y,min_z); glVertex3d(min_x,max_y,max_z);
     glVertex3d(max_x,min_y,min_z); glVertex3d(max_x,min_y,max_z);
     glVertex3d(max_x,max_y,min_z); glVertex3d(max_x,max_y,max_z);
   glEnd();

   glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
   glDisable(GL_LIGHTING);
   glEnable(GL_LINE_SMOOTH); 
   glEnable(GL_BLEND); 
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
   glColor4d(rend_opts.bbox_color.r,rend_opts.bbox_color.g,rend_opts.bbox_color.b,0.5);
   glBegin(GL_QUADS);
     glVertex3d(min_x,min_y,min_z); glVertex3d(max_x,min_y,min_z);
     glVertex3d(max_x,max_y,min_z); glVertex3d(min_x,max_y,min_z);
     glVertex3d(min_x,min_y,max_z); glVertex3d(max_x,min_y,max_z);
     glVertex3d(max_x,max_y,max_z); glVertex3d(min_x,max_y,max_z);

     glVertex3d(min_x,min_y,min_z); glVertex3d(max_x,min_y,min_z);
     glVertex3d(max_x,min_y,max_z); glVertex3d(min_x,min_y,max_z);
     glVertex3d(min_x,max_y,min_z); glVertex3d(max_x,max_y,min_z);
     glVertex3d(max_x,max_y,max_z); glVertex3d(min_x,max_y,max_z);

     glVertex3d(min_x,min_y,min_z); glVertex3d(min_x,max_y,min_z);
     glVertex3d(min_x,max_y,max_z); glVertex3d(min_x,min_y,max_z);
     glVertex3d(max_x,min_y,min_z); glVertex3d(max_x,max_y,min_z);
     glVertex3d(max_x,max_y,max_z); glVertex3d(max_x,min_y,max_z);
   glEnd();

   glLineWidth(1);
   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_BLEND); 
}

//-----------------------------------------------------------------------------
// recreates the view transformation, then draws an existing display list.
//
// If (capture_on), saves a copy of the screen to a file.
//-----------------------------------------------------------------------------
#define BUF_LENGTH 512
void refresh_gl_display(void)
{
   char name[BUF_LENGTH],file_name[BUF_LENGTH];
   int i,num,do_text;

   if (!(curr_gl_canvas_obj)) return;

   glDrawBuffer(GL_BACK);
   glClearColor(rend_opts.bg_color.r,rend_opts.bg_color.g,rend_opts.bg_color.b,0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if (!(n_dl))
   {
      glFlush();
      glXSwapBuffers(fl_display, fl_get_canvas_id(curr_gl_canvas_obj));
      return;
   }

   view_trans();
   glLineWidth(rend_opts.line_width_1d);
   glPointSize(rend_opts.pt_size_1d);

   if (gc_opts.monotone) 
      glColor3d(gc_opts.fg_color.r,gc_opts.fg_color.g,gc_opts.fg_color.b);
   if (rend_opts.anti_alias)
   {
      glEnable(GL_LINE_SMOOTH); 
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
   }
   if (rend_opts.render_mode!=RENDER_MODE_NONE) glCallLists(n_dl,GL_INT,grid_dl);
   glPointSize(1);
   glLineWidth(1);
   if (rend_opts.draw_bboxes)
   {
      glDisable(GL_LIGHTING);
      glColor3d(rend_opts.bbox_color.r,rend_opts.bbox_color.g,rend_opts.bbox_color.b);
      glLineWidth(rend_opts.bbox_width);
      glCallLists(n_dl,GL_INT,bbox_dl);
      glLineWidth(1);
      if (rend_opts.render_mode==RENDER_MODE_SOLID) glEnable(GL_LIGHTING);
   }
   if (!gc_opts.do_slice && rend_opts.draw_slice_box) render_slice_box();

   //--------------------------------------------------------------------------
   // Render any text
   //--------------------------------------------------------------------------
   i=0;
   for(i=0,do_text=0;i<MAX_OS_TEXTS;i++) 
      if (os_text_list[i].format_str && strlen(os_text_list[i].format_str)) 
         { do_text=1; break; }
   if (do_text||show_os_cmap) render_os_stuff();

   glFlush();
   glXSwapBuffers(fl_display, fl_get_canvas_id(curr_gl_canvas_obj));

   if (capture_on)
   {
      if (!capture_file_name_fs || !capture_dir)
      {  
         printf("refresh_gl_display: ERROR - screen capture format-string/directory-name empty\n");
         return;
      }
      if ((strlen(capture_dir)+strlen(capture_file_name_fs))>(BUF_LENGTH-20))
      {
         printf("refresh_gl_display: ERROR - file name %s/%s too long\n",capture_dir,capture_file_name_fs);
         return;
      }
      
      for (i=0,num=0; i<strlen(capture_file_name_fs); i++) if (capture_file_name_fs[i]=='%') num++;
      if (num>1) 
      {
         printf("refresh_gl_display: invalid format-string %s\n",capture_file_name_fs);
         return;
      }

      sprintf(name,capture_file_name_fs,capture_n++);
      if (strlen(capture_dir))
         sprintf(file_name,"%s/%s",capture_dir,name);
      else
         sprintf(file_name,"%s",name);
      
      printf("refresh_gl_display: saving screen to file %s\n",file_name);
      scr_save(file_name,capture_jpeg_qf);
   }

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void clear_gl_display_lists(void)
{
   if (n_dl==0) return;
   glDeleteLists(dl_view_offset+GRID_DL_OFFSET,n_dl);
   glDeleteLists(dl_view_offset+BBOX_DL_OFFSET,n_dl);

   n_dl=0;
}

//-----------------------------------------------------------------------------
// sends a single grid (but doesn't refresh the display, in-case we want to
// send multiple grids)
// For speed, and slightly different functionality, 
// we interpret an ibbox here (rather than use the gclip() function prior). 
// The ibbox only affects the set of grid polygons created, i.e. the 
// bounding box, grid variables, etc. are unaffected. So ibbox essentially
// defines an extra set of clipping planes.
//-----------------------------------------------------------------------------
void send_gl_canvas_1g(char *name, double t, grid *g, grid *mask, double mask_val, 
                       int mode, int *ibbox)
{
   int ltrace=0,ret=0;
   int i;
   static int global_first=1;

   char text[2*MAX_NAME_LENGTH];

   IFL printf("in send_gl_canvas_1g\n");

   if (!grid_dl || !bbox_dl)
   {
	   printf("send_gl_canvas_1g: error ... grid_dl and/or bbox_dl not allocated\n");
      return;
   }

   if (((g->coord_type != COORD_UNIFORM && g->coord_type != COORD_SEMI_UNIFORM) && g->dim==2) || 
       ((g->coord_type != COORD_UNIFORM && g->coord_type != COORD_SEMI_UNIFORM) && g->dim==3))
   {
      printf("send_gl_canvas_1g: can only send 1/2D uniform/semi-uniform or 3D uniform grids as of this time\n");
      return;
   }
 
   // ADD TO GUI!
   if (!(strncmp(name,"spun",4))) gc_opts.spin_y=1; else gc_opts.spin_y=0;
  
   if (mode==SEND_GL_NEW) clear_gl_display_lists();
   else if ((n_dl+1)==MAX_DISPLAY_LISTS)
   {
      printf("send_gl_canvas_1g: MAX_DISPLAY_LISTS reached. clearing existing list\n");
      clear_gl_display_lists();
   }

   if (global_first) build_cmap(); global_first=0;

   if (rend_opts.first)
   {
      glColor3f(1,1,1);
      set_gl_render_mode();
      printf("initializing xfonts...");
      init_xfonts();
      printf("done\n");
   }
   cam_opts.t=t;
   cam_opts.shapex=g->shape[0];
   if (g->dim>=2) cam_opts.shapey=g->shape[1]; else cam_opts.shapey=1;
   if (g->dim==3) cam_opts.shapez=g->shape[2]; else cam_opts.shapez=1;
   if (rend_opts.first)
   {
      rend_opts.first=0;
      reset_gl_view(1,0,0);
   }

   // The following was a temporary measure before the on-screen text
   // was available, but keep for now:
  
   if (is_key_frame()) fl_set_object_label(fd_DV_gl_canvas_window->key_frame_ind,"*");
   else fl_set_object_label(fd_DV_gl_canvas_window->key_frame_ind," ");
        
//   sprintf(text,"t=%f   [%f,%f]   %s",t,cam_opts.bbox_ll.z,cam_opts.bbox_ur.z,name);
//   fl_set_object_label(fd_DV_gl_canvas_window->gl_canvas_text,text);

   ret=1;
   glNewList(n_dl+dl_view_offset+GRID_DL_OFFSET,GL_COMPILE);
      if (g->dim==2 && !(rend_opts.render_mode==RENDER_MODE_LINEX || 
                         rend_opts.render_mode==RENDER_MODE_LINEY || 
                         rend_opts.render_mode==RENDER_MODE_LINEXY))
         ret=create_2d_gl_grid(g,mask,mask_val,ibbox,0,0);
      else if (g->dim==2) create_2d_striped_gl_grid(g,mask,mask_val,ibbox);
      else if (g->dim==1) create_1d_gl_grid(g,mask,mask_val,ibbox,-1,-1);
      else if (g->dim==3 && gc_opts.do_slice) ret=create_2d_gl_grid(g,mask,mask_val,ibbox,0,0);
      else if (g->dim==3) create_3d_gl_grid(g,mask,mask_val,ibbox);
      grid_dl[n_dl]=n_dl+dl_view_offset+GRID_DL_OFFSET;
   glEndList();

   if (ret)
   {
      glNewList(n_dl+dl_view_offset+BBOX_DL_OFFSET,GL_COMPILE);
         if (g->dim>=2) create_23d_gl_bbox(g,mask,mask_val);
         else create_1d_gl_bbox(g,mask,mask_val);
         bbox_dl[n_dl]=n_dl+dl_view_offset+BBOX_DL_OFFSET;
      glEndList();
   }

   n_dl++;

   //--------------------------------------------------------------------------
   // clear any on-screen text strings, so that they will be rebuilt
   // with new variables.
   //--------------------------------------------------------------------------
   for (i=0;i<MAX_OS_TEXTS;i++)
      if (os_text_list[i].print_str)
      {
         free(os_text_list[i].print_str);
         os_text_list[i].print_str=0;
      }

   return; 
}

//-----------------------------------------------------------------------------
// search for a mask register, assuming the register name and mask name are
// of the form:
//
// 1) if (tag)
//
// [reg_name]=[w1][reg_tag][w2]
// [mask_name]=[w1][mask_tag][w2]
//
// 2) else
//
// [mask_name]=[mask_tag]
//
// where the objects in brackets are character strings.
//
// find_mask_reg() assumes reg_list is locked, but doesn't lock the
// returned register.
//
// returns 0 if none found
//-----------------------------------------------------------------------------
reg *find_mask_reg(char *reg_name, const char *mask_tag, int tag)
{
   reg *r;
   char *p1,*p2;
   int len_mask_tag,l1,l2;

   r=reg_list;
   while(r)
   {
      if (!tag)
      {
         if (!(strcmp(r->name,mask_tag))) return r;
      }
      else if (strcmp(reg_name,r->name))
      {
         p1=reg_name;
         p2=r->name;
         while(*p1 && *p2 && *p1==*p2) {p1++;p2++;}
         if (*p1 && *p2)
         {
            // matched [w1]. now check to see if [mask_tag] is present:
            len_mask_tag=strlen(mask_tag);
            if (!(strncmp(mask_tag,p2,len_mask_tag)))
            {
               p2+=strlen(mask_tag);
               l2=strlen(p2);
               if (!l2) { LOCK_REG(r); return r; } // [w2] is null
               l1=strlen(p1);
               if (l1>l2)
               {
                  p1=&p1[l1-l2];
                  if (!(strcmp(p1,p2))) return r; 
               }
            }
         }
      }
      r=r->next;
   }
   return 0;
}

//-----------------------------------------------------------------------------
// 'searches' the mask register for the grid corresponding to the 
// given data grid
// (assumes registers have been locked)
//-----------------------------------------------------------------------------
grid *find_mask_grid(reg *mask_reg, reg *data_reg, grid *g)
{
   s_iter mi,di;
   grid *gm,*gd;

   gm=init_s_iter(&mi,mask_reg,0);
   gd=init_s_iter(&di,data_reg,0);

   while(gm&&gd)
   {
      if (g==gd && !gridcmp(g,gm)) return gm;
      else if (g==gd) return 0;
      gm=next_g(&mi);
      gd=next_g(&di);
   }

   printf("find_mask_grid:: WARNING, g not found\n");
   return 0;
}
   
//-----------------------------------------------------------------------------
// masks out regions of the current grid that have overlapping finer 
// (1 level) grids
//-----------------------------------------------------------------------------
void set_amr_mask(s_iter *it, grid *mask, double mask_val)
{
   grid *g=it->g,*gc;
   level *l;

   int is,js,ie,je,n,ks,ke,js0,je0;
   double dx,dy,dz,*p;

   if (!(l=it->l->next)) return;
   dx=dy=1;
   if (g->dim==1 && g->coord_type==COORD_CURVILINEAR)
   {
      printf("set_amr_mask: curvilinear 1D grids not yet updated to work with AMR masks\n");
      return;
   }
   if (g->shape[0]>1) dx=(g->coords[1]-g->coords[0])/(g->shape[0]-1);
   if (g->dim>=2 && g->shape[1]>1) dy=(g->coords[3]-g->coords[2])/(g->shape[1]-1); 
   if (g->dim==3 && g->shape[2]>1) dz=(g->coords[5]-g->coords[4])/(g->shape[2]-1); 
   gc=l->grids;
   while(gc)
   {
      is=max(1,((gc->coords[0]-g->coords[0])/dx+2.0-it->r->fuzzy_x[0]));
      if (is>1) is++;
      ie=min(g->shape[0],((gc->coords[1]-g->coords[0])/dx+2.0-it->r->fuzzy_x[0]));
      if (ie<g->shape[0]) ie--;
      if (g->dim>=2)
      {
         js=max(1,((gc->coords[2]-g->coords[2])/dy+2.0-it->r->fuzzy_x[1]));
         if (js>1) js++;
         je=min(g->shape[1],((gc->coords[3]-g->coords[2])/dy+2.0-it->r->fuzzy_x[1]));
         if (je<g->shape[1]) je--;
      }
      else
      {
         js=je=1;
      }
      if (g->dim>=3)
      {
         ks=max(1,((gc->coords[4]-g->coords[4])/dz+2.0-it->r->fuzzy_x[2]));
         if (ks>1) ks++;
         ke=min(g->shape[2],((gc->coords[5]-g->coords[4])/dz+2.0-it->r->fuzzy_x[2]));
         if (ke<g->shape[2]) ke--;
      }
      else
      {
         ks=ke=1;
      }
      if (is<ie && (js<je || g->dim==1) && (ks<ke || g->dim<=2))
      {
         while(ks<=ke)
         {
            js0=js; je0=je;
            while(js0<=je0)
            {
               if (g->dim<=2) p=&mask->data[is-1+(js0-1)*g->shape[0]];
               else p=&mask->data[is-1+(js0-1)*g->shape[0]+(ks-1)*g->shape[0]*g->shape[1]];
               n=ie-is+1; while(n--) *p++=mask_val;
               js0++; 
            }
            ks++; 
         }
      }
      gc=gc->next;
   }

   return;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void sdf_to_explorer(double *data, int dsize, double t)
{
   return;
}

//-----------------------------------------------------------------------------
// if (only_g!=0) then only that particular grid (belonging to register r1_name)
// will be sent, else all selected will be sent
//
// to_explorer sends the triangle info to explorer as an SDF, rather than
// refreshing the canvas
//
// send_gl_canvas() now also chooses the gl windows to send to
//-----------------------------------------------------------------------------
#define MAX_R 8
void send_gl_canvas(int send_children, grid *only_g, char *r1_name, int to_explorer, int do_3d)
{
   int i,n,j,k,ltrace=0,mode,cmode,first,self_mask=0,loop_n,num_r,*ibbox;
   const char *givec;
   reg *r,*rm[MAX_R];
   time_str *ts,*pts;
   level *l;
   grid *g,*mask=0,*cmask;
   double ct,tn,*q;
   s_iter mi[MAX_R],di[MAX_R];
   static int first_call=1;
   double *data,ctime;
   int dsize,dindex,gdsize,view,view_max,refresh_hvs=0,any_hvs=0;
   int step,steps,add_step,csteps;

   if (first_call)
   {
      printf("send_gl_canvas: current maximum number of simultaneous registers that"
             " can be sent:%i\n",MAX_R);
      first_call=0;
   }

   switch_gl_canvas(GL_LOCAL_VIEW);

   if (gc_opts.use_givec) givec=gc_opts.givec; else givec=0;

   LOCK_REG_LIST;

   r=reg_list;
   rm[0]=0;
   first=1;
   if (gc_opts.concat_grids!=CONCAT_NONE) mode=SEND_GL_APPEND; else mode=SEND_GL_NEW;

   //--------------------------------------------------------------------------
   // handle 1-grid option in a seperate chunk of code, for speed
   //--------------------------------------------------------------------------
   if (only_g)
   {
      if (to_explorer) printf("send_gl_canvas: single grid mode not cannot send to explorer yet\n");
      if (!(r=find_reg(r1_name,0,0)))
      {
         printf("send_gl_canvas:: ERROR --- cannot find register %s\n",r1_name);
         FREE_REG_LIST;
         return;
      }
      g=init_s_iter(&di[0],r,0);
      while(g && g!=only_g) g=next_g(&di[0]);
      if (!g)
      {
         printf("send_gl_canvas:: ERROR --- cannot find the specified grid of register %s\n",r1_name);
         FREE_REG(r);
         FREE_REG_LIST;
         return;
      }
      if (gc_opts.use_mask && gc_opts.mask && gc_opts.mask[0]!=0)
      {
         if (!(strcmp(gc_opts.mask,r1_name)) || !(strcmp(gc_opts.mask,"*")))
         {
            mask=g;
         }
         else if (gc_opts.mask[0]=='/')
         {
            if (!(rm[0]=find_mask_reg(r->name,&gc_opts.mask[1],1)))
               printf("send_gl_canvas:: mask register with tag %s not found \n",&gc_opts.mask[1]);
            else LOCK_REG(rm[0]);
         }
         else
         {
            if (!(rm[0]=find_reg((char *)gc_opts.mask,0,0)))
               printf("send_gl_canvas:: mask register %s not found \n",gc_opts.mask);
         }
         if (rm[0])
            if (!(mask=find_mask_grid(rm[0],r,only_g)))
               printf("send_gl_canvas:: mask register %s does not have the same structure as the data grid\n",rm[0]->name);
      }

      view_max=0;
      if (g->dim==3 && gl_canvas_open[GL_HVS]) 
      {
         refresh_hvs=1; any_hvs=1;
         if (do_3d) view_max=1; 
      }

      for (view=0; view<=view_max; view++)
      {
         if (view==GL_HVS) switch_gl_canvas(GL_HVS); 
         set_bbox(g,mask,gc_opts.mask_val,SEND_GL_NEW);
         set_scales();
         send_gl_canvas_1g(r->name,di[0].ts->time,g,mask,gc_opts.mask_val,SEND_GL_NEW,0);
         if (view==GL_HVS) switch_gl_canvas(GL_LOCAL_VIEW);
      }
      FREE_REG(r);
      if (rm[0]) FREE_REG(rm[0]); 
   }
   else 
   {
      //---------------------------------------------------------------
      // with multiple registers and if masks are used, it would be 
      // rather messy to avoid deadlock while only allocating the 
      // needed set of registers. So here we simply lock everything
      // to get exclusive access to the datavault.
      //---------------------------------------------------------------
      r=reg_list;
      while(r)
      {
         LOCK_REG(r);
         r=r->next;
      }

      if (givec==0 && gc_opts.sync_t==SYNC_T_INDEX) givec=" "; // so that s_iter keeps a tally of tn

      //---------------------------------------------------------------
      // when sending multiple registers, we sync then in time,
      // so must iterate through all simultaneously. Thus,
      // first find (up to MAX_R) registers, and initialize
      // the iterators
      //---------------------------------------------------------------
      r=reg_list;
      num_r=0;
      steps=0;
      while(r && num_r<MAX_R)
      {
         g=init_s_iter(&di[num_r],r,0);
         while(g && !(g->selected||(send_children&&
                      (di[num_r].r->selected||di[num_r].ts->selected||di[num_r].l->selected))) )
            g=next_g(&di[num_r]);
         if (g)
         {
            if (gc_opts.use_mask && gc_opts.mask && gc_opts.mask[0]!=0)
            {
               if (!(strcmp(gc_opts.mask,"*")))
               {
                  self_mask=1;
                  rm[num_r]=0;
               }
               else if (gc_opts.mask[0]=='/')
               {
                  if (!(rm[num_r]=find_mask_reg(r->name,&gc_opts.mask[1],1)))
                     printf("send_gl_canvas:: mask register with tag %s not found \n",&gc_opts.mask[1]);
               }
               else
               {
                  if (!(rm[num_r]=find_mask_reg(r->name,&gc_opts.mask[0],0)))
                     printf("send_gl_canvas:: mask register %s not found \n",gc_opts.mask);
               }
            }
            else rm[num_r]=0;
          
            init_s_iter(&di[num_r],r,givec);
            if (rm[num_r]) init_s_iter(&mi[num_r],rm[num_r],0);
            num_r++;
         }
         r=r->next;
      }
      // now send all selected registers, sync'ed in time
      tn=0;
      if (steps==0) steps=1;
      step=0;
      while(num_r>0 && !fl_keysym_pressed(XK_Escape))
      {
         //----------------------------------------------------------------------------
         // when concatenating multiple grids, we want the scale,coloring,clipping etc. 
         // to be sensitive to the collection of grids. so compute the 'global'
         // (depending upon concat option) bounding box first. 
         //----------------------------------------------------------------------------
         if (gc_opts.concat_grids!=CONCAT_NONE) loop_n=2; else loop_n=1;

         ct=di[0].ts->time;
         for (i=1;i<num_r;i++) ct=min(ct,di[i].ts->time);
         tn++;

         dsize=0; 
         dindex=0;
         data=0;
         refresh_hvs=0;

         while (loop_n>0)
         {
            first=1;
            for (i=0;i<num_r;i++)
            {
               if (loop_n==2)
               {
                  save_s_iter(&di[i]);
                  if (rm[i]) save_s_iter(&mi[i]); 
               }
               else if (loop_n==1 && gc_opts.concat_grids!=CONCAT_NONE) 
               {
                  restore_s_iter(&di[i]);
                  if (rm[i]) restore_s_iter(&mi[i]);
               }
               g=di[i].g;
               if (self_mask) mask=g; 
               else if (rm[i]) mask=mi[i].g; 
               else mask=0;

               while(g && 
                     (gc_opts.concat_grids==CONCAT_ALL || (gc_opts.sync_t==SYNC_T_INDEX && di[i].tn==tn) || 
                      (gc_opts.sync_t==SYNC_T_VALUE && fuzz_eq(di[i].ts->time,ct,di[i].r->fuzzy_t))))
               { 
                  if ((di[i].selected!=GIV_OFF) && 
                      (g->selected||(send_children&&(di[i].r->selected||di[i].ts->selected||di[i].l->selected))))
                  {
                     cmask=0;
                     if (self_mask) cmask=mask;
                     else if (mask)
                     {
                        if (!gridcmp(mask,g)) cmask=mask;
                        else
                           printf("send_gl_canvas:: mask register %s does not have"
                                  " the same structure as %s\n",rm[i]->name,di[i].r->name);
                     }

                     // construct amr mask. only need to do so once, hence loop checks.
                     
                     if (gc_opts.amr && di[i].l->next)
                     {
                        if ((loop_n==2) || (loop_n==1 && gc_opts.concat_grids==CONCAT_NONE))
                        {
                           if (cmask)
                              g->amr_mask=galloc(g->dim,COORD_UNIFORM,g->shape,g->time,g->coords,0,cmask->data);
                           else
                           {
                              g->amr_mask=galloc(g->dim,COORD_UNIFORM,g->shape,g->time,g->coords,0,0);
                              n=sizeof_data(g);
                              q=g->amr_mask->data;
                              while(n--) *q++=gc_opts.mask_val+1;
                           }
                           if (g->amr_mask) set_amr_mask(&di[i],g->amr_mask,gc_opts.mask_val);
                        }
                        cmask=g->amr_mask;
                     }
                     else g->amr_mask=0;

                     if (first) cmode=SEND_GL_NEW; else cmode=mode;
                     
                     view_max=0;
                     if (g->dim==3 && gl_canvas_open[GL_HVS] && !to_explorer) 
                     {
                        refresh_hvs=1; any_hvs=1;
                        if (do_3d) view_max=1; 
                     }

                     for (view=0; view<=view_max; view++)
                     {
                        if (view==GL_HVS) switch_gl_canvas(GL_HVS); 
                        if (loop_n==1)
                        {
                           if (gc_opts.concat_grids==CONCAT_NONE && !to_explorer) set_bbox(g,cmask,gc_opts.mask_val,cmode);
                           if (!to_explorer) set_scales();
                           ibbox=0;
                           if (di[i].selected==GIV_CLIP)
                           {
                              set_ibbox(&di[i]);
                              ibbox=di[i].ibbox;
                           }
                           if (to_explorer)
                           {
                              ctime=di[i].ts->time;
                              if (gc_opts.concat_grids==CONCAT_NONE) dsize=g->shape[0]*g->shape[1]*2*18;
                              if (!data)
                              {
                                if (!(data=(double *)malloc(dsize*sizeof(double))))
                                   printf("send_gl_canvas: out of memory allocating triangle array\n");
                              }

                              if (data)
                              {
                                 create_2d_gl_grid(g,cmask,gc_opts.mask_val,ibbox,&data[dindex],&gdsize);
                                 dindex=dindex+gdsize;
                                 if (gc_opts.concat_grids==CONCAT_NONE)
                                 {
                                    sdf_to_explorer(data,dindex,ctime);
                                    free(data);
                                    data=0;
                                 }
                              }
                           }
                           else
                           {
                              send_gl_canvas_1g(di[i].r->name,di[i].ts->time,g,cmask,gc_opts.mask_val,cmode,ibbox);
                              if (gc_opts.concat_grids==CONCAT_NONE) refresh_gl_display();
                           }
                           if (g->amr_mask && view==view_max) { gfree(g->amr_mask); g->amr_mask=0; }
                        }
                        else 
                        {
                           if (to_explorer) dsize=dsize+g->shape[0]*g->shape[1]*2*18;
                           else set_bbox(g,cmask,gc_opts.mask_val,cmode); 
                        }
                        if (view==GL_HVS) switch_gl_canvas(GL_LOCAL_VIEW);
                     }
                     first=0;
                  }
                  g=next_g(&di[i]);
                  if (self_mask) mask=g; else if (mask) mask=next_g(&mi[i]);
               }
            }
            loop_n--;
         }

         i=0;
         while(i<num_r)
         {
            if (!(IS_IT_VALID(&di[i])))
            {
               for (j=i; j<(num_r-1); j++)
               {
                  di[j]=di[j+1];
                  mi[j]=mi[j+1];
               }
               num_r--;
            }
            else i++;
         }

         if (!first && gc_opts.concat_grids==CONCAT_TIME)
         {
            if (!to_explorer) 
            {
               refresh_gl_display(); 
               if (refresh_hvs)
               {
                  switch_gl_canvas(GL_HVS);
                  refresh_gl_display();
                  switch_gl_canvas(GL_LOCAL_VIEW);
                  refresh_hvs=0;
               }
            }
            else if (data)
            {
               sdf_to_explorer(data,dindex,ctime);
               free(data);
               data=0;
            }
         }
         step++;
      }

      r=reg_list;
      while(r)
      {
         FREE_REG(r);
         r=r->next;
      }
   }
               
   FREE_REG_LIST;

   if (!to_explorer && (gc_opts.concat_grids==CONCAT_ALL || only_g)) 
   {
      refresh_gl_display();
      if (refresh_hvs)
      {
         switch_gl_canvas(GL_HVS);
         refresh_gl_display();
         switch_gl_canvas(GL_LOCAL_VIEW);
         refresh_hvs=0;
      }
   }
   if (to_explorer && gc_opts.concat_grids==CONCAT_ALL && data)
   {
      sdf_to_explorer(data,dindex,ctime);
      free(data);
   }

   if (gl_canvas_open[GL_HVS] && !any_hvs)
   {
      switch_gl_canvas(GL_HVS);
      clear_gl_display_lists();
      refresh_gl_display();
      switch_gl_canvas(GL_LOCAL_VIEW);
   }

   return;
}

