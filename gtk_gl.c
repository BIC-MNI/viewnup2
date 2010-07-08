/* gtk_gl.c */

#define CHECK_OPENGL_ERROR(){\
   GLenum  error; \
   while((error = glGetError()) != GL_NO_ERROR){ \
      printf("[%s:%d] failed with error [%d] %s\n", \
         __FILE__, __LINE__, error, gluErrorString(error)); \
         } \
                     }

#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#include "gtk_gl.h"
#include "minc_io.h"
#include "callbacks.h"
#include "lookup_table.h"
#include "trackball.h"
#include "viewnup.h"
#include "geometry.h"

extern int verbose;

/* gtkgl callbacks structure */
typedef struct {
   Main_info *ptr;
   Pane_info pane;
   View_info view;
   } GtkGL_info;

/* internal function prototypes */
void     update_gtkgl_coords(Pane_info pane, View_info view, GLdouble x, GLdouble y);
void     translate_gtkgl(Pane_info pane, View_info view,
                         double beginx, double beginy, double x, double y);
void     load_view_texmap(Pane_info pane, View_info view);
int      draw_texture_slice(Pane_info pane, View_info v);
void     draw_vector_slice(Pane_info pane, View_info view);

gint     gtkgl_button_press(GtkWidget * widget, GdkEventButton * event,
                            gpointer func_data);
gint     gtkgl_motion_notify(GtkWidget * widget, GdkEventMotion * event,
                             gpointer func_data);
gint     gtkgl_button_release(GtkWidget * widget, GdkEventButton * event,
                              gpointer func_data);

gint     gtkgl_draw(GtkWidget * widget, GdkEventExpose * event, gpointer func_data);
gint     gtkgl_reshape(GtkWidget * widget, GdkEventConfigure * event, gpointer func_data);
gint     gtkgl_init(GtkWidget * widget, gpointer func_data);
gint     gtkgl_destroy(GtkWidget * widget, gpointer func_data);

void     draw_crosshair(Pane_info pane, View_info view);
void     draw_bounding_box(Pane_info pane, View_info view);
void     draw_slice_box(Pane_info pane, View_info view);
void     draw_bbox_line(double *obl_norm, double *obl_point,
                        double *box_norm, double *box_point,
                        double *min_vec, double *max_vec);

/* Configure the OpenGL framebuffer */
void configure_gtkgl(Main_info * ptr)
{
   /* set up the GdkGLConfig */
   ptr->glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB |
                                             GDK_GL_MODE_ALPHA |
                                             GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);
   if(ptr->glconfig == NULL){
      g_print("*** Cannot find an approriate double-buffered visual.\n");
      exit(EXIT_FAILURE);
      }
   
   ptr->gtkgl_share = gtk_drawing_area_new();
   gtk_widget_set_gl_capability(ptr->gtkgl_share,
                                ptr->glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
   gtk_box_pack_start(GTK_BOX(ptr->main_vbox), ptr->gtkgl_share, FALSE, FALSE, 0);
   gtk_widget_realize(ptr->gtkgl_share);

   /* set up the shared context object */
   ptr->glcontext = gtk_widget_get_gl_context(ptr->gtkgl_share);
   }

