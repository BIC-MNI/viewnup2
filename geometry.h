/* geometry.h - return values are in the first arguments            */

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "globals.h"

/* inits a vector                                                   */
void     vset(double *v, double x, double y, double z);

/* copy a vector                                                    */
void     vcopy(double *copy, double *v);

/* compute the addition of two vectors                              */
void     vadd(double *add, double *v1, double *v2);

/* compute the subtraction of two vectors (v1 - v2)                 */
void     vsub(double *sub, double *v1, double *v2);

/* multiply a vector by a constant                                  */
void     vscale(double *v, double scale);

/* returns the length of a vector                                   */
double   vlength(double *v);

/* returns the euc distance between two points                      */
double   veuc(double *p1, double *p2);

/* normalise a vector                                               */
void     vnormal(double *v);

/* computes the vector cross product of two vectors                 */
void     vcross(double *cross, double *v1, double *v2);

/* returns the vector dot product of two vectors                    */
double   vdot(double *v1, double *v2);

/* find the line that results from intersecting two planes          */
/* returns TRUE is found, FALSE if planes are parallel              */
int      intersect_planes(double *norml, double *pointl,
                          double *normp1, double *pointp1,
                          double *normp2, double *pointp2);

/* find the point that results from intersecting a plane            */
/* and a line returns TRUE is found, FALSE if colinear              */
/* line defined by point and normal                                 */
int      intersect_plane_line_n(double *point,
                                double *norml, double *pointl,
                                double *normp, double *pointp);

/* find the point that results from intersecting a plane            */
/* and a line returns TRUE is found, FALSE if colinear              */
/* line defined by two points                                       */
int      intersect_plane_line_p(double *point,
                                double *point1, double *point2,
                                double *normp, double *pointp);

/*  clip line v1 - v2 against a rectangle                          */
/* returns TRUE if inside, FALSE if outside                        */
int      cohen_sutherland_clip(double *x0, double *y0,
                               double *x1, double *y1,
                               double xmin, double xmax, double ymin, double ymax);

/* transform functions */
void     update_voxel_from_world(Pane_info pane);
void     update_world_from_voxel(Pane_info pane);
void     update_voxel_vector_from_world(Pane_info pane, View_info view);
void     update_world_vector_from_voxel(Pane_info pane, View_info view);

#endif
