/* Hack to ensure that fdesign-emitted code includes the needed
   GL headers.

   Makefile replaces reference to forms.h with myforms.h whenever
   DV_forms.c is modified. */

#include <GL/gl.h>
#include <GL/glx.h>
#include <forms.h>