GtkWidget *create_gtkgl_widget(Main_info * ptr, Pane_info pane, View_info view)
{
   GtkWidget *gtkgl_widget;
   GtkGL_info *gtkgl_info;

   gtkgl_info = (GtkGL_info *) g_malloc(sizeof(GtkGL_info));
   gtkgl_info->ptr = ptr;
   gtkgl_info->pane = pane;
   gtkgl_info->view = view;

   /* create the widget */
   gtkgl_widget = gtk_drawing_area_new();
   gtk_widget_set_gl_capability(gtkgl_widget, ptr->glconfig, ptr->glcontext,
                                TRUE, GDK_GL_RGBA_TYPE);

   /* Check out gdk/gdktypes.h for a list of event masks. */
   gtk_widget_set_events(GTK_WIDGET(gtkgl_widget), GDK_EXPOSURE_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

   /* add callbacks */
   g_signal_connect(GTK_OBJECT(gtkgl_widget), "realize",
                    G_CALLBACK(gtkgl_init), gtkgl_info);
   g_signal_connect(GTK_OBJECT(gtkgl_widget), "configure_event",
                    G_CALLBACK(gtkgl_reshape), gtkgl_info);
   g_signal_connect(GTK_OBJECT(gtkgl_widget), "expose_event",
                    G_CALLBACK(gtkgl_draw), gtkgl_info);
   g_signal_connect(GTK_OBJECT(gtkgl_widget), "destroy",
                    G_CALLBACK(gtkgl_destroy), gtkgl_info);

   g_signal_connect(GTK_OBJECT(gtkgl_widget), "button_press_event",
                    G_CALLBACK(gtkgl_button_press), gtkgl_info);
   g_signal_connect(GTK_OBJECT(gtkgl_widget), "motion_notify_event",
                    G_CALLBACK(gtkgl_motion_notify), gtkgl_info);
   g_signal_connect(GTK_OBJECT(gtkgl_widget), "button_release_event",
                    G_CALLBACK(gtkgl_button_release), gtkgl_info);
   
   return (gtkgl_widget);
   }

/* convenience fnuction to redraw all views in a pane */
void redraw_pane_views(Pane_info pane)
{
   int      c;

   for(c = 0; c < pane->views->len; c++){
      redraw_pane_view(g_ptr_array_index(pane->views, c));
      }
   }

void redraw_pane_view(View_info view)
{
   gtk_widget_queue_draw(GTK_WIDGET(view->gtkgl_widget));
   }

/* convenience fnuction to resize all views in a pane */
void resize_pane_views(Pane_info pane)
{
   int      c;

   for(c = 0; c < pane->views->len; c++){
      resize_pane_view(g_ptr_array_index(pane->views, c));
      }
   }

void resize_pane_view(View_info view)
{
   gtk_widget_queue_resize(GTK_WIDGET(view->gtkgl_widget));
   }

/* checks if a slice needs re-orienting and calls the re-draw */
void update_slice_tilt(Pane_info pane, View_info view)
{
   int      view_type;

   view->tilt = TRUE;
   pane->slice_box = TRUE;

   /* update the voxel tilts */
   quat_to_zvec(view->tilt_w, view->tilt_quat);
   update_voxel_vector_from_world(pane, view);

   /* find the new view type (defaults to transverse) */
   if(fabs(view->tilt_v[0]) >= fabs(view->tilt_v[1])){
      if(fabs(view->tilt_v[0]) >= fabs(view->tilt_v[2])){
         view_type = SAGITTAL;
         }
      else {
         view_type = TRANSVERSE;
         }
      }
   else {
      if(fabs(view->tilt_v[1]) >= fabs(view->tilt_v[2])){
         view_type = CORONAL;
         }
      else {
         view_type = TRANSVERSE;
         }
      }

   if(view->type != view_type){
      view->type = view_type;

      /* reset the view index */
      init_view_idx(view);

      /* force a refresh of the texture map and matricies */
      pane->c_view->reload_texmap = TRUE;
      pane->c_view->reload_cmap = TRUE;
      pane->c_view->refresh_view = TRUE;
      }

   view->reload_image = TRUE;
   redraw_pane_view(view);
   }

/* calculate the extents of a merge volume */
void calc_merge_extents(Pane_info merge)
{
   Pane_info pane;
   int      c, i;

   /* check just in case */
   if(merge->merge){

      /* set the number of dimensions */
      merge->n_dims = MAX_VNUP_DIMS;

      /* init the start and stops */
      for(c = 0; c < MAX_VNUP_DIMS; c++){
         merge->starts[c] = -0.5;
         merge->stops[c] = 0.5;
         }

      /* step through each of the merged panes */
      for(c = 0; c < NUM_MERGE_COEFF; c++){
         pane = merge->merge_panes[c];
         if(pane != NULL){

            /* check the start and stop */
            for(i = 0; i < MAX_VNUP_DIMS; i++){
               if(pane->starts[i] < merge->starts[i]){
                  merge->starts[i] = pane->starts[i];
                  }
               if(pane->stops[i] > merge->stops[i]){
                  merge->stops[i] = pane->stops[i];
                  }
               }
            }
         }
      }

   else {
      g_print("THIS WAS NOT A MERGE PANE! %s\n", merge->file_basename->str);
      }

   }

/* find the current point on the plane via ray casting */
void update_gtkgl_coords(Pane_info pane, View_info view, GLdouble x, GLdouble y)
{
   double   n_point[3], f_point[3];

   if(gluUnProject(x, y, 0.0,
                   view->modelMatrix, view->projMatrix, view->viewport,
                   &n_point[0], &n_point[1], &n_point[2]) &&
      gluUnProject(x, y, 1.0,
                   view->modelMatrix, view->projMatrix, view->viewport,
                   &f_point[0], &f_point[1], &f_point[2])){

      /* get the point on the current oblique plane */
      intersect_plane_line_p(pane->w, n_point, f_point, view->tilt_w, pane->w);

      update_voxel_from_world(pane);
      update_coord_values(pane);
      }
   else {
      g_warning(_("Failed to get world coords on click!\n"));
      }
   }

void translate_gtkgl(Pane_info pane, View_info view,
                     double beginx, double beginy, double x, double y)
{
   GLdouble begin_x, begin_y, begin_z;
   GLdouble after_x, after_y, after_z;

   if(gluUnProject(beginx, beginy, 1.0,
                   view->modelMatrix, view->projMatrix, view->viewport,
                   &begin_x, &begin_y, &begin_z) &&
      gluUnProject(x, y, 1.0,
                   view->modelMatrix, view->projMatrix, view->viewport,
                   &after_x, &after_y, &after_z)){

      if(!view->lock_x_trans){
         view->GLtrans_x += after_x - begin_x;
         }
      if(!view->lock_y_trans){
         view->GLtrans_y += after_y - begin_y;
         }
      if(!view->lock_z_trans){
         view->GLtrans_z += after_z - begin_z;
         }
      }
   else {
      g_warning(_("Failed to get world coords for translate!\n"));
      }
   }

/* This function handles button-press events for the GtkGL widget */
gint gtkgl_button_press(GtkWidget * widget, GdkEventButton * event, gpointer func_data)
{
   Main_info *ptr = get_main_ptr();
   Pane_info pane = ((GtkGL_info *) func_data)->pane;
   View_info view = ((GtkGL_info *) func_data)->view;

   /* if we don't have to draw this pane, return early */
   if(!pane->draw){
      return (TRUE);
      }

   if(gdk_pointer_grab(GTK_WIDGET(widget)->window, FALSE,
                       GDK_POINTER_MOTION_HINT_MASK |
                       GDK_BUTTON1_MOTION_MASK |
                       GDK_BUTTON2_MOTION_MASK |
                       GDK_BUTTON3_MOTION_MASK |
                       GDK_BUTTON_RELEASE_MASK, NULL, NULL, event->time) == 0){
      }

   view->pix_start_pos[0] = event->x;
   view->pix_start_pos[1] = view->pix_size[1] - event->y;

   switch (event->button){
   default:
   case 1:
      if(event->state & GDK_SHIFT_MASK){
         view->translating = TRUE;
         }
      else {
         update_gtkgl_coords(pane, view, view->pix_start_pos[0], view->pix_start_pos[1]);
         }
      break;

   case 2:
      if(event->state & GDK_SHIFT_MASK){
         ;
         }
      else {
         ;
         }
      break;

   case 3:
      if(event->state & GDK_SHIFT_MASK){
         view->rotating = TRUE;
         redraw_pane_view(view);
         }
      else {
         view->tilting = TRUE;
         redraw_pane_view(view);
         }
      break;

      }

   /* start the timer and reset the counter */
   g_timer_start(ptr->timer);
   ptr->draw_count = 0.0;

   /* make the approriate pane and view active */
   if(pane != ptr->c_pane){
      make_pane_active(pane);
      }

   if(view != pane->c_view){
      make_view_active(pane, view);
      }

   return TRUE;
   }

/* This function handles button-release events for the GtkGL widget */
gint gtkgl_button_release(GtkWidget * widget, GdkEventButton * event, gpointer func_data)
{
   Main_info *ptr = get_main_ptr();
   View_info view = ((GtkGL_info *) func_data)->view;
   gdouble  elapsed_time;
   gchar    buf[128];

   view->translating = FALSE;
   view->rotating = FALSE;
   view->tilting = FALSE;
   redraw_pane_view(view);

   gdk_pointer_ungrab(event->time);

   /* reset the timer */
   g_timer_stop(ptr->timer);
   elapsed_time = g_timer_elapsed(ptr->timer, NULL);

   g_snprintf(buf, 128, _("%g frames in %gsec -- %gf/sec"),
              ptr->draw_count, elapsed_time, ptr->draw_count / elapsed_time);

   push_statusbar(ptr, buf);
   g_timer_reset(ptr->timer);
   return TRUE;
   }

/* This function handles motion events for the GtkGL widget */
gint gtkgl_motion_notify(GtkWidget * widget, GdkEventMotion * event, gpointer func_data)
{
   Main_info *ptr = get_main_ptr();
   Pane_info pane = ((GtkGL_info *) func_data)->pane;
   View_info view = ((GtkGL_info *) func_data)->view;

   double   delta_x, delta_y, tmp;
   View_info v_ptr;
   double   tempquat[4];
   GdkModifierType state;
   int      x, y, c;

   /* if we don't have to draw this pane, return early */
   if(!pane->draw){
      return TRUE;
      }

   if(event->is_hint){
      gdk_window_get_pointer(event->window, &x, &y, &state);
      }
   else {
      x = event->x;
      y = event->y;
      state = event->state;
      }

   /* make approriate pane and view active in case GTK hasn't caught up yet */
   if(state & GDK_SHIFT_MASK){
      if(pane != ptr->c_pane){
         make_pane_active(pane);
         }

      if(view != pane->c_view){
         make_view_active(pane, view);
         }
      }

   /* Compensate for OpenGL vs Gtk's impression of where y0 is */
   y = view->pix_size[1] - y;

   delta_x = x - view->pix_start_pos[0];
   delta_y = y - view->pix_start_pos[1];

   if(state & GDK_BUTTON1_MASK){
      if(state & GDK_SHIFT_MASK){
         translate_gtkgl(pane, view, view->pix_start_pos[0],
                         view->pix_start_pos[1], view->pix_start_pos[0] + delta_x,
                         view->pix_start_pos[1] + delta_y);
         update_pi_trans(ptr, view->GLtrans_x, view->GLtrans_y, view->GLtrans_z);

         if(pane->link_trans){
            for(c = 0; c < pane->views->len; c++){
               v_ptr = g_ptr_array_index(pane->views, c);
               if(v_ptr != view){
                  translate_gtkgl(pane, v_ptr,
                                  v_ptr->pix_start_pos[0], v_ptr->pix_start_pos[1],
                                  v_ptr->pix_start_pos[0] + delta_x,
                                  v_ptr->pix_start_pos[1] + delta_y);
                  v_ptr->refresh_view = TRUE;
                  redraw_pane_view(v_ptr);
                  }
               }
            }
         }
      else {
         update_gtkgl_coords(pane, view, view->pix_start_pos[0] + delta_x,
                             view->pix_start_pos[1] + delta_y);
         }
      }

   else if(state & GDK_BUTTON2_MASK){
      if(state & GDK_SHIFT_MASK){
         tmp = 1.0 + (delta_y / view->pix_size[1]);

         if(!view->lock_scale){
            update_pi_scale(ptr, view->GLscalefac * tmp);
            }

         if(pane->link_scales){
            for(c = 0; c < pane->views->len; c++){
               v_ptr = g_ptr_array_index(pane->views, c);
               if(v_ptr != view && !v_ptr->lock_scale){
                  v_ptr->GLscalefac *= tmp;
                  v_ptr->refresh_view = TRUE;
                  redraw_pane_view(v_ptr);
                  }
               }
            }
         }

      else {
         /* yes 100 is a hack value, it just sems to work nice */
         tmp = delta_y * 100 / view->pix_size[1];

         switch (view->type){
         default:
         case TRANSVERSE:
            pane->w[2] += tmp;
            break;
         case SAGITTAL:
            pane->w[0] += tmp;
            break;
         case CORONAL:
            pane->w[1] += tmp;
            break;
            }

         update_voxel_from_world(pane);
         update_coord_values(pane);
         }
      }

   else if(state & GDK_BUTTON3_MASK){
      if(state & GDK_SHIFT_MASK){
         /* rotate the eye position */
         trackball(tempquat,
                   (view->pix_start_pos[0] -
                    (view->pix_size[0] / 2)) / view->pix_size[0],
                   (view->pix_start_pos[1] -
                    (view->pix_size[1] / 2)) / view->pix_size[1],
                   (view->pix_start_pos[0] + delta_x -
                    (view->pix_size[0] / 2)) / view->pix_size[0],
                   (view->pix_start_pos[1] + delta_y -
                    (view->pix_size[1] / 2)) / view->pix_size[1]
            );

         add_quats(tempquat, view->rot_quat, view->rot_quat);
         quat_to_axis(view->rot_vec, &(view->rot_phi), view->rot_quat);
         update_pi_rots(ptr, view->rot_vec[0],
                        view->rot_vec[1], view->rot_vec[2], view->rot_phi);

         if(pane->link_rots){
            for(c = 0; c < pane->views->len; c++){
               v_ptr = g_ptr_array_index(pane->views, c);
               if(v_ptr != view){
                  add_quats(tempquat, v_ptr->rot_quat, v_ptr->rot_quat);
                  quat_to_axis(v_ptr->rot_vec, &(v_ptr->rot_phi), v_ptr->rot_quat);
                  v_ptr->refresh_view = TRUE;
                  redraw_pane_view(v_ptr);
                  }
               }
            }
         }
      else{
         /* change the slice tilt */
         trackball(tempquat,
                   (view->pix_start_pos[0] -
                    (view->pix_size[0] / 2)) / view->pix_size[0],
                   (view->pix_start_pos[1] -
                    (view->pix_size[1] / 2)) / view->pix_size[1],
                   (view->pix_start_pos[0] + delta_x -
                    (view->pix_size[0] / 2)) / view->pix_size[0],
                   (view->pix_start_pos[1] + delta_y -
                    (view->pix_size[1] / 2)) / view->pix_size[1]
            );

         add_quats(tempquat, view->tilt_quat, view->tilt_quat);
         update_slice_tilt(pane, view);
         update_pi_tilts(ptr, view->tilt_w[0], view->tilt_w[1], view->tilt_w[2]);

         if(pane->link_tilts){
            for(c = 0; c < pane->views->len; c++){
               v_ptr = g_ptr_array_index(pane->views, c);
               if(v_ptr != view){
                  add_quats(tempquat, v_ptr->tilt_quat, v_ptr->tilt_quat);
                  quat_to_zvec(v_ptr->tilt_w, v_ptr->tilt_quat);
                  update_slice_tilt(pane, v_ptr);
                  }
               }
            }

         }
      }

   view->pix_start_pos[0] += delta_x;
   view->pix_start_pos[1] += delta_y;
   return TRUE;
   }

/* loads a texmap in the OpenGL space -- ONLY CALL IF YOU KNOW WHAT YOU ARE DOING */
void load_view_texmap(Pane_info pane, View_info view)
{
   int      size0, size1;
   GLint    width_check;

   /* setup texture map */
   switch (view->type){
   default:
   case TRANSVERSE:
      size0 = pane->sizes[1];
      size1 = pane->sizes[0];
      break;
   case SAGITTAL:
      size0 = pane->sizes[2];
      size1 = pane->sizes[1];
      break;
   case CORONAL:
      size0 = pane->sizes[2];
      size1 = pane->sizes[0];
      break;
      }

   if(view->texmap_space != 0){
      g_free(view->texmap);

      view->texmap_space = 0;
      view->texmap_size[0] = 1;
      view->texmap_size[1] = 1;
      }

   while(view->texmap_size[0] < size0){
      view->texmap_size[0] *= 2;
      }
   while(view->texmap_size[1] < size1){
      view->texmap_size[1] *= 2;
      }

   // dodgy hack to make it work
   if(view->texmap_size[0] > view->texmap_size[1]){
      view->texmap_size[1] = view->texmap_size[0];
      }
   else {
      view->texmap_size[0] = view->texmap_size[1];
      }

   view->texmap_space = view->texmap_size[0] * view->texmap_size[1];
   if(verbose){
      g_print(_("g_malloc0ing texmap space - %d\n"), view->texmap_space);
      }
   view->texmap = (unsigned char *)g_malloc0(view->texmap_space);

   view->texmap_stop[0] = (double)size0 / view->texmap_size[0];
   view->texmap_stop[1] = (double)size1 / view->texmap_size[1];

   /* first remove the old one */
   if(glIsTexture(view->texmap_id)){
      glDeleteTextures(1, &(view->texmap_id));
      }

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glGenTextures(1, &(view->texmap_id));
   glBindTexture(GL_TEXTURE_2D, view->texmap_id);
   if(verbose){
      g_print(_("Loading texmap #%d to %s[%d]\n"),
              view->texmap_id, pane->file_basename->str, view->type);
      }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   glPixelTransferi(GL_MAP_COLOR, TRUE);

   /* check if we can make this texture */
   glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA,
                view->texmap_size[0], view->texmap_size[1],
                0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, NULL);
   glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width_check);
   if(width_check == 0){
      /* barf */
      g_error(_("Canna map that texture Capt'n! - %d-%s\n"), view->type,
              pane->file_basename->str);
      }
   
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                view->texmap_size[0], view->texmap_size[1],
                0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, NULL);

   view->reload_texmap = FALSE;
   }

