/* callbacks.c */

#include "callbacks.h"
#include <math.h>

#include "viewnup.h"
#include "lookup_table.h"
#include "minc_io.h"
#include "interface.h"
#include "gtk_gl.h"
#include "trackball.h"
#include "geometry.h"

extern int verbose;

/* internal function prototypes */
int      init_coord_ranges(Pane_info pane);
void     assign_coord_adjustments(Pane_info pane);
void     update_pane_and_filename(Pane_info pane);
void     show_hide_coord_spins(Pane_info pane);
int      update_ranges(Pane_info pane);
void     update_voxel_value(Pane_info pane);
void     update_merge_combos(Main_info * ptr);

/* functions to do stuff to the interface - externally callable */
void init_highlight_styles(Main_info * ptr)
{
   GdkColor white = { 0, 65535, 65535, 65535 };
   GdkColor drk_blue = { 0, 0, 2560, 20480 };
   GdkColor green = { 0, 0, 65535, 0 };

   ptr->pane_highlight_style = gtk_style_new();
   ptr->view_highlight_style = gtk_style_new();

   ptr->pane_highlight_style->fg[GTK_STATE_NORMAL] = white;
   ptr->pane_highlight_style->bg[GTK_STATE_NORMAL] = drk_blue;

   ptr->view_highlight_style->bg[GTK_STATE_NORMAL] = green;
   }

/* updates the statusbar */
void push_statusbar(Main_info * ptr, char *buf, int highlight)
{

   /* check as a just in case */
   if(ptr == NULL){
      ptr = get_main_ptr();
      }

   if(buf == NULL){
      g_print(_("push_statusbar passed a NULL pointer!"));
      return;
      }
   else {
      gtk_statusbar_pop(GTK_STATUSBAR(ptr->statusbar), 1);
      gtk_statusbar_push(GTK_STATUSBAR(ptr->statusbar), 1, buf);
      }

   if(highlight){
      gtk_widget_set_style(GTK_WIDGET(ptr->statusbar), ptr->pane_highlight_style);
      }
   else {
      gtk_widget_set_style(GTK_WIDGET(ptr->statusbar), NULL);
      }
   }

/* change the current active pane */
void make_pane_active(Pane_info pane)
{
   Main_info *ptr = get_main_ptr();
   Pane_dialog *pi = ptr->pane_dialog;

   if(pane != ptr->c_pane){

      /* update pane styles, new then fix up old */
//      gtk_widget_set_style(GTK_WIDGET(pane->pane_label), ptr->pane_highlight_style);
//      gtk_widget_set_style(GTK_WIDGET(pane->eventbox_label), ptr->pane_highlight_style);
//      if(ptr->c_pane != NULL){
//         gtk_widget_set_style(GTK_WIDGET(ptr->c_pane->pane_label), NULL);
//         gtk_widget_set_style(GTK_WIDGET(ptr->c_pane->eventbox_label), NULL);
//         }

      /* change coordinate adjustments and values */
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->wx_spinbutton),
                                     GTK_ADJUSTMENT(pane->coord_wx_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->wy_spinbutton),
                                     GTK_ADJUSTMENT(pane->coord_wy_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->wz_spinbutton),
                                     GTK_ADJUSTMENT(pane->coord_wz_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->wt_spinbutton),
                                     GTK_ADJUSTMENT(pane->coord_wt_adj));

      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->vx_spinbutton),
                                     GTK_ADJUSTMENT(pane->coord_vx_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->vy_spinbutton),
                                     GTK_ADJUSTMENT(pane->coord_vy_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->vz_spinbutton),
                                     GTK_ADJUSTMENT(pane->coord_vz_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->vt_spinbutton),
                                     GTK_ADJUSTMENT(pane->coord_vt_adj));

      gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->coord_vx_adj));
      gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->coord_vy_adj));
      gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->coord_vz_adj));
      gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->coord_vt_adj));
      gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->coord_wx_adj));
      gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->coord_wy_adj));
      gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->coord_wz_adj));
      gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->coord_wt_adj));

      /* Views frame */
      g_signal_handlers_block_by_func(G_OBJECT(pi->linear_interp_checkbutton),
                                      G_CALLBACK(pi_linear_interp_checkbutton_toggled),
                                      ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->vector_checkbutton), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->perspective_checkbutton), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->bbox_checkbutton), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->slicebox_checkbutton), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->crosshair_checkbutton), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->crosshair_spinbutton), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->scale_link_button), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_link_button), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->trans_link_button), ptr);
      gtk_signal_handler_block_by_data(GTK_OBJECT(pi->tilt_link_button), ptr);

      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->linear_interp_checkbutton),
                                   pane->linear_interp);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->vector_checkbutton),
                                   pane->vector);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->perspective_checkbutton),
                                   pane->perspective);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->bbox_checkbutton),
                                   pane->bounding_box);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->slicebox_checkbutton),
                                   pane->slice_box);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->crosshair_checkbutton),
                                   pane->crosshair);

      gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->crosshair_spinbutton),
                                pane->crosshair_size);

      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->scale_link_button),
                                   pane->link_scales);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->rot_link_button),
                                   pane->link_rots);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->trans_link_button),
                                   pane->link_trans);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->tilt_link_button),
                                   pane->link_tilts);

      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->perspective_checkbutton), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->vector_checkbutton), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->slicebox_checkbutton), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->linear_interp_checkbutton), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->bbox_checkbutton), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->crosshair_checkbutton), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->scale_link_button), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->trans_link_button), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_link_button), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->tilt_link_button), ptr);
      gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->crosshair_spinbutton), ptr);

      /* make the panes current view active */
      if(pane->c_view != NULL){
         make_view_active(pane, pane->c_view);
         }
      else {
         make_view_active(pane, g_ptr_array_index(pane->views, 0));
         }

      /* set a few buttons to insensitive */
      if(pane->merge){
         gtk_widget_set_sensitive(GTK_WIDGET(pi->vector_checkbutton), FALSE);
         }
      else {
         gtk_widget_set_sensitive(GTK_WIDGET(pi->vector_checkbutton), TRUE);
         }

      /* hide and show the appropriate frames */
      if(pane->merge){
         gtk_widget_show(GTK_WIDGET(ptr->pane_dialog->merge_frame));
         gtk_widget_hide(GTK_WIDGET(ptr->pane_dialog->cmap_frame));
         gtk_widget_hide(GTK_WIDGET(ptr->pane_dialog->vector_frame));
         }
      else {
         gtk_widget_hide(GTK_WIDGET(ptr->pane_dialog->merge_frame));

         if(pane->vector){
            gtk_widget_show(GTK_WIDGET(ptr->pane_dialog->vector_frame));
            gtk_widget_hide(GTK_WIDGET(ptr->pane_dialog->cmap_frame));
            }
         else {
            gtk_widget_show(GTK_WIDGET(ptr->pane_dialog->cmap_frame));
            gtk_widget_hide(GTK_WIDGET(ptr->pane_dialog->vector_frame));

            /* update cmap frame */
            gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->cmap_min_spinbutton),
                                           GTK_ADJUSTMENT(pane->range_min_adj));
            gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pi->cmap_max_spinbutton),
                                           GTK_ADJUSTMENT(pane->range_max_adj));

            gtk_range_set_adjustment(GTK_RANGE(pi->cmap_min_hscale),
                                     GTK_ADJUSTMENT(pane->range_min_adj));
            gtk_range_set_adjustment(GTK_RANGE(pi->cmap_max_hscale),
                                     GTK_ADJUSTMENT(pane->range_max_adj));

            gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->range_max_adj));
            gtk_adjustment_value_changed(GTK_ADJUSTMENT(pane->range_min_adj));

