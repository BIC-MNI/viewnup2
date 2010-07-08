/* viewnup.c                                                                 */
/*                                                                           */
/* The multiple minc file viewer                                             */
/*                                                                           */
/* Andrew Janke - a.janke@gmail.com                                          */
/*                                                                           */
/* Copyright Andrew Janke.                                                   */
/* Permission to use, copy, modify, and distribute this software and its     */
/* documentation for any purpose and without fee is hereby granted,          */
/* provided that the above copyright notice appear in all copies.  The       */
/* author makes no representations about the                                 */
/* suitability of this software for any purpose.  It is provided "as is"     */
/* without express or implied warranty.                                      */

#include "viewnup.h"

#include <math.h>
#include <popt.h>
#include "interface.h"
#include "callbacks.h"
#include "lookup_table.h"
#include "gtk_gl.h"
#include "trackball.h"
#include "minc_io.h"

/* internal function prototypes */
Main_info *init_main_info(int init_synch_idx, int transverse, int sagittal, int coronal);

/* toplevel struct */
Main_info main_info;

/* Argument variables */
int verbose = FALSE;
static int draw_fast = FALSE;
static int transverse = FALSE;
static int sagittal = FALSE;
static int coronal = FALSE;

static int perspective = FALSE;
static int synch_panes = FALSE;
static int bound_box = TRUE;
static int nobound_box = FALSE;
static int slice_box = FALSE;
static int vector = FALSE;

static int link_scales = FALSE;
static int link_trans = FALSE;
static int link_rots = FALSE;
static int link_tilts = FALSE;

/* popt (gtk) argument table */
struct poptOption options[] = {
   {"verbose", 'v', POPT_ARG_NONE, &verbose, 0,
    "Print out extra information", NULL},
   {"draw_fast", 'f', POPT_ARG_NONE, &draw_fast, 0,
    "Draw fast (dodgy hacks Inc.)", NULL},

   {"synch", 'X', POPT_ARG_NONE, &synch_panes, 0,
    "Synch Panes", NULL},
   {"vector", 'V', POPT_ARG_NONE, &vector, 0,
    "Show 4D files as vectors", NULL},
   {"perspective", 'P', POPT_ARG_NONE, &perspective, 0,
    "Use Perspective views", NULL},
   {"nobound_box", 'B', POPT_ARG_NONE, &nobound_box, 0,
    "Don't show bounding box", NULL},
   {"slice_box", 'S', POPT_ARG_NONE, &slice_box, 0,
    "Show slice box", NULL},

   {"transverse", 't', POPT_ARG_NONE, &transverse, 0,
    "Transverse view only", NULL},
   {"sagittal", 's', POPT_ARG_NONE, &sagittal, 0,
    "Sagittal view only", NULL},
   {"coronal", 'c', POPT_ARG_NONE, &coronal, 0,
    "Coronal view only", NULL},

   {"link_scale", '\0', POPT_ARG_NONE, &link_scales, 0,
    "Link scales between views",
    NULL},
   {"link_trans", '\0', POPT_ARG_NONE, &link_trans, 0,
    "Link translations between views",
    NULL},
   {"link_rots", '\0', POPT_ARG_NONE, &link_rots, 0,
    "Link rotations between views",
    NULL},
   {"link_tilts", '\0', POPT_ARG_NONE, &link_tilts, 0,
    "Link tilts between views", NULL},
   {NULL, '\0', 0, NULL, 0, NULL, NULL}
   };

