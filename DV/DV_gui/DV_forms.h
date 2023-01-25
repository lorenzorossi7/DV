/** Header file generated with fdesign on Fri Nov  4 18:15:40 2011.**/

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
extern void send_to_local_cb(FL_OBJECT *, long);
extern void merge_cb(FL_OBJECT *, long);
extern void hvs_cb(FL_OBJECT *, long);
extern void browser_up_cb(FL_OBJECT *, long);
extern void browser_down_cb(FL_OBJECT *, long);
extern void mstest_menu_cb(FL_OBJECT *, long);

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
extern void func_filter_hist_cb(FL_OBJECT *, long);
extern void func_arg_hist_cb(FL_OBJECT *, long);

extern void gl_home_cb(FL_OBJECT *, long);
extern void null_cb(FL_OBJECT *, long);
extern void gl_options_menu_cb(FL_OBJECT *, long);
extern void gl_zoom_hist_cb(FL_OBJECT *, long);
extern void gl_theta_hist_cb(FL_OBJECT *, long);
extern void gl_phi_hist_cb(FL_OBJECT *, long);
extern void gl_home_1d_cb(FL_OBJECT *, long);

extern void sl_opts_monotone_cb(FL_OBJECT *, long);
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
extern void sl_opts_fg_color_cb(FL_OBJECT *, long);
extern void sl_opts_levelcolor_cb(FL_OBJECT *, long);
extern void sl_3D_bbox_only_cb(FL_OBJECT *, long);
extern void sl_opts_which_canvas_cb(FL_OBJECT *, long);
extern void sl_color_clip_cb(FL_OBJECT *, long);
extern void sl_opts_cmap_scheme_cb(FL_OBJECT *, long);

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
extern void gl_ann_anti_alias_cb(FL_OBJECT *, long);
extern void gl_ann_bg_color_cb(FL_OBJECT *, long);
extern void gl_ann_which_canvas_cb(FL_OBJECT *, long);

extern void db_h_slider_cb(FL_OBJECT *, long);
extern void db_v_slider_cb(FL_OBJECT *, long);
extern void db_data_fs_cb(FL_OBJECT *, long);
extern void db_coord_fs_cb(FL_OBJECT *, long);
extern void db_screen_dump_cb(FL_OBJECT *, long);
extern void db_reverse_y_cb(FL_OBJECT *, long);
extern void db_sm_save_cb(FL_OBJECT *, long);

extern void hvs_options_menu_cb(FL_OBJECT *, long);
extern void hvs_home_cb(FL_OBJECT *, long);
extern void hvs_x_slice_cb(FL_OBJECT *, long);
extern void hvs_y_slice_cb(FL_OBJECT *, long);
extern void hvs_z_slice_cb(FL_OBJECT *, long);
extern void hvs_slice_slider_cb(FL_OBJECT *, long);
extern void hvs_inc_slice_cb(FL_OBJECT *, long);
extern void hvs_dec_slice_cb(FL_OBJECT *, long);

extern void mstest_item1_browser_cb(FL_OBJECT *, long);
extern void mstest_item1_func_cb(FL_OBJECT *, long);
extern void mstest_item1_new_cb(FL_OBJECT *, long);
extern void mstest_item1_input_cb(FL_OBJECT *, long);
extern void mstest_item1_delete_cb(FL_OBJECT *, long);
extern void mstest_item1_input_up_cb(FL_OBJECT *, long);
extern void mstest_item1_input_down_cb(FL_OBJECT *, long);
extern void mstest_item1_up_cb(FL_OBJECT *, long);
extern void mstest_item1_down_cb(FL_OBJECT *, long);
extern void mstest_item1_input_remove_cb(FL_OBJECT *, long);
extern void null_cb(FL_OBJECT *, long);
extern void mstest_item1_go_cb(FL_OBJECT *, long);
extern void mstest_item1_add_cb(FL_OBJECT *, long);

extern void mstest_userfdef_browser_cb(FL_OBJECT *, long);
extern void mstest_userfdef_AB_etc_cb(FL_OBJECT *, long);
extern void mstest_userfdef_apply_cb(FL_OBJECT *, long);
extern void mstest_userfdef_func_cb(FL_OBJECT *, long);
extern void mstest_userfdef_macro_cb(FL_OBJECT *, long);
extern void mstest_userfdef_delete_cb(FL_OBJECT *, long);
extern void mstest_userfdef_up_cb(FL_OBJECT *, long);
extern void mstest_userfdef_down_cb(FL_OBJECT *, long);

extern void f_select_cb(FL_OBJECT *, long);
extern void f_select_scmin_cb(FL_OBJECT *, long);
extern void f_select_scmax_cb(FL_OBJECT *, long);
extern void f_select_cm_min_cb(FL_OBJECT *, long);
extern void f_select_cm_max_cb(FL_OBJECT *, long);


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
	FL_OBJECT *data_browser;
	FL_OBJECT *send_to_local;
	FL_OBJECT *hvs_window;
	FL_OBJECT *up;
	FL_OBJECT *down;
	FL_OBJECT *mstest;
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
	FL_OBJECT *diff_filter;
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
	FL_OBJECT *func;
	FL_OBJECT *apply_filter;
	FL_OBJECT *filter_hist;
	FL_OBJECT *arg_hist;
	FL_OBJECT *givec;
} FD_DV_func_window;