/* This is the function that should render your scene to the GtkGL widget. */
/* It can be used as a callback to the 'Expose' event.                     */
gint gtkgl_draw(GtkWidget * widget, GdkEventExpose * event, gpointer func_data)
{
   Main_info *ptr = get_main_ptr();
   GLdouble tmp_m[4][4];
   int      c, i;

   GdkGLDrawable *gldrawable;
   GdkGLContext *glcontext;

   /* Get pointer to info structs */
   Pane_info pane = ((GtkGL_info *) func_data)->pane;
   View_info view = ((GtkGL_info *) func_data)->view;
   
//   g_print("GLdraw RT|RI|RC|RV  %d|%d|%d|%d  EC|DRAW  %d|%d\n", view->reload_texmap, view->reload_image, view->reload_cmap, view->refresh_view, event->count, pane->draw);
   
   /* Draw only on the last expose event. */
   if(event->count > 0){
      return TRUE;
      }

   /* if we don't have to draw this pane, blank it and return early */
   if(!pane->draw){
      gldrawable = gtk_widget_get_gl_drawable(widget);
      glcontext = gtk_widget_get_gl_context(widget);
      
      if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)){
         return FALSE;
         }
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
      gdk_gl_drawable_gl_end(gldrawable);
      gdk_gl_drawable_swap_buffers(gldrawable);
      
      return TRUE;
      }

   /* get the current drawable and context and make it current */
   gldrawable = gtk_widget_get_gl_drawable(widget);
   glcontext = gtk_widget_get_gl_context(widget);
   
   /* glBegin */
   if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)){
      return FALSE;
      }
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* refresh the view if needed */
   if(view->refresh_view){

      /* refresh the modelling matrix */
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      
      /* rotate */
      glRotated(-view->rot_phi * 180 / M_PI, view->rot_vec[0], view->rot_vec[1],
                view->rot_vec[2]);

      /* scale about the current translated world position */
      glTranslated(pane->w[0] + view->GLtrans_x, 
                   pane->w[1] + view->GLtrans_y, 
                   pane->w[2] + view->GLtrans_z);
      glScaled(view->GLscalefac, view->GLscalefac, view->GLscalefac);
      glTranslated(-(pane->w[0] + view->GLtrans_x), 
                   -(pane->w[1] + view->GLtrans_y), 
                   -(pane->w[2] + view->GLtrans_z));
      
      /* translate to current co-ordinate */
      glTranslated(view->GLtrans_x, view->GLtrans_y, view->GLtrans_z);
      
      /* store the model matrix for later reference */
      glGetDoublev(GL_MODELVIEW_MATRIX, view->modelMatrix);
      view->refresh_view = FALSE;
      }

   /* draw the data */
   if(pane->merge){

      glPushMatrix();

      for(c = 0; c < NUM_MERGE_COEFF; c++){
         if(pane->merge_panes[c] != NULL){

            glTranslated(0.0, 0.0, 1.0);
            //for(i=0; i<20; i++){
            //   fac = ((double)i)/20;
            //   glAlphaFunc(GL_GREATER, 1.0-fac);
            //   glTranslated(0.0, 0.0, -fac);

            //   }
            //glPopMatrix();

            if(pane->merge_panes[c]->vector && (pane->merge_panes[c]->n_dims != 3)){
               draw_vector_slice(pane->merge_panes[c], view);
               }
            else {

               /* force a reload of cmaps per view */
               for(i = 0; i < pane->views->len; i++){
                  ((View_info) g_ptr_array_index(pane->views, i))->reload_cmap = TRUE;
                  }

               if(c == 0){
                  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                  }
               else {
                  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                  }

               glEnable(GL_ALPHA_TEST);
               glAlphaFunc(GL_GREATER, pane->merge_coeff[c] / 100);

               if(!draw_texture_slice(pane->merge_panes[c], view)){
                  // write error message to statusbar
                  return FALSE;
                  }

               glDisable(GL_ALPHA_TEST);
               }
            }
         }

      glPopMatrix();
      }

   else {
      if(pane->vector && (pane->n_dims != 3)){
         draw_vector_slice(pane, view);
         }
      else {
         glEnable(GL_ALPHA_TEST);
         glAlphaFunc(GL_GREATER, pane->alpha_thresh);

         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
         if(!draw_texture_slice(pane, view)){
            // write error message to statusbar
            return FALSE;
            }

         glDisable(GL_ALPHA_TEST);
         }
      }

   /* draw the bounding box, slice box and crosshair    */
   /* we draw these after the slice so that they blend correctly */
   if(pane->bounding_box){
      glLineWidth(1.5);
      glColor4d(0.0, 0.5, 1.0, 0.5);
      draw_bounding_box(pane, view);
      }

   if(pane->slice_box){
      glLineWidth(1.0);
      glColor4d(1.0, 1.0, 0.3, 0.7);
      draw_slice_box(pane, view);
      }

   /* translate to the cursor position before the following */
   glPushMatrix();
   glTranslated(pane->w[0], pane->w[1], pane->w[2]);

   if(pane->crosshair){
      if(view->tilt){
         glColor4d(1.0, 0, 0, 0.3);
         }
      else {
         glColor4d(1.0, 0, 0, 1.0);
         }
      draw_crosshair(pane, view);
      }

   /* rotate to the oblique plane then draw tilt up-vector */
   if(view->tilt){
      build_rotmatrix(tmp_m, view->tilt_quat);
      glMultMatrixd(&tmp_m[0][0]);

      glColor4d(1.0, 1.0, 0.0, 1.0);
      draw_crosshair(pane, view);

      /* and the rotating sphere if we are actively tilting */
      if(view->tilting){
         glLineWidth(1.0);
         glColor4d(1.0, 1.0, 1.0, 0.2);
         gluQuadricDrawStyle(view->quadric, GLU_LINE);
         gluSphere(view->quadric, 40, 15, 15);
         }
      }

   glPopMatrix();
   gdk_gl_drawable_gl_end(gldrawable);

   /* Swap buffers. */
   gdk_gl_drawable_swap_buffers(gldrawable);

   /* increment the draw count */
   ptr->draw_count++;
   return (TRUE);
   }