/* toplevel struct ptr function */
Main_info *init_main_info(int init_synch_idx, int transverse, int sagittal, int coronal)
{
   int      c;
   Main_info *ptr = get_main_ptr();

   /* set up panes structure */
   ptr->panes = g_ptr_array_new();
   ptr->c_pane = NULL;
   ptr->pane_filelist = NULL;
   ptr->pane_filelist = g_list_append(ptr->pane_filelist, NO_FILE_STR);
   ptr->last_merge_id = 0;

   /* put in initial views */
   ptr->init_transverse = transverse;
   ptr->init_sagittal = sagittal;
   ptr->init_coronal = coronal;

   /* set up the synchs structure */
   if(verbose){
      g_print(_("Setting up the initial synchs...\n"));
      }
   ptr->synchs = (Synch_info **) g_malloc(sizeof(Synch_info *) * MAX_SYNCHS);
   ptr->n_synchs = 3;
   for(c = 0; c < ptr->n_synchs; c++){
      ptr->synchs[c] = init_synch_info();
      ptr->synchs[c]->idx = c;
      }
   ptr->init_synch = ptr->synchs[init_synch_idx];

   /* set up the lookup_tables */
   ptr->lookup_tables = (Lookup_Table **) g_malloc(sizeof(Lookup_Table *) * MAX_TABLES);
   ptr->n_tables = 0;

   /* pane info dialog */
   ptr->pane_dialog = (Pane_dialog *) g_malloc(sizeof(Pane_dialog));
   ptr->pane_dialog->signal_ids = g_array_new(FALSE, FALSE, sizeof(gulong));
   ptr->pane_dialog->obj_ptrs = g_ptr_array_new();

   /* initialise highlight and normal styles */
   ptr->drk_blue.pixel = 0;
   ptr->drk_blue.red = 0;
   ptr->drk_blue.green = 2560;
   ptr->drk_blue.blue = 20480;
   ptr->green.pixel = 0;
   ptr->green.red = 0;
   ptr->green.green = 65535;
   ptr->green.blue = 0;

   /* setup tooltips */
   ptr->tooltips = gtk_tooltips_new();

   /* initialise the lookup tables */
   if(verbose){
      g_print(_("Initialising Lookup tables...\n"));
      }
   init_lookup_tables(ptr);
   ptr->cmap_combo_items = get_cmaps_list(ptr);

   /* initialise the timer */
   ptr->timer = g_timer_new();
   return ptr;
   }

Main_info *get_main_ptr(void)
{
   return &main_info;
   }

/* synchs functions */
Synch_info *init_synch_info()
{
   Synch_info *synch = (Synch_info *) g_malloc(sizeof(Synch_info));

   synch->virgin = TRUE;
   synch->synch_x_adj = gtk_adjustment_new(0, -100, 100, 1, 10, 0);
   synch->synch_y_adj = gtk_adjustment_new(0, -100, 100, 1, 10, 0);
   synch->synch_z_adj = gtk_adjustment_new(0, -100, 100, 1, 10, 0);
   synch->synch_t_adj = gtk_adjustment_new(0, -100, 100, 1, 10, 0);

   g_object_ref(synch->synch_x_adj);
   g_object_ref(synch->synch_y_adj);
   g_object_ref(synch->synch_z_adj);
   g_object_ref(synch->synch_t_adj);
   return synch;
   }

int get_synch_idx(Main_info * ptr, Synch_info * synch)
{
   int      c;

   for(c = 0; c < ptr->n_synchs; c++){
      if(ptr->synchs[c] == synch){
         return c;
         }
      }

   return 0;
   }

Synch_info *get_next_synch(Main_info * ptr, Synch_info * synch)
{
   int      synch_idx;

   synch_idx = get_synch_idx(ptr, synch);

   if(synch_idx < (ptr->n_synchs - 1)){
      return ptr->synchs[synch_idx + 1];
      }
   else {
      return ptr->synchs[0];
      }
   }

int remove_synch(Main_info * ptr, Synch_info * synch)
{
   int      synch_idx;
   int      c;

   synch_idx = get_synch_idx(ptr, synch);

   ptr->n_synchs--;

   /* remove and clean up */
   g_object_unref(synch->synch_x_adj);
   g_object_unref(synch->synch_y_adj);
   g_object_unref(synch->synch_z_adj);
   g_object_unref(synch->synch_t_adj);
   g_free(synch);

   /* rearrange the array */
   for(c = synch_idx; c < ptr->n_synchs; c++){
      ptr->synchs[c] = ptr->synchs[c + 1];
      ptr->synchs[c]->idx--;
      }

   return TRUE;
   }

void free_view_info(View_info view)
{
   g_free(view);
   }

