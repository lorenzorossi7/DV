/** Header file generated with fdesign on Tue Dec  4 13:42:16 2001.**/

#ifndef FD_DV_main_window_h_
#define FD_DV_main_window_h_

/** Callbacks, globals and object handlers **/
extern void file_menu_cb(FL_OBJECT *, long);
extern void dv_browser_cb(FL_OBJECT *, long);
extern void debug_menu_cb(FL_OBJECT *, long);
extern void edit_menu_cb(FL_OBJECT *, long);
extern void view_menu_cb(FL_OBJECT *, long);
extern void delete_cb(FL_OBJECT *, long);
extern void diff_cb(FL_OBJECT *, long);
extern void functions_cb(FL_OBJECT *, long);
extern void options_cb(FL_OBJECT *, long);
extern void data_browser_cb(FL_OBJECT *, long);
extern void send_to_explorer_cb(FL_OBJECT *, long);
extern void send_to_acceptSDF_cb(FL_OBJECT *, long);
extern void send_to_local_cb(FL_OBJECT *, long);

extern void diff_browser_cb(FL_OBJECT *, long);
extern void diff_wrt_cb(FL_OBJECT *, long);
extern void diff_mask_cb(FL_OBJECT *, long);
extern void null_cb(FL_OBJECT *, long);
extern void diff_go_cb(FL_OBJECT *, long);

extern void func_browser_cb(FL_OBJECT *, long);
extern void func_reg_a_cb(FL_OBJECT *, long);
extern void func_reg_b_cb(FL_OBJECT *, long);
extern void null_cb(FL_OBJECT *, long);
extern void func_go_cb(FL_OBJECT *, long);
extern void func_cb(FL_OBJECT *, long);

extern void gl_home_cb(FL_OBJECT *, long);
extern void null_cb(FL_OBJECT *, long);
extern void gl_options_menu_cb(FL_OBJECT *, long);
extern void gl_zoom_hist_cb(FL_OBJECT *, long);
extern void gl_theta_hist_cb(FL_OBJECT *, long);
extern void gl_phi_hist_cb(FL_OBJECT *, long);
extern void gl_home_1d_cb(FL_OBJECT *, long);

extern void null_cb(FL_OBJECT *, long);
extern void sl_opts_amr_cb(FL_OBJECT *, long);
extern void sl_opts_use_mask_cb(FL_OBJECT *, long);
extern void sl_opts_use_givec_cb(FL_OBJECT *, long);
extern void sl_opts_coarsen_cb(FL_OBJECT *, long);
extern void sl_opts_scale_hist_cb(FL_OBJECT *, long);
extern void sl_opts_cmap_hist_cb(FL_OBJECT *, long);
extern void sl_opts_sync_cb(FL_OBJECT *, long);
extern void sl_opts_concat_grids_cb(FL_OBJECT *, long);
extern void sl_opts_givec_hist_cb(FL_OBJECT *, long);
extern void sl_opts_ignore_cb(FL_OBJECT *, long);
extern void sl_opts_zoom_cb(FL_OBJECT *, long);

extern void null_cb(FL_OBJECT *, long);
extern void capture_browse_cb(FL_OBJECT *, long);
extern void capture_cb(FL_OBJECT *, long);

extern void null_cb(FL_OBJECT *, long);
extern void gl_ann_edit_cb(FL_OBJECT *, long);
extern void gl_ann_format_str_cb(FL_OBJECT *, long);
extern void gl_ann_font_cb(FL_OBJECT *, long);
extern void gl_ann_color_cb(FL_OBJECT *, long);
extern void gl_ann_pos_text_cb(FL_OBJECT *, long);
extern void gl_ann_disp_cmap_cb(FL_OBJECT *, long);
extern void gl_ann_pos_cmap_cb(FL_OBJECT *, long);
extern void gl_ann_border_color_cb(FL_OBJECT *, long);
extern void gl_ann_border_width_cb(FL_OBJECT *, long);
extern void gl_ann_bbox_width_cb(FL_OBJECT *, long);
extern void gl_ann_bbox_color_cb(FL_OBJECT *, long);
extern void gl_ann_1d_pt_size_cb(FL_OBJECT *, long);
extern void gl_ann_1d_line_width_cb(FL_OBJECT *, long);

extern void db_h_slider_cb(FL_OBJECT *, long);
extern void db_v_slider_cb(FL_OBJECT *, long);
extern void db_data_fs_cb(FL_OBJECT *, long);
extern void db_coord_fs_cb(FL_OBJECT *, long);
extern void db_screen_dump_cb(FL_OBJECT *, long);
extern void db_reverse_y_cb(FL_OBJECT *, long);
extern void db_sm_save_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *DV_main_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *file_menu;
	FL_OBJECT *dv_browser;
	FL_OBJECT *edit_menu;
	FL_OBJECT *view_menu;
	FL_OBJECT *delete_menu;
	FL_OBJECT *diff;
	FL_OBJECT *functions;
	FL_OBJECT *send_to_explorer_menu;
	FL_OBJECT *send_to_acceptSDF_menu;
	FL_OBJECT *send_to_local;
} FD_DV_main_window;

