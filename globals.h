/* globals.h */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "config.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

#define MAX_SYNCHS 10
#define MAX_TABLES 100
#define MAX_VNUP_DIMS 6
#define NUM_MERGE_COEFF 10
#define NO_FILE_STR "---none---"
#define MERGE_STR "Merge"

/* types of views */
#define TRANSVERSE 0
#define SAGITTAL 1
#define CORONAL 2

/* types of ROI's */
#define ROI_NONE 0
#define ROI_CUBE 1
#define ROI_SPHERE 2

/* types of values */
#define VALUE_NONE 0
#define VALUE_REAL 1
#define VALUE_VOXEL 2
#define VALUE_RGBA 3

/* a few macros */
#ifndef SQR
#define SQR(x) x*x
#endif

#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <volume_io.h>

/* forward declarations of structures */
struct pane_info_struct;
struct view_info_struct;

typedef struct pane_info_struct *Pane_info;
typedef struct view_info_struct *View_info;

/* Lookup table structure */
typedef struct {
   char     name[256];
   int      nentries;
   int      vector_length;
   double  *table;
} Lookup_Table;

typedef struct {
   int      virgin;
   int      idx;

   GtkWidget *synch_row_label;
   GtkWidget *synch_x;
   GtkWidget *synch_y;
   GtkWidget *synch_z;
   GtkWidget *synch_t;
   GtkWidget *synch_scale_button;
   GtkWidget *synch_rotation_button;
   GtkWidget *synch_translation_button;
   GtkWidget *synch_tilt_button;
   GtkWidget *synch_scale_pixmap;
   GtkWidget *synch_rotation_pixmap;
   GtkWidget *synch_translation_pixmap;
   GtkWidget *synch_tilt_pixmap;

   GtkObject *synch_x_adj;
   GtkObject *synch_y_adj;
   GtkObject *synch_z_adj;
   GtkObject *synch_t_adj;

   GtkObject *synch_scale_adj;
   GtkObject *synch_trans_x_adj;
   GtkObject *synch_trans_y_adj;
   GtkObject *synch_trans_z_adj;
   GtkObject *synch_rot_x_adj;
   GtkObject *synch_rot_y_adj;
   GtkObject *synch_rot_z_adj;
   GtkObject *synch_rot_phi_adj;
   GtkObject *synch_tilt_x_adj;
   GtkObject *synch_tilt_y_adj;
   GtkObject *synch_tilt_z_adj;

   GtkWidget *synch_remove_button;
   GtkWidget *synch_remove_pixmap;
} Synch_info;

/* view_info structure */
struct view_info_struct {
   int      type;

   unsigned char *texmap;
   int      texmap_id;
   int      texmap_size[2];
   double   texmap_stop[2];
   int      texmap_space;

   /* flags */
   int      reload_texmap;
   int      reload_image;
   int      reload_cmap;
   int      refresh_view;
   int      recalc_view;

   int      x_idx;
   int      y_idx;
   int      z_idx;

   /* mouse click stores (per view) */
   double   pix_start_pos[2];
   double   pix_size[2];

   /* window size in world co-ordinates */
   double   view_start[2];
   double   view_stop[2];

   /* texture map quad coordinates */
   double   q0[3];
   double   q1[3];
   double   q2[3];
   double   q3[3];

   /* scale, translation and rotation and tilt information */
   double   GLscalefac;
   int      lock_scale;

   double   GLtrans_x;
   double   GLtrans_y;
   double   GLtrans_z;
   int      lock_x_trans;
   int      lock_y_trans;
   int      lock_z_trans;

   double   rot_vec[3];
   double   rot_phi;
   double   rot_quat[4];
   int      lock_x_rot;
   int      lock_y_rot;
   int      lock_z_rot;
   int      lock_phi_rot;

   int      tilt;
   double   tilt_quat[4];
   double   tilt_w[3];
   double   tilt_v[3];
   int      lock_x_tilt;
   int      lock_y_tilt;
   int      lock_z_tilt;

   /* projection and viewing matricies */
   double   projMatrix[16];
   double   modelMatrix[16];
   int      viewport[4];