//            gtk_range_slider_update(GTK_RANGE(pi->cmap_min_hscale));
//            gtk_range_slider_update(GTK_RANGE(pi->cmap_max_hscale));
            }
         }

      /* update c_pane ptr */
      ptr->c_pane = pane;
      }
   }

/* change the current active view */
void make_view_active(Pane_info pane, View_info view)
{
   Main_info *ptr = get_main_ptr();
   Pane_dialog *pi = ptr->pane_dialog;

   /* change styles if we have to */
   if(view != pane->c_view){
      /* first change this view */
      gtk_widget_set_style(GTK_WIDGET(view->view_frame), ptr->view_highlight_style);

      /* then restore the previous c_view */
      if(pane->c_view != NULL){
         gtk_widget_set_style(GTK_WIDGET(pane->c_view->view_frame), NULL);
         }
      }

   /* block pertinent callbacks */
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->view_type_combo_entry), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->scale_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->trans_x_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->trans_y_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->trans_z_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_x_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_y_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_z_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_phi_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->tilt_x_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->tilt_y_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->tilt_z_spinbutton), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->scale_lock_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->trans_lock_x_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->trans_lock_y_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->trans_lock_z_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_lock_x_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_lock_y_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_lock_z_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->rot_lock_phi_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->tilt_lock_x_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->tilt_lock_y_button), ptr);
   gtk_signal_handler_block_by_data(GTK_OBJECT(pi->tilt_lock_z_button), ptr);

   /* view and view-type combos */
   switch (view->type){
   default:
   case TRANSVERSE:
      gtk_entry_set_text(GTK_ENTRY(pi->view_type_combo_entry), _("transverse"));
      break;
   case SAGITTAL:
      gtk_entry_set_text(GTK_ENTRY(pi->view_type_combo_entry), _("sagittal"));
      break;
   case CORONAL:
      gtk_entry_set_text(GTK_ENTRY(pi->view_type_combo_entry), _("coronal"));
      break;
      }

   /* scale, trans, rot and tilt spins */
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->scale_spinbutton), view->GLscalefac);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->trans_x_spinbutton), view->GLtrans_x);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->trans_y_spinbutton), view->GLtrans_y);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->trans_z_spinbutton), view->GLtrans_z);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->rot_x_spinbutton), view->rot_vec[0]);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->rot_y_spinbutton), view->rot_vec[1]);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->rot_phi_spinbutton),
                             view->rot_phi * 180 / M_PI);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->rot_z_spinbutton), view->rot_vec[2]);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->tilt_x_spinbutton), view->tilt_w[0]);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->tilt_y_spinbutton), view->tilt_w[1]);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->tilt_z_spinbutton), view->tilt_w[2]);

   /* set lock toggles */
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->scale_lock_button),
                                view->lock_scale);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->trans_lock_x_button),
                                view->lock_x_trans);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->trans_lock_y_button),
                                view->lock_y_trans);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->trans_lock_z_button),
                                view->lock_z_trans);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->rot_lock_x_button),
                                view->lock_x_rot);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->rot_lock_y_button),
                                view->lock_y_rot);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->rot_lock_z_button),
                                view->lock_z_rot);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->rot_lock_phi_button),
                                view->lock_phi_rot);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->tilt_lock_x_button),
                                view->lock_x_tilt);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->tilt_lock_y_button),
                                view->lock_y_tilt);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pi->tilt_lock_z_button),
                                view->lock_z_tilt);

   /* then unblock */
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->view_type_combo_entry), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->scale_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->trans_x_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->trans_y_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->trans_z_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_x_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_y_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_z_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_phi_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->tilt_x_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->tilt_y_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->tilt_z_spinbutton), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->scale_lock_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->trans_lock_x_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->trans_lock_y_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->trans_lock_z_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_lock_x_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_lock_y_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_lock_z_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->rot_lock_phi_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->tilt_lock_x_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->tilt_lock_y_button), ptr);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pi->tilt_lock_z_button), ptr);

   pane->c_view = view;
   }

void update_pi_scale(Main_info * ptr, double scalefac)
{
   Pane_dialog *pi = ptr->pane_dialog;

   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->scale_spinbutton), scalefac);
   }

void update_pi_trans(Main_info * ptr, double x, double y, double z)
{
   Pane_dialog *pi = ptr->pane_dialog;

   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->trans_x_spinbutton), x);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->trans_y_spinbutton), y);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->trans_z_spinbutton), z);
   }

void update_pi_rots(Main_info * ptr, double x, double y, double z, double phi)
{
   Pane_dialog *pi = ptr->pane_dialog;

   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->rot_x_spinbutton), x);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->rot_y_spinbutton), y);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->rot_z_spinbutton), z);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->rot_phi_spinbutton), phi * 180.0 / M_PI);
   }

void update_pi_tilts(Main_info * ptr, double x, double y, double z)
{
   Pane_dialog *pi = ptr->pane_dialog;

   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->tilt_x_spinbutton), x);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->tilt_y_spinbutton), y);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pi->tilt_z_spinbutton), z);
   }

/* set the filename and pane title */
void update_pane_and_filename(Pane_info pane)
{
   if(!pane->merge){
      if(pane->file_name->str != NULL){
         gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(pane->file_open_combo)->entry),
                            pane->file_name->str);
         }
      }
   }

/* set up cmap ranges to file min and max (attach adjustments) */
int update_ranges(Pane_info pane)
{
   GtkAdjustment *min = GTK_ADJUSTMENT(pane->range_min_adj);
   GtkAdjustment *max = GTK_ADJUSTMENT(pane->range_max_adj);

   gdouble  inc_size;

   /* make a guess at a sensible page size */
   inc_size = fabs(pane->pane_max - pane->pane_min) / 512;

   /* first we do the lower ones */
   min->lower = pane->pane_min;
   min->value = pane->pane_min_value;
   min->upper = pane->pane_max;
   min->step_increment = inc_size;
   min->page_increment = inc_size * 4;
   min->page_size = inc_size;

   max->lower = pane->pane_min;
   max->value = pane->pane_max_value;
   max->upper = pane->pane_max;
   max->step_increment = inc_size;
   max->page_increment = inc_size * 4;
   max->page_size = inc_size;

   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->range_min_val),
                                  GTK_ADJUSTMENT(pane->range_min_adj));
   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->range_max_val),
                                  GTK_ADJUSTMENT(pane->range_max_adj));

   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->range_min_val), pane->pane_min_value);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->range_max_val), pane->pane_max_value);

   gtk_range_set_adjustment(GTK_RANGE(pane->range_min_scale),
                            GTK_ADJUSTMENT(pane->range_min_adj));
   gtk_range_set_adjustment(GTK_RANGE(pane->range_max_scale),
                            GTK_ADJUSTMENT(pane->range_max_adj));

   return TRUE;
   }

/* set the coordinate values */
void update_coord_values(Pane_info pane)
{
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_wx), pane->w[0]);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_wy), pane->w[1]);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_wz), pane->w[2]);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_wt), pane->w[3]);
   }

void assign_coord_adjustments(Pane_info pane)
{

   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_vx),
                                  GTK_ADJUSTMENT(pane->coord_vx_adj));
   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_vy),
                                  GTK_ADJUSTMENT(pane->coord_vy_adj));
   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_vz),
                                  GTK_ADJUSTMENT(pane->coord_vz_adj));
   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_vt),
                                  GTK_ADJUSTMENT(pane->coord_vt_adj));

   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_wx),
                                  GTK_ADJUSTMENT(pane->coord_wx_adj));
   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_wy),
                                  GTK_ADJUSTMENT(pane->coord_wy_adj));
   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_wz),
                                  GTK_ADJUSTMENT(pane->coord_wz_adj));
   gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_wt),
                                  GTK_ADJUSTMENT(pane->coord_wt_adj));
   }

