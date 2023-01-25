#ifndef _REG_H 
#define _REG_H

#include <semaphore.h>

/*==========================================================================================*/
/* reg.h -- register definition and functions                                               */
/*==========================================================================================*/

#define MAX_DIM 3
#define MAX_NAME_LENGTH 256

/* coordinate types.
   a 'semi-unform' coordinate is a uniform coordinate system,
   with a second, curvilinear coordinate system attached (ccoords)
   for visualization purposes. So, level sorting (to
   preserve the AMR structure), clipping, *some* grid functions, etc... 
   treat it as a uniform grid.
   NOTE: galloc does NOT initialize ccoords, it only allocates the
   required memory. */

#define COORD_UNIFORM 0
#define COORD_PERIMETER 1
#define COORD_CURVILINEAR 2
#define COORD_SEMI_UNIFORM 3

/* default fuzzy values */

#define FUZZY 1.0e-9

typedef struct grid
{
   struct grid *next;           /* next grid in list */
   struct grid *prev;           /* prev grid in list */
   int dim;                     /* dimension  */
   int coord_type;              /* uniform, perimeter, ... */
   int shape[MAX_DIM];          /* size of array */
   double time;                 /* time of grid */
   double *coords;              /* coordinate info (bounding box, perimeter values,...) */
   double *data;                /* data array */
   double *ccoords;             /* for 'semi-uniform' grids; pointer to curvilinear coordinates  */
   int selected;                /* * in GUI browser list, or via ivec */
   int browser_index;           /* * index if displayed in browser */
   int giv_selected;            /* * used by s_iter */
   struct grid *amr_mask;       /* * used by send_gl_canvas */
} grid;

typedef struct level
{
   struct level *next;          /* next level in list */
   struct level *prev;          /* prev level in list */
   double dx;                   /* dx; taken from first dimension */
   grid   *grids;               /* list of grids at this level */
   int selected;                /* * in GUI browser list, or via ivec */
   int browser_index;           /* * index if displayed in browser */
   int opened;                  /* * in browser */
   int ln;                      /* * global level number, used by s_iter */
} level;

typedef struct time_str
{
   struct time_str *next;       /* next time in list */
   struct time_str *prev;       /* prev level in list */
   double time;
   level *levels;               /* list of level structures, sorted by dx */
   int selected;                /* * in GUI browser list, or via ivec */
   int browser_index;           /* * index if displayed in browser */
   int opened;                  /* * in browser */
} time_str;

typedef struct reg
{
   struct reg *next;            /* next register in list */
   struct reg *prev;            /* prev register in list */
   time_str *ts;                /* list of time structures, sorted by time */
   char name[MAX_NAME_LENGTH];  /* unique name identifying register */
   char *coord_names[MAX_DIM+1];/* optional coordinate names. ('0' or 't', and '1','2',... will also be recognized) */
   double fuzzy_t;              /* fuzzy compare tolerances */
   double fuzzy_x[MAX_DIM];
   sem_t reg_lock;              /* to control concurrent access to the register (see comment (**) below) */
   int selected;                /* * in GUI browser list, or via ivec */
   int browser_index;           /* * index if displayed in browser */
   int opened;                  /* * in browser */
} reg;

/* NOTE:
   specifying the coordinate names in reg is a bit inconsistant with 
   the definition of a grid (where the dimension is declared). However, since
   the names are only for convenient reference to coordinates in *register*
   instructions, defining the names here makes sense.

   the variables above that have a * in the comment field are 'user defined'
   variables within the implied virtual machine specification. i.e., they
   are sets of name-value pairs that a user (like the GUI) can define for 
   some structure, and the virtual machine will have intructions for
   auto-initialization, reading and writing these values. 
*/
/*=============================================================================
   (**)
   Global variables and semaphores for safe, *limited* concurrent access;
   namely the server only adds grids, while other operations are performed by
   the GUI callback routines all executing on a single thread. 
   Then if we always lock the register list or register before reading/altering 
   the corresponding object, we'll be able to maintain the intergrity
   of the data-vault and could be free of deadlocks.
   (one rule of thumb to help avoid deadlocks here --- respect the locking 
    hierarchy, i.e. don't lock the register list if you already own
    register locks).
  ============================================================================= */

extern reg *reg_list;
extern sem_t reg_list_lock;

/*=============================================================================
   register functions
  =============================================================================*/

#define LOCK_REG_LIST sem_wait(&reg_list_lock)
#define FREE_REG_LIST sem_post(&reg_list_lock)
 
#define LOCK_REG(r) sem_wait(&(r->reg_lock));
#define FREE_REG(r) sem_post(&(r->reg_lock)); 

/* for add_grid_str: */
#define REPLACE_IDENTICAL_GRIDS 1

int add_grid(char *name, double t, int dim, int coord_type, int *shape, double *coords, double *ccoords, double *data);
int add_grid_str(char *reg_name, grid *g);
grid *galloc(int dim, int coord_type, int *shape, double t, double *coords, double *ccoords, double *data);
void gfree(grid *g);
reg *find_reg(char *name, int lock_rl, int free_rl);
int get_coord_index(reg *r, char *cx);  
int sizeof_data(grid *g);
int sizeof_coords(grid *g);
int delete_reg(char *reg_name);
void delete_grids(int mode);
/* delete_grids mode: */
#define DM_SELECTED_AND_OPENED 1
#define DM_SELECTED 2
#define DM_ALL 3 
#define DM_LAST_TIME_OF_SELECTED 4 
int clone_reg(char *old_reg, char *new_reg);
int rename_reg(char *old_name, char *new_name);
/* merge modes */
#define MERGE_SELECTED 1
#define MERGE_ALL 2 
int merge_reg(int mode);
int set_coord_name(char *reg_name, char *coord_name, int coord);
int gridcmp(grid *g1,grid *g2);
int grid_str_eq(reg *r1, reg *r2);

grid *gclip(grid *g, int *ibbox);
/* defines wether the clipping operation should */
/* produce (dim-1) grids when selecting a slice of a (dim) grid */
extern int clip_reduce_dim;

int initialize_dv(void);
void shut_down_dv(void);

#endif /* _REG_H */