   int      translating;
   int      rotating;
   int      tilting;

   /* gluQuadric for fun stuff */
   void    *quadric;

   /* Widgets */
   GtkWidget *view_frame;
   GtkWidget *glarea;
};

/* pane_info structure */
struct pane_info_struct {
   GString *file_name;
   GString *file_basename;

   /* merge and synch information */
   int      merge;
   Pane_info merge_panes[NUM_MERGE_COEFF];
   double   merge_coeff[NUM_MERGE_COEFF];
   gboolean merge_link[NUM_MERGE_COEFF];
   Synch_info *synch;

   /* flags and properties */
   int      position;
   int      draw;
   int      draw_fast;
   int      minimise;
   int      linear_interp;
   int      vector;
   int      perspective;
   int      bounding_box;
   int      slice_box;
   int      crosshair;
   double   crosshair_size;
   int      use_vox_coords;

   int      link_scales;
   int      link_trans;
   int      link_rots;
   int      link_tilts;

   View_info c_view;
   GPtrArray *views;

   int      n_dims;
   int      sizes[MAX_VNUP_DIMS];
   double   steps[MAX_VNUP_DIMS];
   double   starts[MAX_VNUP_DIMS];
   double   stops[MAX_VNUP_DIMS];
   double   real_min;
   double   real_max;

   /* cmap stuff */
   double   pane_min;
   double   pane_max;
   double   pane_min_value;
   double   pane_max_value;
   Lookup_Table *cmap_ptr;
   float    cmap_r[256];
   float    cmap_g[256];
   float    cmap_b[256];
   float    cmap_a[256];
   double   alpha_thresh;

   /* value and ROI display */
   int      value_type;
   double   voxel_value;

   int      roi_type;
   double   roi_x;
   double   roi_y;
   double   roi_z;
   int      roi_gauss;
   double   roi_fwhm;

   /* coordinates and COV */
   double   v[MAX_VNUP_DIMS];
   double   w[MAX_VNUP_DIMS];
   double   cov[MAX_VNUP_DIMS];

   /* vector variables */
   int      mult_vect;
   int      vect_points;
   int      RGB_vect;
   double   vect_col[4];
   double   vect_point_col[4];
   double   vect_mult;
   double   vect_alph_mult;
   double   vect_floor;
   double   vect_ceil;

   /* volume information and linear transform */
   double   perc_input;
   Volume   volume;
   volume_input_struct input_info;
   General_transform *transform;

   /* pane widgets */
   GtkWidget *hbox_pane;
   GtkWidget *vsep_eventbox;
   GtkWidget *vsep;
   GtkWidget *vbox_pane;

   GtkWidget *eventbox_label;
   GtkWidget *hbox_label;
   GtkWidget *left_button;
   GtkWidget *arrow_left;
   GtkWidget *right_button;
   GtkWidget *arrow_right;
   GtkWidget *sync_button;
   GtkWidget *pane_label;
   GtkWidget *minimise_button;
   GtkWidget *minimise_pixmap;
   GtkWidget *close_button;
   GtkWidget *close_pixmap;

   GtkWidget *hsep_0;
   GtkWidget *vbox_img;
   GtkWidget *hsep_1;
   GtkWidget *hbox_values;
   GtkWidget *hbox_values_vox;
   GtkWidget *vox_value_button;
   GtkWidget *vox_value;
   GtkWidget *hbox_values_roi;
   GtkWidget *roi_value_button;
   GtkWidget *roi_value;
   GtkWidget *hsep_2;
   GtkWidget *hbox_coord;
   GtkWidget *coord_button;
   GtkWidget *coord_vx;
   GtkWidget *coord_vy;
   GtkWidget *coord_vz;
   GtkWidget *coord_vt;
   GtkWidget *coord_wx;
   GtkWidget *coord_wy;
   GtkWidget *coord_wz;
   GtkWidget *coord_wt;

   GtkWidget *hsep_3;

   GtkWidget *range_table;
   GtkWidget *range_min_val;
   GtkWidget *range_min_scale;
   GtkWidget *range_max_scale;
   GtkWidget *range_max_val;

