/* minc_in_out.c */

#include "minc_io.h"
#include "lookup_table.h"
#include <volume_io.h>

extern int verbose;

static STRING axis_order_3D[3] = { MIzspace, MIyspace, MIxspace };
static STRING axis_order_4D_time[4] = { MIzspace, MIyspace, MIxspace, MItime };
static STRING axis_order_4D_vector[4] =
   { MIzspace, MIyspace, MIxspace, MIvector_dimension };

int start_open_minc_file_to_pane(Pane_info pane, char *filename)
{
   int      sizes[MAX_VAR_DIMS];
   Real     steps[MAX_VAR_DIMS];
   Real     starts[MAX_VAR_DIMS];
   STRING  *dim_names;
   STRING  *axis_order_ptr;
   minc_input_options options;
   int      c;
   gchar    buf[128];

   if(filename == NULL){
      g_print("Oook!, open file was passed a NULL pointer\n");
      return FALSE;
      }

   /* have to run this *BEFORE* any volume in/out stuff */
   if(get_file_dimension_names(filename, &(pane->n_dims), &dim_names) != OK){
      return FALSE;
      }

   /* find the correct axis_order */
   axis_order_ptr = NULL;
   if(verbose){
      g_print("Dims: %d\n", pane->n_dims);
      }
   if(pane->n_dims == 3){
      if(verbose){
         g_print("3D file\n");
         }
      axis_order_ptr = axis_order_3D;
      }
   else {
      axis_order_ptr = NULL;
      for(c = 0; c < pane->n_dims; c++){
         g_print("Dim[%d] %s\n", c, dim_names[c]);

         if(strcmp(dim_names[c], axis_order_4D_time[3]) == 0){
            if(verbose){
               g_print("time\n");
               }
            axis_order_ptr = axis_order_4D_time;
            }

         if(strcmp(dim_names[c], axis_order_4D_vector[3]) == 0){
            if(verbose){
               g_print("vector\n");
               }
            axis_order_ptr = axis_order_4D_vector;
            }
         }
      }

   /* check that we found one */
   if(axis_order_ptr == NULL){
      g_snprintf(buf, 128, "Unable to find an approriate axis ordering");
      push_statusbar(NULL, buf);
      return FALSE;
      }

   /* setup the input options */
   set_default_minc_input_options(&options);

   /* if the file name is the same, reload between the new range */
   if(strcmp(filename, pane->file_name->str) == 0){
      g_snprintf(buf, 128, "loading %s range [%g:%g]", filename,
                 pane->pane_min_value, pane->pane_max_value);
      push_statusbar(NULL, buf);

      set_minc_input_user_real_range(&options,
                                     pane->pane_min_value, pane->pane_max_value);
      }

   /* start the volume input */
   if(start_volume_input(filename, pane->n_dims, axis_order_ptr,
                         NC_BYTE, FALSE, 0.0, 255.0,
                         TRUE, &(pane->volume), &options, &(pane->input_info)) != OK){
      return FALSE;
      }

   /* load a little bit of the file to ensure memory is allocated */
   continue_open_minc_file_to_pane(pane);

   /* get real range */
   get_volume_real_range(pane->volume, &pane->real_min, &pane->real_max);

   /* set up pane range */
   if(strcmp(filename, pane->file_name->str) == 0){
      pane->pane_min = pane->pane_min_value;
      pane->pane_max = pane->pane_max_value;
      }
   else {
      pane->pane_min_value = pane->pane_min = pane->real_min;
      pane->pane_max_value = pane->pane_max = pane->real_max;
      }

   get_volume_sizes(pane->volume, sizes);
   get_volume_separations(pane->volume, steps);
   get_volume_starts(pane->volume, starts);
   pane->transform = get_voxel_to_world_transform(pane->volume);

   /* fill the array up - do this manually to convert from zyxt to xyzt */
   /* x */
   pane->sizes[0] = sizes[2];
   pane->steps[0] = (double)steps[2];
   pane->starts[0] = (double)starts[2];
   pane->stops[0] = starts[2] + (steps[2] * ((double)sizes[2] - 1.0));

   /* y */
   pane->sizes[1] = sizes[1];
   pane->steps[1] = (double)steps[1];
   pane->starts[1] = (double)starts[1];
   pane->stops[1] = starts[1] + (steps[1] * ((double)sizes[1] - 1.0));

   /* z */
   pane->sizes[2] = sizes[0];
   pane->steps[2] = (double)steps[0];
   pane->starts[2] = (double)starts[0];
   pane->stops[2] = starts[0] + (steps[0] * ((double)sizes[0] - 1.0));

   /* t/vector */
   if(pane->n_dims > 3){
      pane->sizes[3] = sizes[3];
      pane->steps[3] = (double)steps[3];
      pane->starts[3] = (double)starts[3];
      pane->stops[3] = starts[3] + (steps[3] * ((double)sizes[3] - 1.0));
      }

   for(c = pane->n_dims; c < MAX_VNUP_DIMS; c++){
      pane->sizes[c] = 0;
      pane->steps[c] = 0;
      pane->starts[c] = 0;
      pane->stops[c] = 0;
      }

   return TRUE;
   }