/* This should be called whenever the size of the area changes   */
gint gtkgl_reshape(GtkWidget * widget, GdkEventConfigure * event, gpointer func_data)
{
   GdkGLDrawable *gldrawable;
   GdkGLContext *glcontext;
   GLdouble world_h, world_w, world_b, world_l;
   GLdouble tmp;
   GLdouble max_dist = 0.0;
   int c;

   /* Get pointer to gtkgl_info */
   Pane_info pane = ((GtkGL_info *) func_data)->pane;
   View_info view = ((GtkGL_info *) func_data)->view;
   
   /* if we don't have to draw this pane, return early */
   if(!pane->draw){
      return TRUE;
      }

   /* get the current drawable and context and make it current */
   gldrawable = gtk_widget_get_gl_drawable(widget);
   glcontext = gtk_widget_get_gl_context(widget);
   
   /* glBegin */
   if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)){
      return FALSE;      }

   view->pix_size[0] = (double)widget->allocation.width;
   view->pix_size[1] = (double)widget->allocation.height;

   /* setup the viewport */
   glViewport(0, 0, (GLint) view->pix_size[0], (GLint) view->pix_size[1]);
   glGetIntegerv(GL_VIEWPORT, view->viewport);
   
   /* set up widths and heights for main drawwing callback */
   world_w = fabs(pane->stops[view->x_idx] - pane->starts[view->x_idx]);
   world_h = fabs(pane->stops[view->y_idx] - pane->starts[view->y_idx]);

   world_l = (pane->starts[view->x_idx] < pane->stops[view->x_idx])
      ? pane->starts[view->x_idx]
      : pane->stops[view->x_idx];

   world_b = (pane->starts[view->y_idx] < pane->stops[view->y_idx])
      ? pane->starts[view->y_idx]
      : pane->stops[view->y_idx];

   /* if height is the limiting direction */
   if((world_h / view->pix_size[1]) > (world_w / view->pix_size[0])){
      view->view_start[1] = world_b;
      view->view_stop[1] = world_b + world_h;
      
      /* get the scaled width about the COV */
      tmp = (view->pix_size[0] * world_h) / view->pix_size[1];
      view->view_start[0] = pane->cov[view->x_idx] - tmp/2;
      view->view_stop[0] = view->view_start[0] + tmp;
      }
   else {                           /* width is the limiting direction */
      view->view_start[0] = world_l;
      view->view_stop[0] = world_l + world_w;
      
      /* get the scaled height about the COV */
      tmp = (view->pix_size[1] * world_w) / view->pix_size[0];
      view->view_start[1] = pane->cov[view->y_idx] - tmp/2;
      view->view_stop[1] = view->view_start[1] + tmp;
      }
   
