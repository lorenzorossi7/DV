/*
 
 scr_save.c : function to save the GL buffer to a file.
  
 currently the file is saved in jpeg format

*/

#include "scr_save.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <jpeglib.h>

//-----------------------------------------------------------------------------
// function assumes that the GL context is valid. 
//-----------------------------------------------------------------------------
void scr_save(const char *file_name,int jpeg_qf)
{
   int vp[4],image_size,width,height,row;
   char *image;
   struct jpeg_compress_struct cinfo;
   struct jpeg_error_mgr jerr;
   JSAMPROW rp;
   FILE *file=0;

   // obtain the image dimension, [vp]=[x,y,width,height]
   glGetIntegerv(GL_VIEWPORT,vp);

   width=vp[2];
   height=vp[3];

   printf("vp:%i,%i,%i,%i\n",vp[0],vp[1],width,height);

   image_size=sizeof(char)*3*width*height;
   if (!(image=(char *)malloc(image_size)))
   {
      printf("scr_save:: out of memory ... can't save the image\n");
      return;
   }

   glPixelStorei(GL_PACK_ALIGNMENT,1);
   glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

   // following 'libjpeg.doc' to use the jpeg library:
   cinfo.err=jpeg_std_error(&jerr);
   jpeg_create_compress(&cinfo);
   if (!(file=fopen(file_name,"wb")))
   {
      printf("scr_save:: cannot open file %s for writing\n",file_name);
      goto cleanup;
   }
   jpeg_stdio_dest(&cinfo,file); 
   cinfo.image_width=width;
   cinfo.image_height=height;
   cinfo.input_components=3;
   cinfo.in_color_space=JCS_RGB;
   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo,jpeg_qf,0);
   jpeg_start_compress(&cinfo, TRUE);
   
   // the jpeg library wants the image from top-to-bottom,
   // glReadPixels() returns bottom-to-top, so invert below:
 
   for(row=height-1; row>=0; row--)
   {
      rp=&image[row*width*3];
      jpeg_write_scanlines(&cinfo,&rp,1);
   }
   
   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);

cleanup:
   if (file) fclose(file);
   free(image);
   return;
}