void show_hide_coord_spins(Pane_info pane)
{

   if(pane->use_vox_coords){
      gtk_widget_show(pane->coord_vx);
      gtk_widget_show(pane->coord_vy);
      gtk_widget_show(pane->coord_vz);
      gtk_widget_show(pane->coord_vt);

      gtk_widget_hide(pane->coord_wx);
      gtk_widget_hide(pane->coord_wy);
      gtk_widget_hide(pane->coord_wz);
      gtk_widget_hide(pane->coord_wt);
      }
   else {
      gtk_widget_show(pane->coord_wx);
      gtk_widget_show(pane->coord_wy);
      gtk_widget_show(pane->coord_wz);
      gtk_widget_show(pane->coord_wt);

      gtk_widget_hide(pane->coord_vx);
      gtk_widget_hide(pane->coord_vy);
      gtk_widget_hide(pane->coord_vz);
      gtk_widget_hide(pane->coord_vt);
      }
   }

/* set the coordinate ranges */
int init_coord_ranges(Pane_info pane)
{
   GtkAdjustment *vx_adj = GTK_ADJUSTMENT(pane->coord_vx_adj);
   GtkAdjustment *vy_adj = GTK_ADJUSTMENT(pane->coord_vy_adj);
   GtkAdjustment *vz_adj = GTK_ADJUSTMENT(pane->coord_vz_adj);
   GtkAdjustment *vt_adj = GTK_ADJUSTMENT(pane->coord_vt_adj);

   GtkAdjustment *wx_adj = GTK_ADJUSTMENT(pane->coord_wx_adj);
   GtkAdjustment *wy_adj = GTK_ADJUSTMENT(pane->coord_wy_adj);
   GtkAdjustment *wz_adj = GTK_ADJUSTMENT(pane->coord_wz_adj);
   GtkAdjustment *wt_adj = GTK_ADJUSTMENT(pane->coord_wt_adj);

   vx_adj->lower = 0;
   vx_adj->upper = pane->sizes[0] - 1;
   vy_adj->lower = 0;
   vy_adj->upper = pane->sizes[1] - 1;
   vz_adj->lower = 0;
   vz_adj->upper = pane->sizes[2] - 1;
   vt_adj->lower = 0;
   vt_adj->upper = pane->sizes[3] - 1;

   if(pane->starts[0] < pane->stops[0]){
      wx_adj->lower = pane->starts[0];
      wx_adj->upper = pane->stops[0];
      }
   else {
      wx_adj->upper = pane->starts[0];
      wx_adj->lower = pane->stops[0];
      }

   if(pane->starts[1] < pane->stops[1]){
      wy_adj->lower = pane->starts[1];
      wy_adj->upper = pane->stops[1];
      }
   else {
      wy_adj->upper = pane->starts[1];
      wy_adj->lower = pane->stops[1];
      }

   if(pane->starts[2] < pane->stops[2]){
      wz_adj->lower = pane->starts[2];
      wz_adj->upper = pane->stops[2];
      }
   else {
      wz_adj->upper = pane->starts[2];
      wz_adj->lower = pane->stops[2];
      }

   if(pane->starts[3] < pane->stops[3]){
      wt_adj->lower = pane->starts[3];
      wt_adj->upper = pane->stops[3];
      }
   else {
      wt_adj->upper = pane->starts[3];
      wt_adj->lower = pane->stops[3];
      }

   assign_coord_adjustments(pane);
   return TRUE;
   }

/* update the voxel value entry widget */
void update_voxel_value(Pane_info pane)
{
   char     num_string[128];

   /* check if we have to do anything */
   if(pane->merge || pane->volume == NULL){
      return;
      }

   get_voxel_value(pane);

   if(pane->value_type == VALUE_RGBA){
      sprintf(num_string, "%.2f|%.2f|%.2f|%.2f",
              pane->cmap_r[(int)pane->voxel_value],
              pane->cmap_g[(int)pane->voxel_value],
              pane->cmap_b[(int)pane->voxel_value], pane->cmap_a[(int)pane->voxel_value]);
      }
   else {
      sprintf(num_string, "%g", pane->voxel_value);
      }

   gtk_entry_set_text(GTK_ENTRY(pane->vox_value), num_string);
   }

/* update the synch button and more importantly the adjustments */
void update_synch_button(Main_info * ptr, Pane_info pane)
{
   Synch_info *synch;
   char     synch_text[10];

   if(pane->synch->idx == 0){
      sprintf(synch_text, "S:-");

      assign_coord_adjustments(pane);
      update_coord_values(pane);
      }

   else {
      sprintf(synch_text, "S:%d", pane->synch->idx);

      /* point the panes coordinate's at the synch adjustments */
      synch = pane->synch;
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_wx),
                                     GTK_ADJUSTMENT(synch->synch_x_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_wy),
                                     GTK_ADJUSTMENT(synch->synch_y_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_wz),
                                     GTK_ADJUSTMENT(synch->synch_z_adj));
      gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(pane->coord_wt),
                                     GTK_ADJUSTMENT(synch->synch_t_adj));

      if(synch->virgin){
         /* assign the panes coordinates to the sync */
         gtk_spin_button_set_value(GTK_SPIN_BUTTON(synch->synch_x), pane->w[0]);
         gtk_spin_button_set_value(GTK_SPIN_BUTTON(synch->synch_y), pane->w[1]);
         gtk_spin_button_set_value(GTK_SPIN_BUTTON(synch->synch_z), pane->w[2]);
         gtk_spin_button_set_value(GTK_SPIN_BUTTON(synch->synch_t), pane->w[3]);
         synch->virgin = FALSE;
         }
      else {
         /* assign the sync's co-ordinates to the pane */
         pane->w[0] = (double)
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(synch->synch_x));
         pane->w[1] = (double)
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(synch->synch_y));
         pane->w[2] = (double)
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(synch->synch_z));
         pane->w[3] = (double)
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(synch->synch_t));

         update_voxel_from_world(pane);
         }
      }
   update_coord_values(pane);

   gtk_button_set_label(GTK_BUTTON(pane->sync_button), synch_text);
   }

void update_merge_combos(Main_info * ptr)
{
   int      c;
   gchar   *prev;

   /* first rebuild the merge_filelist */
   build_pane_filelist(ptr);

   /* now attempt some trickery */
   for(c = 0; c < NUM_MERGE_COEFF; c++){

      /* get the current value */
      prev =
         g_strdup(gtk_entry_get_text(GTK_ENTRY(ptr->pane_dialog->merge_combo_entry[c])));

      /* block the combo callback */
      gtk_signal_handler_block_by_data(GTK_OBJECT(ptr->pane_dialog->merge_combo_entry[c]),
                                       GINT_TO_POINTER(c));

      /* update the combo */
      gtk_combo_set_popdown_strings(GTK_COMBO(ptr->pane_dialog->merge_combo[c]),
                                    ptr->pane_filelist);

      /* put it back to what it was if it still exists */
      if(g_list_find_custom(ptr->pane_filelist, prev, (GCompareFunc) strcmp) != NULL){
         gtk_entry_set_text(GTK_ENTRY(ptr->pane_dialog->merge_combo_entry[c]), prev);
         }

      /* unblock the combo callback */
      gtk_signal_handler_unblock_by_data(GTK_OBJECT
                                         (ptr->pane_dialog->merge_combo_entry[c]),
                                         GINT_TO_POINTER(c));
      }

   g_free(prev);
   }