/* input a bit more of a volume */
int continue_open_minc_file_to_pane(Pane_info pane)
{
   if(input_more_of_volume(pane->volume, &(pane->input_info), &(pane->perc_input))){
      return TRUE;
      }
   else {
      delete_volume_input(&(pane->input_info));
      return FALSE;
      }
   }

/* fills in the texmap pointer */
int get_minc_image(Pane_info pane, View_info view)
{
   Volume   vol;
   unsigned char *ptr;
   double   x_start, x_stop;
   double   x_incr, y_incr;
   double   y_offset;
   register int x_idx, y_idx, z_idx, t_idx;
   int      vec[4];

   /* get window to volume struct and pointer to data */
   vol = pane->volume;
   ptr = view->texmap;

   switch (view->type){
   default:
   case TRANSVERSE:
      x_idx = 0;
      y_idx = 1;
      z_idx = 2;
      t_idx = 3;
      break;
   case SAGITTAL:
      x_idx = 1;
      y_idx = 2;
      z_idx = 0;
      t_idx = 3;
      break;
   case CORONAL:
      x_idx = 0;
      y_idx = 2;
      z_idx = 1;
      t_idx = 3;
      break;
      }

   vec[t_idx] = (int)pane->v[t_idx];

   /* do real gnarly stuff for tilts, other wise... */
   if(view->tilt){
      x_incr = view->tilt_v[x_idx] / view->tilt_v[z_idx];
      y_incr = view->tilt_v[y_idx] / view->tilt_v[z_idx];

      for(vec[y_idx] = 0; vec[y_idx] < pane->sizes[y_idx]; vec[y_idx]++){
         y_offset = pane->v[z_idx] + ((pane->v[y_idx] - vec[y_idx]) * y_incr);

         if(x_incr < 0.0){
            x_start = 0.5 + pane->v[x_idx] + (y_offset / x_incr);
            x_stop = -0.5 + pane->v[x_idx] - ((pane->sizes[x_idx] - y_offset) / x_incr);
            }
         else {
            x_start = 0.5 + pane->v[x_idx] - ((pane->sizes[x_idx] - y_offset) / x_incr);
            x_stop = -0.5 + pane->v[x_idx] + (y_offset / x_incr);
            }

         /* clamp the start and stop */
         if(x_start < 0.0){
            x_start = 0.0;
            }
         if(x_stop > pane->sizes[x_idx]){
            x_stop = pane->sizes[x_idx];
            }

         /* check that we have to do anything at all */
         if((x_start > pane->sizes[x_idx]) || (x_stop < 0.0)){
            ptr = memset(ptr, 0, (int)pane->sizes[x_idx]);
            ptr += (int)pane->sizes[x_idx];
            }
         else {
            /* fill the blanks before */
            if(x_start > 0.0){
               ptr = memset(ptr, 0, (int)x_start);
               ptr += (int)x_start;
               }

            /* now get some real data */
            if(pane->n_dims == 3){
               for(vec[x_idx] = (int)x_start; vec[x_idx] < x_stop; vec[x_idx]++){

                  vec[z_idx] = y_offset + ((pane->v[x_idx] - vec[x_idx]) * x_incr);
                  GET_VOXEL_3D_TYPED(*ptr++, (unsigned char), vol,
                                     vec[2], vec[1], vec[0]);
                  }
               }
            else {
               for(vec[x_idx] = (int)x_start; vec[x_idx] < x_stop; vec[x_idx]++){

                  vec[z_idx] = y_offset + ((pane->v[x_idx] - vec[x_idx]) * x_incr);
                  GET_VOXEL_4D_TYPED(*ptr++, (unsigned char), vol,
                                     vec[2], vec[1], vec[0], vec[3]);
                  }
               }

            /* fill the blanks after */
            if(x_start < pane->sizes[x_idx]){
               ptr = memset(ptr, 0, (pane->sizes[x_idx] - x_stop));
               ptr += (int)(pane->sizes[x_idx] - x_stop);
               }
            }
         }
      }

   else {
      if(pane->v[z_idx] < 0 || pane->v[z_idx] >= pane->sizes[z_idx]){
         ptr = memset(ptr, 0, view->texmap_space);
         return TRUE;
         }
      else {
         vec[z_idx] = (int)pane->v[z_idx];

         for(vec[y_idx] = 0; vec[y_idx] < pane->sizes[y_idx]; vec[y_idx]++){
            if(pane->n_dims == 3){
               for(vec[x_idx] = 0; vec[x_idx] < pane->sizes[x_idx]; vec[x_idx]++){
                  GET_VOXEL_3D_TYPED(*ptr++, (unsigned char), vol,
                                     vec[2], vec[1], vec[0]);
                  }
               }
            else {
               for(vec[x_idx] = 0; vec[x_idx] < pane->sizes[x_idx]; vec[x_idx]++){
                  GET_VOXEL_4D_TYPED(*ptr++, (unsigned char), vol,
                                     vec[2], vec[1], vec[0], vec[3]);
                  }
               }
            }
         }
      }

   return TRUE;
   }