View_info init_view_info(int type)
{
   View_info view;

   view = g_malloc(sizeof *view);

   view->type = type;
   view->texmap_size[0] = 1;
   view->texmap_size[1] = 1;
   view->texmap_space = 0;

   view->reload_cmap = TRUE;
   view->reload_image = TRUE;
   view->reload_texmap = TRUE;
   view->refresh_view = TRUE;

   /* init array indexes */
   init_view_idx(view);

   view->GLscalefac = 1.0;

   view->GLtrans_x = 0.0;
   view->GLtrans_y = 0.0;
   view->GLtrans_z = 0.0;

   /* tilts for oblique views */
   view->tilt = FALSE;
   init_tilt_vec(view);

   /* initialise the rotation vector and phi */
   init_rot_vec(view);

   view->lock_scale = FALSE;
   view->lock_x_trans = FALSE;
   view->lock_y_trans = FALSE;
   view->lock_z_trans = FALSE;
   view->lock_x_rot = FALSE;
   view->lock_y_rot = FALSE;
   view->lock_z_rot = FALSE;
   view->lock_phi_rot = FALSE;
   view->lock_x_tilt = FALSE;
   view->lock_y_tilt = FALSE;
   view->lock_z_tilt = FALSE;

   /* init drawing states */
   view->translating = FALSE;
   view->rotating = FALSE;
   view->tilting = FALSE;
   return view;
   }

void free_pane_info(Pane_info pane)
{

   g_string_free(pane->file_name, TRUE);
   g_string_free(pane->file_basename, TRUE);

   /* free the view_info structs */
   g_ptr_array_free(pane->views, TRUE);

   /* kill the adjustments */
   g_object_unref(pane->range_min_adj);
   g_object_unref(pane->range_max_adj);

   g_object_unref(pane->coord_vx_adj);
   g_object_unref(pane->coord_vy_adj);
   g_object_unref(pane->coord_vz_adj);
   g_object_unref(pane->coord_vt_adj);

   g_object_unref(pane->coord_wx_adj);
   g_object_unref(pane->coord_wy_adj);
   g_object_unref(pane->coord_wz_adj);
   g_object_unref(pane->coord_wt_adj);

   g_free(pane);
   }

Pane_info init_pane_info(Synch_info * synch)
{
   int      c;
   Pane_info pane;

   pane = (Pane_info) g_malloc(sizeof(*pane));

   /* set the name and properties */
   pane->file_name = g_string_new(NULL);
   pane->file_basename = g_string_new(NULL);

   /* merge and synch */
   pane->merge = FALSE;
   pane->synch = synch;

   pane->position = 0;
   pane->draw_fast = draw_fast;
   pane->draw = FALSE;
   pane->linear_interp = TRUE;
   pane->vector = vector;
   pane->perspective = perspective;
   pane->bounding_box = bound_box;
   pane->slice_box = slice_box;
   pane->crosshair = TRUE;
   pane->crosshair_size = 100.0;

   pane->link_scales = link_scales;
   pane->link_trans = link_trans;
   pane->link_rots = link_rots;
   pane->link_tilts = link_tilts;

   pane->use_vox_coords = FALSE;
   pane->cmap_ptr = NULL;
   pane->alpha_thresh = 0.1;

   pane->views = g_ptr_array_new();
   pane->c_view = NULL;

   pane->n_dims = 0;
   /* set the default position and sizes */
   for(c = 0; c < MAX_VNUP_DIMS; c++){
      pane->sizes[c] = 1.0;
      pane->steps[c] = 1.0;
      pane->starts[c] = -0.5;
      pane->stops[c] = 0.5;

      pane->w[c] = 0.0;
      pane->v[c] = 0.0;
      pane->cov[c] = 0.0;
      }

   /* vector variables */
   pane->mult_vect = TRUE;
   pane->vect_points = TRUE;
   pane->RGB_vect = FALSE;
   pane->vect_col[0] = 0.0;
   pane->vect_col[1] = 0.5;
   pane->vect_col[2] = 1.0;
   pane->vect_col[3] = 1.0;

   pane->vect_point_col[0] = 0.0;
   pane->vect_point_col[1] = 1.8;
   pane->vect_point_col[2] = 0.9;
   pane->vect_point_col[3] = 0.8;

   pane->vect_mult = 1.0;
   pane->vect_alph_mult = 1.0;
   pane->vect_floor = -DBL_MAX;
   pane->vect_ceil = DBL_MAX;

   /* init volume structures and transform */
   pane->perc_input = 0.0;
   pane->volume = NULL;
   pane->transform = g_malloc(sizeof(General_transform));
   create_linear_transform(pane->transform, NULL);

   /* merge adjustments and pane ptrs */
   for(c = 0; c < NUM_MERGE_COEFF; c++){
      pane->merge_panes[c] = NULL;
      pane->merge_coeff[c] = 50.0;
      pane->merge_link[c] = FALSE;
      }

   return pane;
   }

