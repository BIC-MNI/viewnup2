/* callbacks.h */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "globals.h"


/* convenience functions */
void     init_highlight_styles(Main_info * ptr);
void     update_coord_values(Pane_info pane);
void     make_pane_active(Pane_info pane);
void     make_view_active(Pane_info pane, View_info view);
Pane_info add_pane(Main_info * ptr, int clone, Pane_info clone_pane, int merge);
void     push_statusbar(Main_info * ptr, char *buf, int highlight);
void     update_synch_button(Main_info * ptr, Pane_info pane);

/* pane_info dialog convenience functions */
void     update_pi_scale(Main_info * ptr, double scalefac);
void     update_pi_trans(Main_info * ptr, double x, double y, double z);
void     update_pi_rots(Main_info * ptr, double x, double y, double z, double phi);
void     update_pi_tilts(Main_info * ptr, double x, double y, double z);

/* menu callbacks */
void     about_activate(GtkMenuItem * menuitem, gpointer user_data);
void     dump_info_activate(GtkMenuItem * menuitem, gpointer user_data);
void     edit_activate(GtkMenuItem * menuitem, gpointer user_data);
void     exit_activate(GtkMenuItem * menuitem, gpointer user_data);
void     intensity_profiles_activate(GtkMenuItem * menuitem, gpointer user_data);
void     manual_activate(GtkMenuItem * menuitem, gpointer user_data);
void     open_cmap_activate(GtkMenuItem * menuitem, gpointer user_data);
void     open_scheme_activate(GtkMenuItem * menuitem, gpointer user_data);
void     pane_info_activate(GtkMenuItem * menuitem, gpointer user_data);
void     print_activate(GtkMenuItem * menuitem, gpointer user_data);
void     save_pane_activate(GtkMenuItem * menuitem, gpointer user_data);
void     save_view_activate(GtkMenuItem * menuitem, gpointer user_data);
void     save_window_activate(GtkMenuItem * menuitem, gpointer user_data);
void     synch_activate(GtkMenuItem * menuitem, gpointer user_data);

/* pane callbacks */
void     left_button_clicked(GtkButton * button, gpointer user_data);
void     right_button_clicked(GtkButton * button, gpointer user_data);
void     sync_button_clicked(GtkButton * button, gpointer user_data);
void     coord_button_clicked(GtkButton * button, gpointer user_data);
void     vox_value_button_clicked(GtkButton * button, gpointer user_data);
void     roi_value_button_clicked(GtkButton * button, gpointer user_data);
void     vx_coord_changed(GtkEditable *editable, gpointer user_data);
void     vy_coord_changed(GtkEditable *editable, gpointer user_data);
void     vz_coord_changed(GtkEditable *editable, gpointer user_data);
void     vt_coord_changed(GtkEditable *editable, gpointer user_data);
void     wx_coord_changed(GtkEditable *editable, gpointer user_data);
void     wy_coord_changed(GtkEditable *editable, gpointer user_data);
void     wz_coord_changed(GtkEditable *editable, gpointer user_data);
void     wt_coord_changed(GtkEditable *editable, gpointer user_data);
void     range_min_val_changed(GtkEditable *editable, gpointer user_data);
void     range_max_val_changed(GtkEditable *editable, gpointer user_data);
void     cmap_grey_radio_clicked(GtkButton * button, gpointer user_data);
void     cmap_hot_radio_clicked(GtkButton * button, gpointer user_data);
void     cmap_spect_radio_clicked(GtkButton * button, gpointer user_data);
void     cmap_bluered_radio_clicked(GtkButton * button, gpointer user_data);
void     cmap_combo_entry_changed(GtkEditable *editable, gpointer user_data);
void     file_open_entry_activate(GtkEntry *entry, gpointer user_data);

void     hbox_pane_set_focus_child(GtkContainer * container,
                                   GtkWidget *widget, gpointer user_data);
void     vsep_eventbox_realize(GtkWidget *widget, gpointer user_data);
gboolean
vsep_eventbox_button_press_event(GtkWidget *widget,
                                 GdkEventButton * event, gpointer user_data);
gboolean
vsep_eventbox_button_release_event(GtkWidget *widget,
                                   GdkEventButton * event, gpointer user_data);
gboolean
vsep_eventbox_motion_notify_event(GtkWidget *widget,
                                  GdkEventMotion * event, gpointer user_data);


/* open dialog callbacks */
void     cmap_file_ok_button_clicked(GtkButton * button, gpointer user_data);
void     scheme_ok_button_clicked(GtkButton * button, gpointer user_data);

/* synch dialog callbacks */
void     synch_coord_changed(GtkEditable *editable, gpointer user_data);
void     synch_remove_button_clicked(GtkButton * button, gpointer user_data);
void     synch_add_button_clicked(GtkButton * button, gpointer user_data);
gboolean
synch_dialog_delete_event(GtkWidget *widget, GdkEvent * event, gpointer user_data);

/* Pane info dialog callbacks */
gboolean pane_info_dialog_delete_event(GtkWidget *widget,
                                       GdkEvent * event, gpointer user_data);

void     pi_pane_add_button_clicked(GtkButton * button, gpointer user_data);
void     pi_pane_close_button_clicked(GtkButton * button, gpointer user_data);
void     pi_pane_clone_button_clicked(GtkButton * button, gpointer user_data);

void     pi_voxel_button_clicked(GtkButton * button, gpointer user_data);
void     pi_world_button_clicked(GtkButton * button, gpointer user_data);
void     pi_scale_button_clicked(GtkButton * button, gpointer user_data);
void     pi_trans_button_clicked(GtkButton * button, gpointer user_data);
void     pi_rot_button_clicked(GtkButton * button, gpointer user_data);
void     pi_tilt_button_clicked(GtkButton * button, gpointer user_data);