   GtkWidget *hbox_cmap;
   GtkWidget *cmap_grey_radio;
   GtkWidget *cmap_hot_radio;
   GtkWidget *cmap_spect_radio;
   GtkWidget *cmap_bluered_radio;
   GtkWidget *cmap_combo;
   GtkWidget *cmap_combo_entry;

   GtkWidget *hsep_4;

   GtkWidget *hbox_file_open;
   GtkWidget *file_open;
   GtkWidget *file_open_combo;

   /* adjustments */
   GtkObject *coord_vx_adj;
   GtkObject *coord_vy_adj;
   GtkObject *coord_vz_adj;
   GtkObject *coord_vt_adj;

   GtkObject *coord_wx_adj;
   GtkObject *coord_wy_adj;
   GtkObject *coord_wz_adj;
   GtkObject *coord_wt_adj;

   GtkObject *range_min_adj;
   GtkObject *range_max_adj;
};

typedef struct {
   GtkWidget *pane_combo;
   GtkWidget *pane_combo_entry;
   GtkWidget *minimise_checkbutton;

   GtkWidget *wx_spinbutton;
   GtkWidget *wy_spinbutton;
   GtkWidget *wz_spinbutton;
   GtkWidget *wt_spinbutton;

   GtkWidget *vx_spinbutton;
   GtkWidget *vy_spinbutton;
   GtkWidget *vz_spinbutton;
   GtkWidget *vt_spinbutton;

   GtkWidget *crosshair_checkbutton;
   GtkWidget *perspective_checkbutton;
   GtkWidget *vector_checkbutton;
   GtkWidget *slicebox_checkbutton;
   GtkWidget *linear_interp_checkbutton;
   GtkWidget *bbox_checkbutton;
   GtkObject *crosshair_spinbutton_adj;
   GtkWidget *crosshair_spinbutton;

   /* view link togglebuttons */
   GtkWidget *scale_link_button;
   GtkWidget *trans_link_button;
   GtkWidget *rot_link_button;
   GtkWidget *tilt_link_button;

   GtkWidget *view_type_combo;
   GtkWidget *view_type_combo_entry;

   /* scale, rot, trans and tilt spins */
   GtkWidget *scale_spinbutton;
   GtkWidget *trans_x_spinbutton;
   GtkWidget *trans_y_spinbutton;
   GtkWidget *trans_z_spinbutton;
   GtkWidget *rot_x_spinbutton;
   GtkWidget *rot_y_spinbutton;
   GtkWidget *rot_z_spinbutton;
   GtkWidget *rot_phi_spinbutton;
   GtkWidget *tilt_y_spinbutton;
   GtkWidget *tilt_x_spinbutton;
   GtkWidget *tilt_z_spinbutton;

   /* scale, rot, trans and tilt adjustments */
   GtkObject *scale_spinbutton_adj;
   GtkObject *trans_x_spinbutton_adj;
   GtkObject *trans_y_spinbutton_adj;
   GtkObject *trans_z_spinbutton_adj;
   GtkObject *rot_x_spinbutton_adj;
   GtkObject *rot_y_spinbutton_adj;
   GtkObject *rot_z_spinbutton_adj;
   GtkObject *rot_phi_spinbutton_adj;
   GtkObject *tilt_y_spinbutton_adj;
   GtkObject *tilt_x_spinbutton_adj;
   GtkObject *tilt_z_spinbutton_adj;

   /* lock togglebuttons */
   GtkWidget *scale_lock_button;
   GtkWidget *trans_lock_x_button;
   GtkWidget *trans_lock_y_button;
   GtkWidget *trans_lock_z_button;
   GtkWidget *rot_lock_x_button;
   GtkWidget *rot_lock_y_button;
   GtkWidget *rot_lock_z_button;
   GtkWidget *rot_lock_phi_button;
   GtkWidget *tilt_lock_x_button;
   GtkWidget *tilt_lock_y_button;
   GtkWidget *tilt_lock_z_button;

   /* roi */
   GtkWidget *roi_frame;
   GtkWidget *roi_none_radiobutton;
   GtkWidget *roi_cube_radiobutton;
   GtkWidget *roi_sphere_radiobutton;
   GtkObject *roi_fwhm_spinbutton_adj;
   GtkWidget *roi_fwhm_spinbutton;
   GtkWidget *roi_gauss_checkbutton;
   GtkObject *roi_x_spinbutton_adj;
   GtkWidget *roi_x_spinbutton;
   GtkObject *roi_y_spinbutton_adj;
   GtkWidget *roi_y_spinbutton;
   GtkObject *roi_z_spinbutton_adj;
   GtkWidget *roi_z_spinbutton;

   /* cmap */
   GtkWidget *cmap_frame;
   GtkWidget *cmap_min_spinbutton;
   GtkWidget *cmap_max_spinbutton;
   GtkWidget *cmap_max_hscale;
   GtkWidget *cmap_min_hscale;
   GtkWidget *cmap_grey_radiobutton;
   GtkWidget *cmap_hot_radiobutton;
   GtkWidget *cmap_spect_radiobutton;
   GtkWidget *cmap_bluered_radiobutton;
   GtkWidget *cmap_combo;
   GtkWidget *cmap_combo_entry;
   GtkObject *cmap_alpha_spinbutton_adj;
   GtkWidget *cmap_alpha_spinbutton;
   GtkWidget *cmap_alpha_hscale;

   /* vector */
   GtkWidget *vector_frame;
   GtkWidget *vector_alpha_mult_euc_checkbutton;
   GtkWidget *vector_mult_spinbutton;
   GtkWidget *vector_alpha_mult_spinbutton;
   GtkWidget *vector_points_checkbutton;
   GtkWidget *vector_rgb_togglebutton;
   GtkWidget *vector_colourpicker;
   GtkWidget *vector_point_colourpicker;
   GtkObject *vector_max_spinbutton_adj;
   GtkWidget *vector_max_spinbutton;
   GtkObject *vector_min_spinbutton_adj;
   GtkWidget *vector_min_spinbutton;
   GtkWidget *vector_min_hscale;
   GtkWidget *vector_max_hscale;

   /* merge */
   GtkWidget *merge_frame;
   GtkWidget *merge_combo[NUM_MERGE_COEFF];
   GtkWidget *merge_combo_entry[NUM_MERGE_COEFF];
   GtkObject *merge_coeff_adj[NUM_MERGE_COEFF];
   GtkWidget *merge_hscale[NUM_MERGE_COEFF];
   GtkWidget *merge_link_button[NUM_MERGE_COEFF];
   GtkWidget *merge_link_pixmap[NUM_MERGE_COEFF];

} Pane_dialog;