int get_voxel_value(Pane_info pane)
{

   /* check we aren't outside */
   if(pane->v[0] < 0 || pane->v[1] < 0 || pane->v[2] < 0 ||
      pane->v[0] >= pane->sizes[0] ||
      pane->v[1] >= pane->sizes[1] || pane->v[2] >= pane->sizes[2]){
      pane->voxel_value = 0.0;
      }

   else {
      switch (pane->value_type){
      default:
      case VALUE_REAL:
         if(pane->n_dims == 3){
            GET_VALUE_3D_TYPED(pane->voxel_value, (double), pane->volume,
                               (int)pane->v[2], (int)pane->v[1], (int)pane->v[0]);
            }
         else {
            GET_VALUE_4D_TYPED(pane->voxel_value, (double), pane->volume,
                               (int)pane->v[2],
                               (int)pane->v[1], (int)pane->v[0], (int)pane->v[3]);
            }
         break;
      case VALUE_VOXEL:
      case VALUE_RGBA:
         if(pane->n_dims == 3){
            GET_VOXEL_3D_TYPED(pane->voxel_value, (double), pane->volume,
                               (int)pane->v[2], (int)pane->v[1], (int)pane->v[0]);
            }
         else {
            GET_VOXEL_4D_TYPED(pane->voxel_value, (double), pane->volume,
                               (int)pane->v[2],
                               (int)pane->v[1], (int)pane->v[0], (int)pane->v[3]);
            }
         break;
         }
      }

   return TRUE;
   }

/* gets the 3 vector components at a point in a file */
void get_vector_components(Pane_info pane, int x, int y, int z,
                           double *dx, double *dy, double *dz)
{

   GET_VALUE_4D_TYPED(*dx, (double), pane->volume, z, y, x, 0);
   GET_VALUE_4D_TYPED(*dy, (double), pane->volume, z, y, x, 1);
   GET_VALUE_4D_TYPED(*dz, (double), pane->volume, z, y, x, 2);
   }