extern FD_DV_main_window * create_form_DV_main_window(void);
typedef struct {
	FL_FORM *DV_diff_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *diff_browser;
	FL_OBJECT *diff_wrt;
	FL_OBJECT *diff_mask;
	FL_OBJECT *diff_mask_val;
	FL_OBJECT *diff_new_reg;
	FL_OBJECT *diff_go;
	FL_OBJECT *diff_overwrite;
	FL_OBJECT *diff_stencil;
} FD_DV_diff_window;

extern FD_DV_diff_window * create_form_DV_diff_window(void);
typedef struct {
	FL_FORM *DV_func_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *func_browser;
	FL_OBJECT *func_reg_a;
	FL_OBJECT *func_reg_b;
	FL_OBJECT *func_mask;
	FL_OBJECT *func_overwrite;
	FL_OBJECT *func_go;
	FL_OBJECT *func_args;
	FL_OBJECT *func_new_reg;
	FL_OBJECT *func_info;
	FL_OBJECT *func_mask_val;
	FL_OBJECT *givec;
	FL_OBJECT *func;
} FD_DV_func_window;

extern FD_DV_func_window * create_form_DV_func_window(void);
typedef struct {
	FL_FORM *DV_gl_canvas_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *gl_canvas;
	FL_OBJECT *gl_canvas_text;
	FL_OBJECT *home_button;
	FL_OBJECT *gl_zoom;
	FL_OBJECT *gl_theta;
	FL_OBJECT *gl_phi;
	FL_OBJECT *gl_options_menu;
	FL_OBJECT *gl_zoom_hist;
	FL_OBJECT *gl_theta_hist;
	FL_OBJECT *gl_phi_hist;
} FD_DV_gl_canvas_window;

extern FD_DV_gl_canvas_window * create_form_DV_gl_canvas_window(void);
typedef struct {
	FL_FORM *DV_sl_opts_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *scale;
	FL_OBJECT *cmap_range_minz;
	FL_OBJECT *cmap_range_maxz;
	FL_OBJECT *amr;
	FL_OBJECT *use_mask;
	FL_OBJECT *mask;
	FL_OBJECT *mask_val;
	FL_OBJECT *use_givec;
	FL_OBJECT *givec;
	FL_OBJECT *coarsen_step_size;
	FL_OBJECT *scale_hist;
	FL_OBJECT *cmap_hist;
	FL_OBJECT *cmap_type;
	FL_OBJECT *zmin_clip;
	FL_OBJECT *zmax_clip;
	FL_OBJECT *sync;
	FL_OBJECT *concat;
	FL_OBJECT *givec_hist;
	FL_OBJECT *sl_opts_ingore_ccoords;
	FL_OBJECT *zoom_a;
	FL_OBJECT *zoom_b;
} FD_DV_sl_opts_window;

extern FD_DV_sl_opts_window * create_form_DV_sl_opts_window(void);
typedef struct {
	FL_FORM *DV_capture_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *dir;
	FL_OBJECT *browse;
	FL_OBJECT *format;
	FL_OBJECT *first_index;
	FL_OBJECT *start_stop;
	FL_OBJECT *jpeg_qf;
} FD_DV_capture_window;

extern FD_DV_capture_window * create_form_DV_capture_window(void);
typedef struct {
	FL_FORM *DV_annotation_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *edit;
	FL_OBJECT *format_str;
	FL_OBJECT *var_options;
	FL_OBJECT *font;
	FL_OBJECT *color;
	FL_OBJECT *pos_text;
	FL_OBJECT *disp_cmap;
	FL_OBJECT *pos_cmap;
	FL_OBJECT *border_color;
	FL_OBJECT *border_width;
} FD_DV_annotation_window;

extern FD_DV_annotation_window * create_form_DV_annotation_window(void);
typedef struct {
	FL_FORM *DV_dbrowser_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *info;
	FL_OBJECT *data;
	FL_OBJECT *h_slider;
	FL_OBJECT *v_slider;
	FL_OBJECT *i_info;
	FL_OBJECT *j_info;
	FL_OBJECT *data_fs;
	FL_OBJECT *coord_fs;
	FL_OBJECT *screen_dump;
	FL_OBJECT *sm_save;
} FD_DV_dbrowser_window;

extern FD_DV_dbrowser_window * create_form_DV_dbrowser_window(void);

#endif /* FD_DV_main_window_h_ */