void print_pane_info(Pane_info pane)
{
   int      c;

   View_info view;

   /* print the name and properties */
   g_print(_("\nPANE[%d] ---    %s\n"), pane->position, pane->file_basename->str);
   g_print(_("  file:       %s\n"), pane->file_name->str);

   g_print(_("  #    size  step  start  stop\n"));
   for(c = 0; c < pane->n_dims; c++){
      g_print(_("  [%d] %4d  %7.3f  %7.3f %7.3f\n"), c, pane->sizes[c],
              pane->steps[c], pane->starts[c], pane->stops[c]);
      }

   g_print(_("  Real range  %g-%g\n"), pane->real_min, pane->real_max);
   g_print(_("  Pane range  %g-%g\n"), pane->pane_min, pane->pane_max);
   g_print(_("  Pane rrange %g-%g\n"), pane->pane_min_value, pane->pane_max_value);
   g_print(_("  merge/synch %d/%d\n"), pane->merge, pane->synch->idx);
   g_print(_("  draw/fast   %d/%d\n"), pane->draw, pane->draw_fast);
   g_print(_("  interp      %d\n"), pane->linear_interp);
   g_print(_("  vect/perp   %d/%d\n"), pane->vector, pane->perspective);
   g_print(_("  bbox/sbox   %d/%d\n"), pane->bounding_box, pane->slice_box);
   g_print(_("  cross/size  %d/%g\n"), pane->crosshair, pane->crosshair_size);
   g_print(_("  use_vox     %d\n"), pane->use_vox_coords);
   g_print(_("  link(strt)  %d|%d|%d|%d\n"), pane->link_scales, pane->link_trans,
           pane->link_rots, pane->link_tilts);
   g_print(_("  v-xyzt      %5.2f|%5.2f|%5.2f|%5.2f\n"), pane->v[0], pane->v[1],
           pane->v[2], pane->v[3]);
   g_print(_("  w-xyzt      %5.2f|%5.2f|%5.2f|%5.2f\n"), pane->w[0], pane->w[1],
           pane->w[2], pane->w[3]);
   g_print(_("  cov-xyzt    %5.2f|%5.2f|%5.2f|%5.2f\n"), pane->cov[0], pane->cov[1],
           pane->cov[2], pane->cov[3]);
   g_print(_("  Cmap        %s\n"), pane->cmap_ptr->name);

   for(c = 0; c < pane->views->len; c++){
      view = g_ptr_array_index(pane->views, c);

      g_print(_("V[%d] - %d\n"), c, view->type);
      g_print(_("  +XYZ idx     %d|%d|%d\n"), view->x_idx, view->y_idx, view->z_idx);
      g_print(_("  +TEX[%d]\n"), view->texmap_id);
      g_print(_("  |    +size   %d|%d - %d\n"), view->texmap_size[0],
              view->texmap_size[1], view->texmap_space);
      g_print(_("  |    +stop   %g|%g\n"), view->texmap_stop[0], view->texmap_stop[1]);
      g_print(_("  |    -+quad coords\n"));
      g_print(_("  |     | q0  [%g:%g:%g]\n"), view->q0[0], view->q0[1], view->q0[2]);
      g_print(_("  |     | q1  [%g:%g:%g]\n"), view->q1[0], view->q1[1], view->q1[2]);
      g_print(_("  |     | q2  [%g:%g:%g]\n"), view->q2[0], view->q2[1], view->q2[2]);
      g_print(_("  |     | q3  [%g:%g:%g]\n"), view->q3[0], view->q3[1], view->q3[2]);
      g_print(_("  +pix_size    %g|%g\n"), view->pix_size[0], view->pix_size[1]);
      g_print(_("  +view_start  %g|%g\n"), view->view_start[0], view->view_start[1]);
      g_print(_("  +view_stop   %g|%g\n"), view->view_stop[0], view->view_stop[1]);
      g_print(_("  +scale       %g\n"), view->GLscalefac);
      g_print(_("  +trans       %g|%g|%g\n"), view->GLtrans_x, view->GLtrans_y,
              view->GLtrans_z);
      g_print(_("  +rot_q       %g|%g|%g|%g\n"), view->rot_quat[0], view->rot_quat[1],
              view->rot_quat[2], view->rot_quat[3]);
      g_print(_("  |rot_v       %g|%g|%g phi %g\n"), view->rot_vec[0], view->rot_vec[1],
              view->rot_vec[2], view->rot_phi);
      g_print(_("  +tlt_q       %g|%g|%g|%g\n"), view->tilt_quat[0], view->tilt_quat[1],
              view->tilt_quat[2], view->tilt_quat[3]);
      g_print(_("  |tlt_vw      %g|%g|%g\n"), view->tilt_w[0], view->tilt_w[1],
              view->tilt_w[2]);
      g_print(_("  |tlt_vv      %g|%g|%g\n"), view->tilt_v[0], view->tilt_v[1],
              view->tilt_v[2]);
      }

   /* print out merge information */
   if(pane->merge){
      g_print(_("Merge info\n"));
      for(c = 0; c < NUM_MERGE_COEFF; c++){
         g_print(_("  | %5.2f [%d] - %s\n"), pane->merge_coeff[c],
                 pane->merge_link[c], pane->merge_panes[c]->file_basename->str);
         }
      }
   }