void     pi_scale_link_button_clicked(GtkButton * button, gpointer user_data);
void     pi_trans_link_button_clicked(GtkButton * button, gpointer user_data);
void     pi_rot_link_button_clicked(GtkButton * button, gpointer user_data);
void     pi_tilt_link_button_clicked(GtkButton * button, gpointer user_data);

void     pi_scale_lock_button_clicked(GtkButton * button, gpointer user_data);
void     pi_trans_lock_x_button_clicked(GtkButton * button, gpointer user_data);
void     pi_trans_lock_y_button_clicked(GtkButton * button, gpointer user_data);
void     pi_trans_lock_z_button_clicked(GtkButton * button, gpointer user_data);
void     pi_rot_lock_x_button_clicked(GtkButton * button, gpointer user_data);
void     pi_rot_lock_y_button_clicked(GtkButton * button, gpointer user_data);
void     pi_rot_lock_z_button_clicked(GtkButton * button, gpointer user_data);
void     pi_rot_lock_phi_button_clicked(GtkButton * button, gpointer user_data);
void     pi_tilt_lock_x_button_clicked(GtkButton * button, gpointer user_data);
void     pi_tilt_lock_y_button_clicked(GtkButton * button, gpointer user_data);
void     pi_tilt_lock_z_button_clicked(GtkButton * button, gpointer user_data);

void     pi_view_type_combo_entry_changed(GtkEditable *editable, gpointer user_data);
void     pi_view_add_button_clicked(GtkButton * button, gpointer user_data);
void     pi_view_clone_button_clicked(GtkButton * button, gpointer user_data);
void     pi_view_close_button_clicked(GtkButton * button, gpointer user_data);

void     pi_roi_none_radiobutton_clicked(GtkButton * button, gpointer user_data);
void     pi_roi_cube_radiobutton_clicked(GtkButton * button, gpointer user_data);
void     pi_roi_sphere_radiobutton_clicked(GtkButton * button, gpointer user_data);
void     pi_roi_fwhm_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_roi_x_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_roi_y_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_roi_z_spinbutton_changed(GtkEditable *editable, gpointer user_data);

void     pi_crosshair_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_scale_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_trans_x_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_trans_y_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_trans_z_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_rot_x_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_rot_y_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_rot_z_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_rot_phi_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_tilt_x_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_tilt_y_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_tilt_z_spinbutton_changed(GtkEditable *editable, gpointer user_data);

void     pi_cmap_grey_radiobutton_clicked(GtkButton * button, gpointer user_data);
void     pi_cmap_hot_radiobutton_clicked(GtkButton * button, gpointer user_data);
void     pi_cmap_spect_radiobutton_clicked(GtkButton * button, gpointer user_data);
void     pi_cmap_bluered_radiobutton_clicked(GtkButton * button, gpointer user_data);
void     pi_cmap_reload_button_clicked(GtkButton * button, gpointer user_data);
void     pi_cmap_combo_entry_changed(GtkEditable *editable, gpointer user_data);
void     pi_cmap_alpha_spinbutton_changed(GtkEditable *editable, gpointer user_data);

void     pi_vector_mult_button_clicked(GtkButton * button, gpointer user_data);
void     pi_vector_colour_button_clicked(GtkButton * button, gpointer user_data);
void     pi_point_color_button_clicked(GtkButton * button, gpointer user_data);
void     pi_alpha_mult_button_clicked(GtkButton * button, gpointer user_data);
void     pi_vector_mult_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_vector_alpha_mult_spinbutton_changed
   (GtkEditable *editable, gpointer user_data);
void     pi_vector_max_spinbutton_changed(GtkEditable *editable, gpointer user_data);
void     pi_vector_min_spinbutton_changed(GtkEditable *editable, gpointer user_data);

/* pi color pickers */
void     pi_vector_point_colourpicker_color_changed(GtkColorSelection * colorselection,
                                                    gpointer user_data);
void     pi_vector_colourpicker_color_changed(GtkColorSelection * colorselection,
                                              gpointer user_data);

/* pane toggles */
void     pi_vector_alpha_mult_euc_checkbutton_toggled(GtkToggleButton * togglebutton,
                                                      gpointer user_data);
void     pi_bbox_checkbutton_toggled(GtkToggleButton * togglebutton, gpointer user_data);
void     pi_crosshair_checkbutton_toggled(GtkToggleButton * togglebutton,
                                          gpointer user_data);
void     pi_linear_interp_checkbutton_toggled(GtkToggleButton * togglebutton,
                                              gpointer user_data);
void     pi_perspective_checkbutton_toggled(GtkToggleButton * togglebutton,
                                            gpointer user_data);
void     pi_roi_gauss_checkbutton_toggled(GtkToggleButton * togglebutton,
                                          gpointer user_data);
void     pi_slicebox_checkbutton_toggled(GtkToggleButton * togglebutton,
                                         gpointer user_data);
void     pi_vector_checkbutton_toggled(GtkToggleButton * togglebutton,
                                       gpointer user_data);
void     pi_vector_points_checkbutton_toggled(GtkToggleButton * togglebutton,
                                              gpointer user_data);
void     pi_vector_rgb_togglebutton_toggled(GtkToggleButton * togglebutton,
                                            gpointer user_data);

/* merge callbacks */
void     pi_merge_combo_entry_changed(GtkEditable *editable, gpointer user_data);
void     pi_merge_coeff_value_changed(GtkAdjustment * adjustment, gpointer user_data);
void     pi_merge_link_button_toggled(GtkToggleButton * togglebutton, gpointer user_data);


#endif