//   g_print("+++ PIX_SIZE[0|1] %g|%g\n", view->pix_size[0], view->pix_size[1]);
//   g_print("+++ TYPE[%d]  world_{w|h|l|b} %g|%g|%g|%g  tmp: %g\n", view->type, world_w, world_h, world_l, world_b, tmp); 
//   g_print("+++ view_start|stop[0] %g|%g  view_start|stop[1] %g|%g\n", view->view_start[0], view->view_stop[0], view->view_start[1], view->view_stop[1]); 
   
   /* set up projection transformation */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   /* find the max distance from corner to corner */
   for(c = 0; c < 3; c++){
      max_dist += SQR2(pane->stops[c] - pane->starts[c]);
      }
   max_dist = sqrt(max_dist);

   /* set up frustum or ortho view */
   if(pane->perspective){
      glFrustum(view->view_start[0], view->view_stop[0],
                view->view_start[1], view->view_stop[1],
                (max_dist * 1.5) * view->GLscalefac,
                (max_dist * 2.6) * view->GLscalefac);
      }
   else {
      glOrtho(view->view_start[0], view->view_stop[0],
              view->view_start[1], view->view_stop[1],
              0.0,
              (max_dist*2 + 1) * view->GLscalefac);
      }

   /* Look at the COV */
 //  gluLookAt(pane->cov[0],
 //            pane->cov[1],
 //            (pane->cov[2] + max_dist) * view->GLscalefac,
 //            pane->cov[0], pane->cov[1], pane->cov[2], 
 //            0, 1, 0);
   gluLookAt(0, 0, max_dist * view->GLscalefac,
             0, 0, 0, 
             0, 1, 0);

   /* store this matrix for later reference */
   glGetDoublev(GL_PROJECTION_MATRIX, view->projMatrix);
   
   /* glEnd */
   gdk_gl_drawable_gl_end(gldrawable);
   
   view->refresh_view = TRUE;
   return TRUE;
   }

