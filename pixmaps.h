/* pixmaps.h */

#ifndef PIXMAPS_H
#define PIXMAPS_H

#define CHAIN_V_PIXMAP      1
#define LOCKED_PIXMAP       2
#define SLIDE_PIXMAP        3
#define CLOSE_PIXMAP        4
#define ADD_PIXMAP          5
#define CLONE_PIXMAP        6

#define TRANSLATION_PIXMAP  10
#define ROTATION_PIXMAP     11
#define SCALE_PIXMAP        12
#define TILT_PIXMAP         13

/* toolbar pixmap id's */
#define ORTHOGRAPHIC_PIXMAP 100
#define PERSPECTIVE_PIXMAP  101
#define VOLUME_PIXMAP       102

#define ADD_PANE_PIXMAP             103
#define CLONE_PANE_PIXMAP           104
#define CLOSE_PANE_PIXMAP           105
#define ADD_VIEW_PIXMAP             106
#define ADD_TRANSVERSE_VIEW_PIXMAP  107
#define ADD_SAGITTAL_VIEW_PIXMAP    108
#define ADD_CORONAL_VIEW_PIXMAP     109
#define CLONE_VIEW_PIXMAP           110
#define CLOSE_VIEW_PIXMAP           111



#include "globals.h"

GtkWidget *create_pixmap(GtkWidget *widget, int pixmap_id);

#endif
