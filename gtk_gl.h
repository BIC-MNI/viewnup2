/* gtk_gl.h */

#ifndef GTK_GL_H
#define GTK_GL_H

#include "globals.h"

/* function prototypes */
GdkGLConfig *configure_gtkgl(void);

GtkWidget* create_glarea(Pane_info pane, View_info view);

void redraw_pane_views (Pane_info pane);
void redraw_pane_view  (View_info view);

void update_slice_tilt(Pane_info pane, View_info view);

void calc_merge_extents(Pane_info merge);

#endif