/* adds a pane to the end and returns a pointer to it's struct */
Pane_info add_pane(Main_info * ptr, int clone, Pane_info clone_pane, int merge)
{
   Pane_info pane;
   View_info view;
   int      c;

   /* create the pane structure and widgets */
   pane = init_pane_info(ptr->init_synch);
   pane->position = ptr->panes->len;
   g_ptr_array_add(ptr->panes, pane);

   /* set the merge variable */
   pane->merge = merge;

   /* set the file label and filename (and cmap adjustments) */
   if(merge){
      ptr->last_merge_id++;
      g_string_sprintf(pane->file_basename, "%s %02d", MERGE_STR, ptr->last_merge_id);
      }
   else {
      g_string_sprintf(pane->file_basename, NO_FILE_STR);

      /* cmap range adjusters */
      pane->range_min_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);
      pane->range_max_adj = gtk_adjustment_new(1, 0, 1, 1, 10, 1);

      g_object_ref(pane->range_min_adj);
      g_object_ref(pane->range_max_adj);
      }

   /* coordinate adjusters */
   pane->coord_vx_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);
   pane->coord_vy_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);
   pane->coord_vz_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);
   pane->coord_vt_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);

   pane->coord_wx_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);
   pane->coord_wy_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);
   pane->coord_wz_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);
   pane->coord_wt_adj = gtk_adjustment_new(0, 0, 1, 1, 10, 1);

   g_object_ref(pane->coord_vx_adj);
   g_object_ref(pane->coord_vy_adj);
   g_object_ref(pane->coord_vz_adj);
   g_object_ref(pane->coord_vt_adj);

   g_object_ref(pane->coord_wx_adj);
   g_object_ref(pane->coord_wy_adj);
   g_object_ref(pane->coord_wz_adj);
   g_object_ref(pane->coord_wt_adj);

   /* create the widgets now that we have adjustments */
   create_pane_widgets(ptr, pane);

   /* populate cmaps list */
   if(!merge){
      gtk_combo_set_popdown_strings(GTK_COMBO(pane->cmap_combo), ptr->cmap_combo_items);
      }

   /* init a few things */
   update_pane_and_filename(pane);
   show_hide_coord_spins(pane);
   update_synch_button(ptr, pane);

   if(clone){
      g_print("can't add a clone pane yet\n");
      }
   else {
      /* setup initial views */
      if(ptr->init_transverse){
         g_ptr_array_add(pane->views, init_view_info(TRANSVERSE));
         }
      if(ptr->init_sagittal){
         g_ptr_array_add(pane->views, init_view_info(SAGITTAL));
         }
      if(ptr->init_coronal){
         g_ptr_array_add(pane->views, init_view_info(CORONAL));
         }
      }

   /* create the approriate gtkglarea windows */
   for(c = 0; c < pane->views->len; c++){
      view = g_ptr_array_index(pane->views, c);
      create_view_widgets(ptr, pane, view);
      }

   if(verbose){
      g_print("Added pane %s Merge: %d  Clone %d  Position %d\n",
              pane->file_basename->str, merge, clone, pane->position);
      }
   return pane;
   }

/* menu callbacks */
void open_scheme_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   GtkWidget *scheme_filesel;

   scheme_filesel = create_scheme_fileselection();
   gtk_widget_show(scheme_filesel);
   }

void open_cmap_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   GtkWidget *cmap_filesel;

   cmap_filesel = create_cmap_fileselection();
   gtk_widget_show(cmap_filesel);
   }

void save_view_activate(GtkMenuItem * menuitem, gpointer user_data)
{

   }

void save_pane_activate(GtkMenuItem * menuitem, gpointer user_data)
{

   }

void save_window_activate(GtkMenuItem * menuitem, gpointer user_data)
{

   }

void print_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   }

void exit_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   exit(0);
   }

void edit_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   }

void dump_info_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   print_pane_info(get_main_ptr()->c_pane);
   }

void intensity_profiles_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   g_print("BOOOM! - not dun yet, cry.\n");
   }

void synch_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();

   gtk_widget_show(ptr->synch_dialog);
   }

void pane_info_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();

   gtk_widget_show(ptr->pane_info_dialog);

   gdk_window_show(ptr->pane_info_dialog->window);
   gdk_window_raise(ptr->pane_info_dialog->window);
   }

void about_activate(GtkMenuItem * menuitem, gpointer user_data)
{
   GtkWidget *about_dialog;

   about_dialog = create_viewnup_about();
   gtk_widget_show(about_dialog);
   }

void manual_activate(GtkMenuItem * menuitem, gpointer user_data)
{

   }

/* top of pane buttons       */
/* pane buttons and controls */
void left_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();
   Pane_info curr_pane = (Pane_info) user_data;
   Pane_info prev_pane;
   int      c = 0;

   if(curr_pane->position != 0){
      /* get the id of the "prev" pane */
      while(((Pane_info) g_ptr_array_index(ptr->panes, c))->position + 1 !=
            curr_pane->position){
         c++;
         }
      prev_pane = g_ptr_array_index(ptr->panes, c);
      curr_pane->position--;
      prev_pane->position++;

      /* rearrange em */
      gtk_box_reorder_child(GTK_BOX(ptr->pane_hbody),
                            GTK_WIDGET(curr_pane->hbox_pane), curr_pane->position);
      gtk_box_reorder_child(GTK_BOX(ptr->pane_hbody),
                            GTK_WIDGET(prev_pane->hbox_pane), prev_pane->position);
      }
   }

void right_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();
   Pane_info curr_pane = (Pane_info) user_data;
   Pane_info next_pane;
   int      c = 0;

   if(curr_pane->position != (ptr->panes->len - 1)){
      /* get the id of the "next" pane */
      while(((Pane_info) g_ptr_array_index(ptr->panes, c))->position - 1 !=
            curr_pane->position){
         c++;
         }
      next_pane = g_ptr_array_index(ptr->panes, c);
      curr_pane->position++;
      next_pane->position--;

      /* rearrange em */
      gtk_box_reorder_child(GTK_BOX(ptr->pane_hbody),
                            GTK_WIDGET(curr_pane->hbox_pane), curr_pane->position);
      gtk_box_reorder_child(GTK_BOX(ptr->pane_hbody),
                            GTK_WIDGET(next_pane->hbox_pane), next_pane->position);
      }
   }

void sync_button_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;
   Main_info *ptr = get_main_ptr();

   pane->synch = get_next_synch(ptr, pane->synch);
   update_synch_button(ptr, pane);
   }

/* voxel values */
void vox_value_button_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   /* Never change a merge pane unless you want a boom */
   if(pane->merge == TRUE){
      return;
      }

   switch (pane->value_type){
   default:
   case VALUE_NONE:
      pane->value_type = VALUE_REAL;
      gtk_button_set_label(GTK_BUTTON(button), "real:");
      break;
   case VALUE_REAL:
      pane->value_type = VALUE_VOXEL;
      gtk_button_set_label(GTK_BUTTON(button), "voxel:");
      break;
   case VALUE_VOXEL:
      pane->value_type = VALUE_RGBA;
      gtk_button_set_label(GTK_BUTTON(button), "rgba:");
      break;
   case VALUE_RGBA:
      pane->value_type = VALUE_NONE;
      gtk_button_set_label(GTK_BUTTON(button), "none");
      break;
      }

   if(pane->value_type == VALUE_NONE){
      gtk_entry_set_text(GTK_ENTRY(pane->vox_value), " --- ");
      }
   else {
      update_voxel_value(pane);
      }
   }

/* Coordinates stuff */
/* co-ordinate button and spinbuttons */
void coord_button_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   if(pane->use_vox_coords){
      gtk_button_set_label(GTK_BUTTON(button), "W:");

      pane->use_vox_coords = FALSE;
      }
   else {
      gtk_button_set_label(GTK_BUTTON(button), "V:");
      pane->use_vox_coords = TRUE;
      }

   /* change the coord values to suit */
   show_hide_coord_spins(pane);
   }