/* This function is a callback for the realization of the GtkGL widget. */
gint gtkgl_init(GtkWidget * widget, gpointer func_data)
{
   GdkGLDrawable *gldrawable;
   GdkGLContext *glcontext;

   /* Get pointer to gtkgl_info */
   Pane_info pane = ((GtkGL_info *) func_data)->pane;
   View_info view = ((GtkGL_info *) func_data)->view;
   
   /* get the current drawable and context and make it current */
   gldrawable = gtk_widget_get_gl_drawable(widget);
   glcontext = gtk_widget_get_gl_context(widget);

   if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)){
      return FALSE;
      }

   /* drawmode */
   if(pane->draw_fast){
      glDisable(GL_BLEND);
      }
   else {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LINE_SMOOTH);
      }

   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClearDepth(1.0);
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);
   
   g_print("Init view widget\n");

   view->quadric = gluNewQuadric();

   gdk_gl_drawable_gl_end(gldrawable);
   return TRUE;
   }

/* This function is a callback for the main GtkGL widget. */
gint gtkgl_destroy(GtkWidget * widget, gpointer func_data)
{
   View_info view = ((GtkGL_info *) func_data)->view;

   gluDeleteQuadric(view->quadric);
   glDeleteTextures(1, &(view->texmap_id));
   g_free(view->texmap);
   return TRUE;
   }

/* draws a vector slice of a volume in a view */
void draw_vector_slice(Pane_info pane, View_info view)
{
   double   dx, dy, dz;

   int      v[3];
   double   w[3];
   double   dw[3];
   double   euc, euc_max;
   double   vect_col[4];

   dx = dy = dz = 0.0;
   w[0] = pane->starts[0];
   w[1] = pane->starts[1];
   w[2] = pane->starts[2];

   glPointSize(1.0);
   glLineWidth(0.8);

   euc_max = sqrt(3 * SQR2(pane->pane_max - pane->pane_min)) / 2.0;

   v[view->z_idx] = (int)pane->v[view->z_idx];
   w[view->z_idx] = pane->w[view->z_idx];

   if(v[view->z_idx] >= 0 && v[view->z_idx] < pane->sizes[view->z_idx] - 1){

      w[view->y_idx] = pane->starts[view->y_idx];
      for(v[view->y_idx] = 0; v[view->y_idx] < pane->sizes[view->y_idx] - 1;
          v[view->y_idx]++){
         w[view->x_idx] = pane->starts[view->x_idx];
         for(v[view->x_idx] = 0; v[view->x_idx] < pane->sizes[view->x_idx] - 1;
             v[view->x_idx]++){

            get_vector_components(pane, v[0], v[1], v[2], &dx, &dy, &dz);

            dw[0] = w[0] + (dx * pane->vect_mult);
            dw[1] = w[1] + (dy * pane->vect_mult);
            dw[2] = w[2] + (dz * pane->vect_mult);

            euc = sqrt(SQR2(dx) * SQR2(dy) * SQR2(dz));
            if((euc > pane->vect_floor) && (euc < pane->vect_ceil)){

               if(pane->vect_points){
                  glColor4dv(pane->vect_point_col);
                  glBegin(GL_POINTS);
                  glVertex3dv(w);
                  glEnd();
                  }

               if(pane->RGB_vect){
                  vect_col[0] = fabs(dx) / euc_max;
                  vect_col[1] = fabs(dy) / euc_max;
                  vect_col[2] = fabs(dz) / euc_max;
                  }
               else {
                  vect_col[0] = pane->vect_col[0];
                  vect_col[1] = pane->vect_col[1];
                  vect_col[2] = pane->vect_col[2];
                  }

               if(pane->mult_vect){
                  vect_col[3] = euc / euc_max * pane->vect_alph_mult;
                  }
               else {
                  vect_col[3] = pane->vect_col[3];
                  }

               glColor4dv(vect_col);
               glBegin(GL_LINES);
               glVertex3dv(w);
               glVertex3dv(dw);
               glEnd();
               }

            w[view->x_idx] += pane->steps[view->x_idx];
            }

         w[view->y_idx] += pane->steps[view->y_idx];
         }
      }

   }

