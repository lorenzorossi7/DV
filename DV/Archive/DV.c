/*
   Data Vault version 0.1
   Copyright 2000/2001 F.Pretorius & M.W Choptuik
   
   main routine. starts the server and waits for incoming SDF streams.
   (client/server mechanism taken from the cliser example)
*/

#include "Cliser.h" 
#include "reg.h"
#include "misc.h"
#include "DV.h"
#include "instr.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "DV_gui.h"

// data-vault server 

int stop_dv_service;
int refresh_GUI_now;

int dv_service(int ss, int cs)
{
   FILE *stream;   

   int ltrace=1;

   IFL printf("dv_service invoked::\n");

   // a somewhat clumsy mechanism to stop the server from the GUI,
   // but ser0_stop_DV() didn't work (if that is even the 'correct' way to stop server)

   if (stop_dv_service) return 0;

   if (!(stream=fdopen(cs,"r"))) 
   {
      printf("dv_service: ERROR opening input stream\n");
      close(cs);
      return 1;
   }

   refresh_GUI_now=0;
   instr_read_sdf_stream(stream);
   if (refresh_GUI_now && auto_refresh)
      create_browser_list();

clean_up:
   fclose(stream);
   close(cs);
   return 1;
}

int main(int argc,char **argv)
{
   int ret,ss;
   pthread_t thread;
   void *arg;

   if (!initialize_dv())
   {
      printf("error initializing dv\n");
      exit(-1);
   }

   if (!(ss=ser0_start_DV(DV_PORT)))
   {
      printf("unable to start server on port %i\n",DV_PORT);
      exit(-1);
   }

   if (DV_PORT==5005)
   {
      printf("warning: DV_PORT=BBH_PORT=5005, so the 'Send to Accept...' functions will"
             " not work properly.\nTo use them recompile with DV_PORT=5010 (say)\n");
   }
      
   // start the GUI on a seperate thread
   // (default attributes and no argument to start_gui for now)
  
   arg=0;
   stop_dv_service=0;
   if ((ret=pthread_create(&thread,(pthread_attr_t *)0,start_gui,arg)))
   {
      printf("pthread_create failed, return code=%i. Cannot start GUI\n",ret);
      exit(-1);
   }

   printf("starting the data-vault server ...\n");

   ser0_serve_block_DV(ss,dv_service);

   printf("... shutting down the data-vault\n");
   shut_down_dv();
}