void vx_coord_changed(GtkEditable * editable, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   pane->v[0] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_world_from_voxel(pane);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_wx), pane->w[0]);
   }

void vy_coord_changed(GtkEditable * editable, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   pane->v[1] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_world_from_voxel(pane);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_wy), pane->w[1]);
   }

void vz_coord_changed(GtkEditable * editable, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   pane->v[2] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_world_from_voxel(pane);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_wz), pane->w[2]);
   }

void vt_coord_changed(GtkEditable * editable, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   pane->v[3] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_world_from_voxel(pane);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_wt), pane->w[3]);
   }

void wx_coord_changed(GtkEditable * editable, gpointer user_data)
{
   int      c;
   Pane_info pane = (Pane_info) user_data;
   View_info view;

   pane->w[0] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_voxel_from_world(pane);

   /* update the voxel spins but block callbacks first */
   gtk_signal_handler_block_by_data(GTK_OBJECT(pane->coord_vx), user_data);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_vx), pane->v[0]);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pane->coord_vx), user_data);

   for(c = 0; c < pane->views->len; c++){
      view = g_ptr_array_index(pane->views, c);
      if(view->type == SAGITTAL){
         view->reload_image = TRUE;
         }
      redraw_pane_view(view);
      }

   if(pane->value_type != VALUE_NONE){
      update_voxel_value(pane);
      }
   }

void wy_coord_changed(GtkEditable * editable, gpointer user_data)
{
   int      c;
   Pane_info pane = (Pane_info) user_data;
   View_info view;

   pane->w[1] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_voxel_from_world(pane);

   /* update the voxel spins but block callbacks first */
   gtk_signal_handler_block_by_data(GTK_OBJECT(pane->coord_vy), user_data);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_vy), pane->v[1]);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pane->coord_vy), user_data);

   for(c = 0; c < pane->views->len; c++){
      view = g_ptr_array_index(pane->views, c);
      if(view->type == CORONAL){
         view->reload_image = TRUE;
         }
      redraw_pane_view(view);
      }

   if(pane->value_type != VALUE_NONE){
      update_voxel_value(pane);
      }
   }

void wz_coord_changed(GtkEditable * editable, gpointer user_data)
{
   int      c;
   Pane_info pane = (Pane_info) user_data;
   View_info view;

   pane->w[2] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_voxel_from_world(pane);

   /* update the voxel spins but block callbacks first */
   gtk_signal_handler_block_by_data(GTK_OBJECT(pane->coord_vz), user_data);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_vz), pane->v[2]);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pane->coord_vz), user_data);

   for(c = 0; c < pane->views->len; c++){
      view = g_ptr_array_index(pane->views, c);
      if(view->type == TRANSVERSE){
         view->reload_image = TRUE;
         }
      redraw_pane_view(view);
      }

   if(pane->value_type != VALUE_NONE){
      update_voxel_value(pane);
      }
   }

void wt_coord_changed(GtkEditable * editable, gpointer user_data)
{
   int      c;
   Pane_info pane = (Pane_info) user_data;
   View_info view;

   pane->w[3] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_voxel_from_world(pane);

   /* update the voxel spins but block callbacks first */
   gtk_signal_handler_block_by_data(GTK_OBJECT(pane->coord_vt), user_data);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(pane->coord_vt), pane->v[3]);
   gtk_signal_handler_unblock_by_data(GTK_OBJECT(pane->coord_vt), user_data);

   for(c = 0; c < pane->views->len; c++){
      view = g_ptr_array_index(pane->views, c);

      view->reload_image = TRUE;
      redraw_pane_view(view);
      }

   if(pane->value_type != VALUE_NONE){
      update_voxel_value(pane);
      }
   }

/* open file stuff          */
/* pane open file callbacks */
void file_open_entry_activate(GtkEntry * entry, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();
   Pane_info pane = (Pane_info) user_data;
   View_info view;
   gchar   *text;
   gchar    buf[128];
   int      c;

   text = g_strdup(gtk_entry_get_text(entry));

   if(text == NULL){
      g_snprintf(buf, 128, _("open file passed NULL pointer!"));
      push_statusbar(ptr, buf, 1);
      return;
      }

   if(!g_file_test(text, G_FILE_TEST_EXISTS)){
      g_snprintf(buf, 128, _("'%s' does not exist"), text);
      push_statusbar(ptr, buf, 1);
      return;
      }

   /* start to load the file to the pane */
   if(!start_open_minc_file_to_pane(pane, text)){
      g_snprintf(buf, 128, _("'%s' is either broken or not a MINC file"), text);
      push_statusbar(ptr, buf, 1);
      return;
      }

   /* set up the panes name and title */
   g_string_sprintf(pane->file_name, "%s", text);
   g_string_sprintf(pane->file_basename, "%s", g_basename(text));
   update_pane_and_filename(pane);

   /* set up coordinate ranges and values */
   init_coord_ranges(pane);
   for(c = 0; c < MAX_VNUP_DIMS; c++){
      pane->v[c] = (double)(pane->sizes[c] - 1) / 2.0;
      }

   update_world_from_voxel(pane);
   for(c = 0; c < MAX_VNUP_DIMS; c++){
      pane->cov[c] = pane->w[c];
      }

   update_coord_values(pane);

   /* set up it's colormap with the new files range */
   if(pane->cmap_ptr == NULL){
      pane->cmap_ptr = ptr->lookup_tables[0];
      }
   if(!load_table_to_pane(pane)){
      g_snprintf(buf, 128, _("Ack! couldn't load cmap for %s"), pane->file_basename->str);
      push_statusbar(ptr, buf, 1);
      return;
      }
   update_ranges(pane);

   /* associate the pane with a synch */
   update_synch_button(ptr, pane);

   /* refresh OpenGL windows and reload texmaps */
   pane->draw = TRUE;
   for(c = 0; c < pane->views->len; c++){
      view = g_ptr_array_index(pane->views, c);
      view->reload_texmap = TRUE;
      view->reload_image = TRUE;
      view->reload_cmap = TRUE;
      view->refresh_view = TRUE;
      }

   /* continue to load it updating as we go if we need to.. */
   if(pane->perc_input != 1.0){
      while(continue_open_minc_file_to_pane(pane)){

         /* let GTK+ do a few things */
         while(g_main_iteration(FALSE));

         for(c = 0; c < pane->views->len; c++){
            view = g_ptr_array_index(pane->views, c);
            view->reload_image = TRUE;
            }
         gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ptr->progressbar),
                                       pane->perc_input);

         if(!pane->draw_fast){
            redraw_pane_views(pane);
            }
         }
      }

   /* finish up */
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ptr->progressbar), 1.0);
   update_voxel_value(pane);
   redraw_pane_views(pane);

   g_snprintf(buf, 128, _("%s successfully loaded"), pane->file_basename->str);
   push_statusbar(ptr, buf, 1);

   /* update the merge combos */
   update_merge_combos(ptr);

   free(text);
   }

/* cmap callbacks    */
/* range spinbuttons */
void range_min_val_changed(GtkEditable * editable, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;
   int      c;

   pane->pane_min_value = (double)gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));

   load_table_to_pane(pane);
   for(c = 0; c < pane->views->len; c++){
      ((View_info) g_ptr_array_index(pane->views, c))->reload_cmap = TRUE;
      }
   redraw_pane_views(pane);

   if(pane->value_type == VALUE_RGBA){
      update_voxel_value(pane);
      }
   }