/* build the current list of pane names */
void build_pane_filelist(Main_info * ptr)
{
   Pane_info pane_info;
   int      c;

   g_list_free(ptr->pane_filelist);
   ptr->pane_filelist = NULL;
   ptr->pane_filelist = g_list_append(ptr->pane_filelist, NO_FILE_STR);

   for(c = 0; c < ptr->panes->len; c++){
      pane_info = g_ptr_array_index(ptr->panes, c);

      if(!pane_info->merge){
         ptr->pane_filelist = g_list_append(ptr->pane_filelist,
                                            pane_info->file_basename->str);
         }
      }
   }

void init_rot_vec(View_info view)
{
   switch (view->type){
   default:
   case TRANSVERSE:
      view->rot_vec[0] = 0.0;
      view->rot_vec[1] = 0.0;
      view->rot_vec[2] = 1.0;
      view->rot_phi = 0.0;
      axis_to_quat(view->rot_vec, view->rot_phi, view->rot_quat);
      break;

   case SAGITTAL:
      view->rot_quat[0] = -0.5;
      view->rot_quat[1] = -0.5;
      view->rot_quat[2] = -0.5;
      view->rot_quat[3] = -0.5;
      quat_to_axis(view->rot_vec, &(view->rot_phi), view->rot_quat);
      break;

   case CORONAL:
      view->rot_vec[0] = 1.0;
      view->rot_vec[1] = 0.0;
      view->rot_vec[2] = 0.0;
      view->rot_phi = M_PI_2;
      axis_to_quat(view->rot_vec, view->rot_phi, view->rot_quat);
      break;
      }
   }

/* initialise the tilt vector to sensical start values */
void init_tilt_vec(View_info view)
{
   double   z_up[3] = { 0, 0, 1 };

   switch (view->type){
   default:
   case TRANSVERSE:
      view->tilt_w[0] = 0.0;
      view->tilt_w[1] = 0.0;
      view->tilt_w[2] = 1.0;
      break;

   case SAGITTAL:
      view->tilt_w[0] = 1.0;
      view->tilt_w[1] = 0.0;
      view->tilt_w[2] = 0.0;
      break;

   case CORONAL:
      view->tilt_w[0] = 0.0;
      view->tilt_w[1] = -1.0;
      view->tilt_w[2] = 0.0;
      break;
      }

   vects_to_quat(z_up, view->tilt_w, view->tilt_quat);
   }