/* draws a slice of a volume in a view */
int draw_texture_slice(Pane_info pane, View_info v)
{

   /* setup the quad to be texture mapped */
   v->q0[v->x_idx] = pane->starts[v->x_idx];
   v->q1[v->x_idx] = pane->starts[v->x_idx];
   v->q0[v->y_idx] = pane->starts[v->y_idx];
   v->q1[v->y_idx] = pane->stops[v->y_idx];
   v->q0[v->z_idx] = pane->w[v->z_idx];
   v->q1[v->z_idx] = pane->w[v->z_idx];

   v->q2[v->x_idx] = pane->stops[v->x_idx];
   v->q3[v->x_idx] = pane->stops[v->x_idx];
   v->q2[v->y_idx] = pane->stops[v->y_idx];
   v->q3[v->y_idx] = pane->starts[v->y_idx];
   v->q2[v->z_idx] = pane->w[v->z_idx];
   v->q3[v->z_idx] = pane->w[v->z_idx];

   if(v->tilt){
      v->q0[v->z_idx] -= (pane->starts[v->x_idx] - pane->w[v->x_idx])
         * (v->tilt_w[v->x_idx] / v->tilt_w[v->z_idx])
         + (pane->starts[v->y_idx] - pane->w[v->y_idx])
         * (v->tilt_w[v->y_idx] / v->tilt_w[v->z_idx]);

      v->q1[v->z_idx] -= (pane->starts[v->x_idx] - pane->w[v->x_idx])
         * (v->tilt_w[v->x_idx] / v->tilt_w[v->z_idx])
         + (pane->stops[v->y_idx] - pane->w[v->y_idx])
         * (v->tilt_w[v->y_idx] / v->tilt_w[v->z_idx]);

      v->q2[v->z_idx] -= (pane->stops[v->x_idx] - pane->w[v->x_idx])
         * (v->tilt_w[v->x_idx] / v->tilt_w[v->z_idx])
         + (pane->stops[v->y_idx] - pane->w[v->y_idx])
         * (v->tilt_w[v->y_idx] / v->tilt_w[v->z_idx]);

      v->q3[v->z_idx] -= (pane->stops[v->x_idx] - pane->w[v->x_idx])
         * (v->tilt_w[v->x_idx] / v->tilt_w[v->z_idx])
         + (pane->starts[v->y_idx] - pane->w[v->y_idx])
         * (v->tilt_w[v->y_idx] / v->tilt_w[v->z_idx]);
      }

   /* load and alloc the texture map if needed */
   if(v->reload_texmap){
      load_view_texmap(pane, v);
      }

   /* set the Texture interpolation function */
   if(pane->linear_interp){
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      }
   else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      }

   /* reload the cmap if needed */
   if(v->reload_cmap){
      glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 256, pane->cmap_r);
      glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 256, pane->cmap_g);
      glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 256, pane->cmap_b);
      glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 256, pane->cmap_a);

      v->reload_cmap = FALSE;
      v->reload_image = TRUE;
      }

   /* get a new image if needed */
   if(v->reload_image){
      if(!get_minc_image(pane, v)){
         return FALSE;
         }
      
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                      pane->sizes[v->x_idx], pane->sizes[v->y_idx],
                      GL_COLOR_INDEX, GL_UNSIGNED_BYTE, (GLvoid *) v->texmap);

      v->reload_image = FALSE;
      }

   /* draw the image itself via a 2D texture-map */
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, v->texmap_id);
   glColor4d(1.0, 1.0, 1.0, 1.0);

   glBegin(GL_QUADS);
   glTexCoord2d(0.0, 0.0);
   glVertex3dv(v->q0);

   glTexCoord2d(0.0, v->texmap_stop[0]);
   glVertex3dv(v->q1);

   glTexCoord2d(v->texmap_stop[1], v->texmap_stop[0]);
   glVertex3dv(v->q2);

   glTexCoord2d(v->texmap_stop[1], 0.0);
   glVertex3dv(v->q3);
   glEnd();

   glFlush();
   glDisable(GL_TEXTURE_2D);

   CHECK_OPENGL_ERROR();

   return TRUE;
   }

/* Draw a Cross hair in 3D */
void draw_crosshair(Pane_info pane, View_info view)
{
   GLdouble size = pane->crosshair_size;
   GLdouble csize;

   if(size < 5){
      size = 5;
      }
   csize = 1 / view->GLscalefac;

   gluQuadricDrawStyle(view->quadric, GLU_FILL);
   glEnable(GL_POLYGON_SMOOTH);
   /* draw the x axis + arrow */
   glPushMatrix();
   glTranslated(0.0, 0.0, -size);
   gluCylinder(view->quadric, 0, csize, size * 2, 12.0, 1);

   glTranslated(0.0, 0.0, size * 2);
   gluCylinder(view->quadric, csize * 3, 0.0, csize * 7, 16.0, 1);
   glPopMatrix();

   /* draw the y axis + arrow */
   glPushMatrix();
   glRotated(-90.0, 1.0, 0.0, 0.0);
   glTranslated(0.0, 0.0, -size);
   gluCylinder(view->quadric, 0, csize, size * 2, 12.0, 1);

   glTranslated(0.0, 0.0, size * 2);
   gluCylinder(view->quadric, csize * 3, 0.0, csize * 7, 16.0, 1);
   glPopMatrix();

   /* draw the z axis + arrow */
   glPushMatrix();
   glRotated(90.0, 0.0, 1.0, 0.0);
   glTranslated(0.0, 0.0, -size);
   gluCylinder(view->quadric, 0, csize, size * 2, 12.0, 1);

   glTranslated(0.0, 0.0, size * 2);
   gluCylinder(view->quadric, csize * 3, 0.0, csize * 7, 16.0, 1);
   glPopMatrix();

   glDisable(GL_POLYGON_SMOOTH);
   }