void range_max_val_changed(GtkEditable * editable, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;
   int      c;

   pane->pane_max_value = (double)gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));

   load_table_to_pane(pane);
   for(c = 0; c < pane->views->len; c++){
      ((View_info) g_ptr_array_index(pane->views, c))->reload_cmap = TRUE;
      }
   redraw_pane_views(pane);

   if(pane->value_type == VALUE_RGBA){
      update_voxel_value(pane);
      }
   }

/* grey colour map button */
void cmap_grey_radio_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   gtk_entry_set_text(GTK_ENTRY(pane->cmap_combo_entry), "grey");
   }

/* spect colour map button */
void cmap_spect_radio_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   gtk_entry_set_text(GTK_ENTRY(pane->cmap_combo_entry), "spectral");
   }

/* hot colour map button */
void cmap_hot_radio_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   gtk_entry_set_text(GTK_ENTRY(pane->cmap_combo_entry), "hotmetal");
   }

/* bluered colour map button */
void cmap_bluered_radio_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;

   gtk_entry_set_text(GTK_ENTRY(pane->cmap_combo_entry), "bluered");
   }

/* colour map combo */
void cmap_combo_entry_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();
   Pane_info pane = (Pane_info) user_data;
   Lookup_Table *cmap;

   cmap = get_cmap_ptr(gtk_entry_get_text(GTK_ENTRY(editable)), ptr);
   if(cmap != NULL && pane->cmap_ptr != cmap){
      int      c;

      pane->cmap_ptr = cmap;
      load_table_to_pane(pane);
      for(c = 0; c < pane->views->len; c++){
         ((View_info) g_ptr_array_index(pane->views, c))->reload_cmap = TRUE;
         }
      redraw_pane_views(pane);

      if(pane->value_type == VALUE_RGBA){
         update_voxel_value(pane);
         }
      }
   }

/* pane focus and resizing callbacks */
void hbox_pane_set_focus_child(GtkContainer * container, GtkWidget * widget,
                               gpointer user_data)
{
   make_pane_active((Pane_info) user_data);
   }

void vsep_eventbox_realize(GtkWidget * widget, gpointer user_data)
{
   GdkCursor *curs = gdk_cursor_new(GDK_LEFT_PTR);

   gdk_window_set_cursor(GTK_WIDGET(widget)->window, curs);

   g_print("Set up cursor\n");
   }

gboolean
vsep_eventbox_button_press_event(GtkWidget * widget, GdkEventButton * event,
                                 gpointer user_data)
{
   gdk_pointer_grab(GTK_WIDGET(widget)->window, FALSE,
                    GDK_POINTER_MOTION_HINT_MASK |
                    GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_RELEASE_MASK, NULL, NULL,
                    event->time);
   return TRUE;
   }

gboolean
vsep_eventbox_button_release_event(GtkWidget * widget, GdkEventButton * event,
                                   gpointer user_data)
{
   gdk_pointer_ungrab(event->time);
   return TRUE;
   }

gboolean
vsep_eventbox_motion_notify_event(GtkWidget * widget, GdkEventMotion * event,
                                  gpointer user_data)
{
   Pane_info pane = (Pane_info) user_data;
   GdkModifierType state;
   int      x, y;

   if(event->is_hint){
      gdk_window_get_pointer(event->window, &x, &y, &state);
      }
   else {
      x = event->x;
      y = event->y;
      state = event->state;
      }

   if(state){
      gtk_widget_set_usize(pane->vbox_pane,
                           GTK_WIDGET(pane->vbox_pane)->allocation.width + x, -2);
      }

   return TRUE;
   }

/* DIALOG CALLBACKS */
/* cmap filesel callback */
void cmap_file_ok_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();
   char    *filename;
   Lookup_Table *lut;
   gchar    buf[128];

//   int           c;

   filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(user_data));

   /* read in the table */
   lut = read_lookup_table(filename);

   if(lut == NULL){
      g_snprintf(buf, 128, _("Trouble reading colourmap: %s"), filename);
      push_statusbar(ptr, buf, 1);
      }

   else {
      ptr->c_pane->cmap_ptr = lut;

      // here we should just update the cmap combo 

      /* insert the table into pane cmap pop-ups here 
         if(store_table(lut, ptr)){
         ptr->c_pane->cmap_ptr = lut;
         load_table_to_pane(ptr->c_pane);
         for(c=0; c<ptr->c_pane->n_views; c++){
         ptr->c_pane->views[c]->reload_cmap = TRUE;
            }
         redraw_pane_views(pane);
            }
       */
      }

   gtk_widget_destroy(GTK_WIDGET(user_data));
   }

/* scheme filesel callback */
void scheme_ok_button_clicked(GtkButton * button, gpointer user_data)
{
   g_print("%s\n", gtk_file_selection_get_filename(GTK_FILE_SELECTION(user_data)));
   gtk_widget_destroy(GTK_WIDGET(user_data));
   }

/* Synch dialog       */
void synch_coord_changed(GtkEditable * editable, gpointer user_data)
{
   Synch_info *synch = (Synch_info *) user_data;

   if(synch->virgin){
      synch->virgin = FALSE;
      }
   }

void synch_remove_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();
   Synch_info *synch = (Synch_info *) user_data;
   Pane_info pane;

   int      c;

   /* First remove the widgets to save unref problems */
   gtk_widget_ref(GTK_WIDGET(ptr->synch_table));
   gtk_container_remove(GTK_CONTAINER(ptr->synch_dialog), GTK_WIDGET(ptr->synch_table));

   remove_synch(ptr, synch);

   ptr->synch_table = create_synch_table(ptr);
   gtk_container_add(GTK_CONTAINER(ptr->synch_dialog), GTK_WIDGET(ptr->synch_table));

   /* Then change panes that refer to it (and update all others) */
   for(c = 0; c < ptr->panes->len; c++){
      pane = g_ptr_array_index(ptr->panes, c);
      if(pane->synch == synch){
         pane->synch = ptr->synchs[0];
         }

      /* update synch pointers */
      update_synch_button(ptr, pane);
      }
   }

void synch_add_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();

   gtk_widget_ref(GTK_WIDGET(ptr->synch_table));
   gtk_container_remove(GTK_CONTAINER(ptr->synch_dialog), GTK_WIDGET(ptr->synch_table));

   ptr->synchs[ptr->n_synchs] = init_synch_info();
   ptr->synchs[ptr->n_synchs]->idx = ptr->n_synchs;
   ptr->n_synchs++;

   ptr->synch_table = create_synch_table(ptr);
   gtk_container_add(GTK_CONTAINER(ptr->synch_dialog), GTK_WIDGET(ptr->synch_table));
   }

gboolean synch_dialog_delete_event(GtkWidget * widget, GdkEvent * event,
                                   gpointer user_data)
{
   gtk_widget_hide(GTK_WIDGET(widget));
   return TRUE;
   }

/* Pane info dialog */
gboolean pane_info_dialog_delete_event(GtkWidget * widget, GdkEvent * event,
                                       gpointer user_data)
{
   gtk_widget_hide(GTK_WIDGET(widget));
   return TRUE;
   }

void pi_pane_add_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane;

   pane = add_pane(ptr, FALSE, NULL, FALSE);

   make_pane_active(pane);
   make_view_active(pane, pane->views->pdata[0]);
   }

void pi_pane_clone_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane;

   pane = add_pane(ptr, TRUE, ptr->c_pane, FALSE);

   make_pane_active(pane);
   make_view_active(pane, pane->views->pdata[0]);
   }

void pi_pane_close_button_clicked(GtkButton * button, gpointer user_data)
{
//   Main_info *ptr = (Main_info *) user_data;

   }

/* buttons to reset coordinates */
void pi_voxel_button_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = ((Main_info *) user_data)->c_pane;

   pane->w[0] = pane->cov[0];
   pane->w[1] = pane->cov[1];
   pane->w[2] = pane->cov[2];
   pane->w[3] = pane->cov[3];
   update_coord_values(pane);
   }