/* set up view array indexes */
void init_view_idx(View_info view)
{
   switch (view->type){
   default:
   case TRANSVERSE:
      view->x_idx = 0;
      view->y_idx = 1;
      view->z_idx = 2;
      break;
   case SAGITTAL:
      view->x_idx = 1;
      view->y_idx = 2;
      view->z_idx = 0;
      break;
   case CORONAL:
      view->x_idx = 0;
      view->y_idx = 2;
      view->z_idx = 1;
      break;
      }
   }

int main(int argc, char *argv[])
{
   Main_info *ptr;
   Pane_info pane;
   char   **infiles;
   int      n_infiles;
   int      add_merge_pane = TRUE;
   int      c;
   int      init_synch_idx;
   poptContext pctx;
   
   // temporary measure
   verbose = TRUE;
   
#ifdef ENABLE_NLS
   bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
   textdomain(PACKAGE);
#endif

   /* init and parse args for gtk and gtkgl */
   gtk_init(&argc, &argv);
   gtk_gl_init(&argc, &argv);

   if(verbose){
      g_print(_("Finished gtk & gtkgl init\n"));
      }
   
   /* parse viewnup's remaining arguments */
//   optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);

   /* assume remaining args are input files */
   n_infiles = argc - 1;
   if(n_infiles > 128){
      g_error(_("Too many input files (max 128)\n"));
      }
   infiles = &argv[1];

   /* set up default initial views */
   if(!transverse && !sagittal && !coronal){
      transverse = TRUE;
      sagittal = TRUE;
      coronal = TRUE;
      }

   if(synch_panes){
      init_synch_idx = 1;
      }
   else {
      init_synch_idx = 0;
      }

   /* a bit of info */
   if(verbose){
      gint     major, minor;

      // stuff about synchs

      // stuff about input filenames etc

      // stuff about merge panes

      gdk_gl_query_version(&major, &minor);
      g_print(_("OpenGL extension version - %d.%d\n"), major, minor);
      }

   /* init main info structure */
   ptr = init_main_info(init_synch_idx, transverse, sagittal, coronal);
   if(verbose){
      g_print(_("Initialised main info...\n"));
      }
   
   /* create the main window and dialogs */
   ptr->main_widget = create_viewnup_main(ptr);
   ptr->synch_dialog = create_synch_dialog(ptr);
   ptr->pane_info_dialog = create_pane_info_dialog(ptr);
   gtk_widget_show(ptr->main_widget);
   if(verbose){
      g_print(_("Created and shown main widgets...\n"));
      }
   
   /* initialize and configure Main gtkgl widget and context */
   configure_gtkgl(ptr);
   if(verbose){
      g_print(_("Configured gtkgl...\n"));
      }

   /* setup the initial panes */
   if(verbose){
      g_print(_("Opening infiles...\n"));
      }
   if(n_infiles == 0){
      add_pane(ptr, FALSE, NULL, FALSE);
      add_pane(ptr, FALSE, NULL, FALSE);
      add_merge_pane = TRUE;
      }
   else {
      for(c = 0; c < n_infiles; c++){
         if(verbose){
            g_print(_("[%d] - %s\n"), c, infiles[c]);
            }
         
         /* add the pane */
         pane = add_pane(ptr, FALSE, NULL, FALSE);
         
         /* trigger the file-open callback */
         gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(pane->file_open_combo)->entry), infiles[c]);
         gtk_widget_activate(GTK_WIDGET(GTK_COMBO(pane->file_open_combo)->entry));
         }

      add_merge_pane = (n_infiles == 1) ? FALSE : TRUE;
      }

   /* Add the merge pane if wanted */
   if(add_merge_pane){
      pane = add_pane(ptr, FALSE, NULL, TRUE);
      }

   /* make the first pane active */
   make_pane_active(g_ptr_array_index(ptr->panes, 0));

   g_print(_("Entering gtk_main() loop\n"));
   gtk_main();
   return TRUE;
   }