/* Draw a 3D bounding box */
void draw_bounding_box(Pane_info pane, View_info view)
{

   glBegin(GL_LINE_STRIP);
   glVertex3d(pane->starts[0], pane->starts[1], pane->starts[2]);
   glVertex3d(pane->stops[0], pane->starts[1], pane->starts[2]);
   glVertex3d(pane->stops[0], pane->stops[1], pane->starts[2]);
   glVertex3d(pane->starts[0], pane->stops[1], pane->starts[2]);
   glVertex3d(pane->starts[0], pane->starts[1], pane->starts[2]);
   glVertex3d(pane->starts[0], pane->starts[1], pane->stops[2]);
   glVertex3d(pane->stops[0], pane->starts[1], pane->stops[2]);
   glVertex3d(pane->stops[0], pane->stops[1], pane->stops[2]);
   glVertex3d(pane->starts[0], pane->stops[1], pane->stops[2]);
   glVertex3d(pane->starts[0], pane->starts[1], pane->stops[2]);
   glEnd();

   glBegin(GL_LINES);
   glVertex3d(pane->stops[0], pane->starts[1], pane->starts[2]);
   glVertex3d(pane->stops[0], pane->starts[1], pane->stops[2]);
   glVertex3d(pane->stops[0], pane->stops[1], pane->starts[2]);
   glVertex3d(pane->stops[0], pane->stops[1], pane->stops[2]);
   glVertex3d(pane->starts[0], pane->stops[1], pane->starts[2]);
   glVertex3d(pane->starts[0], pane->stops[1], pane->stops[2]);
   glEnd();
   }

/* draw a line for the slice on the bounding box */
void draw_bbox_line(double *obl_norm, double *obl_point,
                    double *box_norm, double *box_point, double *min_vec, double *max_vec)
{

   double   v0[3], v1[3];
   double   line_norm[3], line_point[3];
   double   tmp_norm[3], tmp_point0[3], tmp_point1[3];
   int      idx0 = 1, idx1 = 0;
   int      status = TRUE;

   /* intersect the oblique with the bbox wall */
   status &= intersect_planes(line_norm, line_point,
                              box_norm, box_point, obl_norm, obl_point);

   /* then extend this line to a nearby bbox_wall */
   if(box_norm[0] == 1){              /* x wall */
      vset(tmp_norm, 0, 1, 0);
      vset(tmp_point0, 0, min_vec[1], 0);
      vset(tmp_point1, 0, max_vec[1], 0);
      idx0 = 1;
      idx1 = 2;
      }
   else if(box_norm[1] == 1){         /* y wall */
      vset(tmp_norm, 1, 0, 0);
      vset(tmp_point0, min_vec[0], 0, 0);
      vset(tmp_point1, max_vec[0], 0, 0);
      idx0 = 0;
      idx1 = 2;
      }
   else if(box_norm[2] == 1){         /* z wall */
      vset(tmp_norm, 1, 0, 0);
      vset(tmp_point0, min_vec[0], 0, 0);
      vset(tmp_point1, max_vec[0], 0, 0);
      idx0 = 0;
      idx1 = 1;
      }

   status &= intersect_plane_line_n(v0, line_norm, line_point, tmp_norm, tmp_point0);
   status &= intersect_plane_line_n(v1, line_norm, line_point, tmp_norm, tmp_point1);

   /* then clip it to the box */
   status &= cohen_sutherland_clip(&v0[idx0], &v0[idx1], &v1[idx0], &v1[idx1],
                                   min_vec[idx0], max_vec[idx0],
                                   min_vec[idx1], max_vec[idx1]);
   if(status){
      glBegin(GL_LINES);
      glVertex3dv(v0);
      glVertex3dv(v1);
      glEnd();
      }

   }

/* draw a slice box - some serious nerdly fun here */
void draw_slice_box(Pane_info pane, View_info view)
{
   double   pnorm[3], ppoint[3];

   /* x-walls */
   vset(pnorm, 1, 0, 0);
   vset(ppoint, pane->starts[0], 0, 0);
   draw_bbox_line(view->tilt_w, pane->w, pnorm, ppoint, pane->starts, pane->stops);

   vset(ppoint, pane->stops[0], 0, 0);
   draw_bbox_line(view->tilt_w, pane->w, pnorm, ppoint, pane->starts, pane->stops);

   /* y-walls */
   vset(pnorm, 0, 1, 0);
   vset(ppoint, 0, pane->starts[1], 0);
   draw_bbox_line(view->tilt_w, pane->w, pnorm, ppoint, pane->starts, pane->stops);

   vset(ppoint, 0, pane->stops[1], 0);
   draw_bbox_line(view->tilt_w, pane->w, pnorm, ppoint, pane->starts, pane->stops);

   /* z-walls */
   vset(pnorm, 0, 0, 1);
   vset(ppoint, 0, 0, pane->starts[2]);
   draw_bbox_line(view->tilt_w, pane->w, pnorm, ppoint, pane->starts, pane->stops);

   vset(ppoint, 0, 0, pane->stops[2]);
   draw_bbox_line(view->tilt_w, pane->w, pnorm, ppoint, pane->starts, pane->stops);
   }