void pi_world_button_clicked(GtkButton * button, gpointer user_data)
{
   Pane_info pane = ((Main_info *) user_data)->c_pane;

   pane->w[0] = 0.0;
   pane->w[1] = 0.0;
   pane->w[2] = 0.0;
   pane->w[3] = 0.0;
   update_coord_values(pane);
   }

/* pane togglebuttons */
void pi_linear_interp_checkbutton_toggled(GtkToggleButton * togglebutton,
                                          gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   ptr->c_pane->linear_interp =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
   redraw_pane_views(ptr->c_pane);
   }

void pi_crosshair_checkbutton_toggled(GtkToggleButton * togglebutton, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   ptr->c_pane->crosshair = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
   redraw_pane_views(ptr->c_pane);
   }

void pi_perspective_checkbutton_toggled(GtkToggleButton * togglebutton,
                                        gpointer user_data)
{
   Pane_info pane = ((Main_info *) user_data)->c_pane;
   View_info view;
   int      c;

   pane->perspective = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));

   for(c = 0; c < pane->views->len; c++){
      view = g_ptr_array_index(pane->views, c);

      view->GLscalefac *= (pane->perspective) ? 2.0 : 0.5;
      view->refresh_view = TRUE;
      redraw_pane_view(view);
      }
   }

void pi_vector_checkbutton_toggled(GtkToggleButton * togglebutton, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   ptr->c_pane->vector = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));

   if(ptr->c_pane->vector){
      gtk_widget_show(GTK_WIDGET(ptr->pane_dialog->vector_frame));
      gtk_widget_hide(GTK_WIDGET(ptr->pane_dialog->cmap_frame));
      }
   else {
      gtk_widget_show(GTK_WIDGET(ptr->pane_dialog->cmap_frame));
      gtk_widget_hide(GTK_WIDGET(ptr->pane_dialog->vector_frame));
      }

   redraw_pane_views(ptr->c_pane);
   }

void pi_slicebox_checkbutton_toggled(GtkToggleButton * togglebutton, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   ptr->c_pane->slice_box = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
   redraw_pane_views(ptr->c_pane);
   }

void pi_bbox_checkbutton_toggled(GtkToggleButton * togglebutton, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   ptr->c_pane->bounding_box =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
   redraw_pane_views(ptr->c_pane);
   }

void pi_crosshair_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   ptr->c_pane->crosshair_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   redraw_pane_views(ptr->c_pane);
   }

void pi_view_type_combo_entry_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;
   gchar   *text;

   text = gtk_entry_get_text(GTK_ENTRY(editable));

   if(strcmp(text, "transverse") == 0){
      view->type = TRANSVERSE;
      };
   if(strcmp(text, "sagittal") == 0){
      view->type = SAGITTAL;
      };
   if(strcmp(text, "coronal") == 0){
      view->type = CORONAL;
      };

   view->reload_texmap = TRUE;
   view->reload_image = TRUE;
   view->reload_cmap = TRUE;
   view->refresh_view = TRUE;
   view->recalc_view = TRUE;

   init_tilt_vec(view);
   init_view_idx(view);
   init_rot_vec(view);
   redraw_pane_view(view);
   }

void pi_view_add_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   View_info view = init_view_info(pane->c_view->type);

   g_ptr_array_add(pane->views, view);
   create_view_widgets(ptr, pane, view);
   }

void pi_view_clone_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;
   View_info view;

   g_warning("Clone view not implemented yet! just adding...\n");

   view = init_view_info(pane->c_view->type);
   g_ptr_array_add(pane->views, view);
   create_view_widgets(ptr, pane, view);
   }

void pi_view_close_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   /* destroy the widgets, then the view structure */
   if(g_ptr_array_remove(pane->views, pane->c_view)){

      delete_view_widgets(pane, pane->c_view);

      free_view_info(pane->c_view);
      pane->c_view = NULL;
      make_view_active(pane, g_ptr_array_index(pane->views, 0));
      }
   else {
      g_warning(_("Glark! I canna remove the view capt'n! (This is bad)\n"));
      }
   }

/* buttons to reset spins to "normal" values */
void pi_scale_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;
   View_info v_ptr;
   int      c;

   update_pi_scale(ptr, 1.0);

   if(pane->link_scales){
      for(c = 0; c < pane->views->len; c++){
         v_ptr = g_ptr_array_index(pane->views, c);
         if(!v_ptr->lock_scale){
            v_ptr->GLscalefac = 1.0;
            v_ptr->refresh_view = TRUE;
            redraw_pane_view(v_ptr);
            }
         }
      }
   }

void pi_trans_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   update_pi_trans(ptr, 0.0, 0.0, 0.0);
   }

void pi_rot_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   init_rot_vec(view);
   update_pi_rots(ptr, view->rot_vec[0], view->rot_vec[1], view->rot_vec[2],
                  view->rot_phi);
   }

void pi_tilt_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->tilt = FALSE;
   init_tilt_vec(view);
   update_pi_tilts(ptr, view->tilt_w[0], view->tilt_w[1], view->tilt_w[2]);
   }

/* link buttons */
void pi_scale_link_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->link_scales =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_trans_link_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->link_trans =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_rot_link_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->link_rots =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_tilt_link_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->link_tilts =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

/* scale, trans, rot and tilt spins */
void pi_scale_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->GLscalefac = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   view->refresh_view = TRUE;
   redraw_pane_view(view);
   }

void pi_trans_x_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->GLtrans_x = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   view->refresh_view = TRUE;
   redraw_pane_view(view);
   }

void pi_trans_y_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->GLtrans_y = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   view->refresh_view = TRUE;
   redraw_pane_view(view);
   }

void pi_trans_z_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->GLtrans_z = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   view->refresh_view = TRUE;
   redraw_pane_view(view);
   }

void pi_rot_x_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->rot_vec[0] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   axis_to_quat(view->rot_vec, view->rot_phi, view->rot_quat);
   view->refresh_view = TRUE;
   redraw_pane_view(view);
   }

void pi_rot_y_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->rot_vec[1] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   axis_to_quat(view->rot_vec, view->rot_phi, view->rot_quat);
   view->refresh_view = TRUE;
   redraw_pane_view(view);
   }

void pi_rot_z_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->rot_vec[2] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   axis_to_quat(view->rot_vec, view->rot_phi, view->rot_quat);
   view->refresh_view = TRUE;
   redraw_pane_view(view);
   }

void pi_rot_phi_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->rot_phi = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable)) * M_PI / 180.0;
   axis_to_quat(view->rot_vec, view->rot_phi, view->rot_quat);
   view->refresh_view = TRUE;
   redraw_pane_view(view);
   }

void pi_tilt_x_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->tilt_w[0] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_slice_tilt(ptr->c_pane, view);
   }

void pi_tilt_y_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->tilt_w[1] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_slice_tilt(ptr->c_pane, view);
   }

void pi_tilt_z_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   View_info view = ptr->c_pane->c_view;

   view->tilt_w[2] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   update_slice_tilt(ptr->c_pane, view);
   }

/* lock buttons */
void pi_scale_lock_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_scale =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_trans_lock_x_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_x_trans =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_trans_lock_y_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_y_trans =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_trans_lock_z_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_z_trans =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_rot_lock_x_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_x_rot =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_rot_lock_y_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_y_rot =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_rot_lock_z_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_z_rot =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_rot_lock_phi_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_phi_rot =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_tilt_lock_x_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_x_tilt =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_tilt_lock_y_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_y_tilt =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