/* main_info structure */
typedef struct {

   /* pane info structs */
   Pane_info c_pane;
   GPtrArray *panes;
   GList   *pane_filelist;
   int      last_merge_id;

   /* init view types */
   int      init_transverse;
   int      init_sagittal;
   int      init_coronal;

   Synch_info *init_synch;
   int      n_synchs;
   Synch_info **synchs;

   /* Lookup Tables */
   int      n_tables;
   Lookup_Table **lookup_tables;

   /* pane info dialog */
   Pane_dialog *pane_dialog;

   /* widgets */
   GtkWidget *main_widget;
   GtkWidget *synch_dialog;
   GtkWidget *synch_table;
   GtkWidget *pane_info_dialog;

   GtkWidget *pane_hbody;
   GtkWidget *statusbar;
   GtkWidget *progressbar;

   /* gtkgl */
//   GtkWidget *glarea_share;
   GdkGLConfig *glconfig;

   /* tooltips */
   GtkTooltips *tooltips;

   /* styles */
   GtkStyle *pane_highlight_style;
   GtkStyle *view_highlight_style;
   GtkStyle *pane_normal_style;
   GtkStyle *view_normal_style;

   /* Glists */
   GList   *cmap_combo_items;

   /* timers */
   GTimer  *timer;
   double   draw_count;
} Main_info;

#endif