extern FD_DV_func_window * create_form_DV_func_window(void);
typedef struct {
	FL_FORM *DV_gl_canvas_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *key_frame_ind;
	FL_OBJECT *gl_canvas;
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
	FL_OBJECT *monotone;
	FL_OBJECT *scale;
	FL_OBJECT *cmap_range_minz;
	FL_OBJECT *cmap_range_maxz;
	FL_OBJECT *amr;
	FL_OBJECT *use_mask;
	FL_OBJECT *mask;
	FL_OBJECT *mask_val;
	FL_OBJECT *use_givec;
	FL_OBJECT *givec;
	FL_OBJECT *coarsen_step;
	FL_OBJECT *scale_hist;
	FL_OBJECT *cmap_hist;
	FL_OBJECT *cmap_type;
	FL_OBJECT *zmin_clip;
	FL_OBJECT *zmax_clip;
	FL_OBJECT *sync_t;
	FL_OBJECT *concat_grids;
	FL_OBJECT *givec_hist;
	FL_OBJECT *ignore_ccoords;
	FL_OBJECT *fg_color;
	FL_OBJECT *levelcolor;
	FL_OBJECT *bbox_only;
	FL_OBJECT *which_canvas;
	FL_OBJECT *color_clip;
	FL_OBJECT *cmap_scheme;
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
	FL_OBJECT *bbox_width;
	FL_OBJECT *bb_color;
	FL_OBJECT *point_size;
	FL_OBJECT *line_width;
	FL_OBJECT *anti_alias;
	FL_OBJECT *bg_color;
	FL_OBJECT *which_canvas;
	FL_OBJECT *autoscale;
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
typedef struct {
	FL_FORM *DV_hvs_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *gl_canvas;
	FL_OBJECT *hvs_options_menu;
	FL_OBJECT *home_button;
	FL_OBJECT *x_input;
	FL_OBJECT *y_input;
	FL_OBJECT *z_input;
	FL_OBJECT *slice_slider;
	FL_OBJECT *inc_slice;
	FL_OBJECT *dec_slice;
	FL_OBJECT *x_button;
	FL_OBJECT *y_button;
	FL_OBJECT *z_button;
} FD_DV_hvs_window;

extern FD_DV_hvs_window * create_form_DV_hvs_window(void);
typedef struct {
	FL_FORM *DV_mstest_item1_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *mstest_item1_browser;
	FL_OBJECT *mstest_item1_func;
	FL_OBJECT *mstest_item1_edit;
	FL_OBJECT *mstest_item1_input;
	FL_OBJECT *mstest_item1_delete;
	FL_OBJECT *mstest_item1_input_up;
	FL_OBJECT *mstest_item1_input_down;
	FL_OBJECT *mstest_item1_info;
	FL_OBJECT *mstest_item1_up;
	FL_OBJECT *mstest_item1_down;
	FL_OBJECT *mstest_item1_new;
	FL_OBJECT *mstest_item1_input_remove;
	FL_OBJECT *mstest_item1_delmode;
	FL_OBJECT *mstest_item1_go;
	FL_OBJECT *mstest_item1_add;
	FL_OBJECT *mstest_item1_godelmode;
	FL_OBJECT *mstest_item1_gomode;
	FL_OBJECT *filter;
} FD_DV_mstest_item1_window;

extern FD_DV_mstest_item1_window * create_form_DV_mstest_item1_window(void);
typedef struct {
	FL_FORM *DV_mstest_userfdef_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *mstest_userfdef_browser;
	FL_OBJECT *mstest_userfdef_reg_a;
	FL_OBJECT *mstest_userfdef_reg_b;
	FL_OBJECT *mstest_userfdef_mask;
	FL_OBJECT *mstest_userfdef_done;
	FL_OBJECT *mstest_userfdef_args;
	FL_OBJECT *mstest_userfdef_new;
	FL_OBJECT *mstest_userfdef_mvalue;
	FL_OBJECT *mstest_userfdef_func;
	FL_OBJECT *mstest_userfdef_macro;
	FL_OBJECT *mstest_userfdef_name;
	FL_OBJECT *mstest_userfdef_delete;
	FL_OBJECT *mstest_userfdef_up;
	FL_OBJECT *mstest_userfdef_down;
} FD_DV_mstest_userfdef_window;

extern FD_DV_mstest_userfdef_window * create_form_DV_mstest_userfdef_window(void);
typedef struct {
	FL_FORM *DV_f_select_window;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *wf;
	FL_OBJECT *select;
	FL_OBJECT *show_min;
	FL_OBJECT *show_max;
	FL_OBJECT *set_cm_min;
	FL_OBJECT *set_cm_max;
	FL_OBJECT *cm_min;
	FL_OBJECT *cm_max;
} FD_DV_f_select_window;

extern FD_DV_f_select_window * create_form_DV_f_select_window(void);

#endif /* FD_DV_main_window_h_ */