void pi_tilt_lock_z_button_clicked(GtkButton * button, gpointer user_data)
{
   ((Main_info *) user_data)->c_pane->c_view->lock_z_tilt =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
   }

/* cmap */
void pi_cmap_grey_radiobutton_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   gtk_entry_set_text(GTK_ENTRY(ptr->pane_dialog->cmap_combo_entry), "grey");
   }

void pi_cmap_hot_radiobutton_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   gtk_entry_set_text(GTK_ENTRY(ptr->pane_dialog->cmap_combo_entry), "hotmetal");
   }

void pi_cmap_spect_radiobutton_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   gtk_entry_set_text(GTK_ENTRY(ptr->pane_dialog->cmap_combo_entry), "spectral");
   }

void pi_cmap_bluered_radiobutton_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   gtk_entry_set_text(GTK_ENTRY(ptr->pane_dialog->cmap_combo_entry), "bluered");
   }

void pi_cmap_combo_entry_changed(GtkEditable * editable, gpointer user_data)
{
   gchar   *text;
   Main_info *ptr = (Main_info *) user_data;

   text = gtk_entry_get_text(GTK_ENTRY(editable));
   gtk_entry_set_text(GTK_ENTRY(ptr->c_pane->cmap_combo_entry), text);
   }

void pi_cmap_reload_button_clicked(GtkButton * button, gpointer user_data)
{
//   Main_info *ptr = (Main_info *) user_data;

   }

void pi_cmap_alpha_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->alpha_thresh = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   redraw_pane_views(pane);
   }

/* vector callbacks */
void pi_vector_alpha_mult_euc_checkbutton_toggled(GtkToggleButton * togglebutton,
                                                  gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->mult_vect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
   redraw_pane_views(pane);
   }

void pi_vector_mult_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   gtk_spin_button_set_value(GTK_SPIN_BUTTON(ptr->pane_dialog->vector_mult_spinbutton),
                             1.0);
   }

void pi_alpha_mult_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;

   gtk_spin_button_set_value(GTK_SPIN_BUTTON
                             (ptr->pane_dialog->vector_alpha_mult_spinbutton), 1.0);
   }

void pi_vector_mult_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->vect_mult = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   redraw_pane_views(pane);
   }

void pi_vector_alpha_mult_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->vect_alph_mult = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   redraw_pane_views(pane);
   }

void pi_vector_colour_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->vect_col[0] = 0.0;
   pane->vect_col[1] = 0.5;
   pane->vect_col[2] = 1.0;
   pane->vect_col[3] = 1.0;
   redraw_pane_views(pane);
   }

void pi_point_color_button_clicked(GtkButton * button, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->vect_point_col[0] = 0.0;
   pane->vect_point_col[1] = 1.8;
   pane->vect_point_col[2] = 0.9;
   pane->vect_point_col[3] = 0.8;
   redraw_pane_views(pane);
   }

/* colour pickers */
void pi_vector_point_colourpicker_color_changed(GtkColorSelection * colorselection,
                                                gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;
   GdkColor col;
   guint16  alpha;

   gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colorselection), &col);
   alpha = gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colorselection));

   pane->vect_point_col[0] = (double)col.red / 65535;
   pane->vect_point_col[1] = (double)col.green / 65535;
   pane->vect_point_col[2] = (double)col.blue / 65535;
   pane->vect_point_col[3] = (double)alpha / 65535;
   redraw_pane_views(pane);
   }

void pi_vector_colourpicker_color_changed(GtkColorSelection * colorselection,
                                          gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;
   GdkColor col;
   guint16  alpha;

   gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colorselection), &col);
   alpha = gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colorselection));

   pane->vect_point_col[0] = (double)col.red / 65535;
   pane->vect_point_col[1] = (double)col.green / 65535;
   pane->vect_point_col[2] = (double)col.blue / 65535;
   pane->vect_point_col[3] = (double)alpha / 65535;
   redraw_pane_views(pane);
   }

void pi_vector_points_checkbutton_toggled(GtkToggleButton * togglebutton,
                                          gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->vect_points = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
   redraw_pane_views(pane);
   }

void pi_vector_max_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->vect_ceil = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   redraw_pane_views(pane);
   }

void pi_vector_min_spinbutton_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->vect_floor = gtk_spin_button_get_value(GTK_SPIN_BUTTON(editable));
   redraw_pane_views(pane);
   }

void pi_vector_rgb_togglebutton_toggled(GtkToggleButton * togglebutton,
                                        gpointer user_data)
{
   Main_info *ptr = (Main_info *) user_data;
   Pane_info pane = ptr->c_pane;

   pane->RGB_vect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
   redraw_pane_views(pane);
   }

/* merge frame callbacks */
void pi_merge_combo_entry_changed(GtkEditable * editable, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();
   gint     merge_number = GPOINTER_TO_INT(user_data);
   gchar   *text = gtk_entry_get_text(GTK_ENTRY(editable));
   Pane_info pane;
   int      c;

   /* find the appropriate pane */
   if(strcmp(text, NO_FILE_STR) == 0){
      ptr->c_pane->merge_panes[merge_number] = NULL;
      }
   else {
      c = 0;
      pane = g_ptr_array_index(ptr->panes, c);
      while((strcmp(text, pane->file_basename->str) != 0) && (c < ptr->panes->len)){

         pane = g_ptr_array_index(ptr->panes, c);
         c++;
         }

      ptr->c_pane->merge_panes[merge_number] = pane;
      }

   calc_merge_extents(ptr->c_pane);
   }

void pi_merge_coeff_value_changed(GtkAdjustment * adjustment, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();
   Pane_info pane = ptr->c_pane;
   gint     merge_number = GPOINTER_TO_INT(user_data);
   double   prev_value, new_value;
   int      c, num_links = 0;

   /* first change the current value */
   prev_value = pane->merge_coeff[merge_number];
   new_value = (double)(GTK_ADJUSTMENT(adjustment)->value);
   pane->merge_coeff[merge_number] = new_value;

   /* count the number of links */
   for(c = 0; c < NUM_MERGE_COEFF; c++){
      if(c != merge_number && pane->merge_link[c]){
         num_links++;
         }
      }

   /* now adjust the other co-efficients */
   /* This "algorithm" is not exact but with only two passes (3 required) */
   /*   However, IMHO it's close enough for our intents and purposes      */
   /*   The total will not always exactly equal 100                       */
   if(num_links > 0){
      for(c = 0; c < NUM_MERGE_COEFF; c++){
         if(c != merge_number && pane->merge_link[c]){

            /* figure out the new co-efficient */
            pane->merge_coeff[c] += (prev_value - new_value) / (double)num_links;

            /* clamp it, (here's where we get inaccurate and introduce errors */
            if(pane->merge_coeff[c] < 0.0){
               pane->merge_coeff[c] = 0.0;
               }
            if(pane->merge_coeff[c] > 100.0){
               pane->merge_coeff[c] = 100.0;
               }

            /* then set the appropriate adjustment */
            gtk_signal_handler_block_by_data(GTK_OBJECT
                                             (ptr->pane_dialog->merge_coeff_adj[c]),
                                             GINT_TO_POINTER(c));
            gtk_adjustment_set_value(GTK_ADJUSTMENT(ptr->pane_dialog->merge_coeff_adj[c]),
                                     pane->merge_coeff[c]);
            gtk_signal_handler_unblock_by_data(GTK_OBJECT
                                               (ptr->pane_dialog->merge_coeff_adj[c]),
                                               GINT_TO_POINTER(c));
            }
         }
      }

   }

void pi_merge_link_button_toggled(GtkToggleButton * togglebutton, gpointer user_data)
{
   Main_info *ptr = get_main_ptr();

   ptr->c_pane->merge_link[GPOINTER_TO_INT(user_data)] =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));
   }
