/* minc_in_out.h */

#ifndef MINC_IO_H
#define MINC_IO_H

#include "globals.h"
#include "callbacks.h"

int      start_open_minc_file_to_pane(Pane_info pane, char *filename);
int      continue_open_minc_file_to_pane(Pane_info pane);

int      get_minc_image(Pane_info pane, View_info view);
int      get_voxel_value(Pane_info pane);

void     get_vector_components(Pane_info pane, int x, int y, int z,
                               double *dx, double *dy, double *dz);

#endif
