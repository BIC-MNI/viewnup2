/* geometry.c */

#include <math.h>
#include "geometry.h"

/* inits a vector */
void vset(double *v, double x, double y, double z)
{
   v[0] = x;
   v[1] = y;
   v[2] = z;
   }

/* copy a vector */
void vcopy(double *copy, double *v)
{
   copy[0] = v[0];
   copy[1] = v[1];
   copy[2] = v[2];
   }

/* compute the addition of two vectors */
void vadd(double *add, double *v1, double *v2)
{
   add[0] = v1[0] + v2[0];
   add[1] = v1[1] + v2[1];
   add[2] = v1[2] + v2[2];
   }

/* compute the subtraction of two vectors */
void vsub(double *sub, double *v1, double *v2)
{
   sub[0] = v1[0] - v2[0];
   sub[1] = v1[1] - v2[1];
   sub[2] = v1[2] - v2[2];
   }

/* multiply a vector by a constant */
void vscale(double *v, double scale)
{
   v[0] *= scale;
   v[1] *= scale;
   v[2] *= scale;
   }

/*computes the vector cross product of two vectors */
void vcross(double *cross, double *v1, double *v2)
{
   cross[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
   cross[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
   cross[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
   }

/* returns the vector dot product of two vectors */
double vdot(double *v1, double *v2)
{
   return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
   }

/* returns the euc length of a vector */
double vlength(double *v)
{
   return sqrt(SQR2(v[0]) + SQR2(v[1]) + SQR2(v[2]));
   }

/* returns the euc distance between two points */
double veuc(double *p1, double *p2)
{
   return sqrt(SQR2(p2[0] - p1[0]) + SQR2(p2[1] - p1[1]) + SQR2(p2[2] - p1[2]));
   }

/* normalise a vector */
void vnormal(double *v)
{
   vscale(v, 1.0 / vlength(v));
   }

/* find the line that results from intersecting two planes   */
/* returns TRUE is found, FALSE if planes are parallel       */
/* http://astronomy.swin.edu.au/pbourke/geometry/planeplane/ */
int intersect_planes(double *norml, double *pointl,
                     double *normp1, double *pointp1, double *normp2, double *pointp2)
{
   double   tmp1[3], tmp2[3];
   double   det;
   double   c1, c2, d1, d2;

   /* check that we haven't been given parallel planes (coplanar also) */
   vcross(tmp1, normp1, normp2);
   if(tmp1[0] == 0 && tmp1[1] == 0 && tmp1[2] == 0){
      return FALSE;
      }

   det = (vdot(normp1, normp1) * vdot(normp2, normp2)) - SQR2(vdot(normp1, normp2));

   d1 = vdot(normp1, pointp1);
   d2 = vdot(normp2, pointp2);
   c1 = ((d1 * vdot(normp2, normp2)) - (d2 * vdot(normp1, normp2))) / det;
   c2 = ((d2 * vdot(normp1, normp1)) - (d1 * vdot(normp1, normp2))) / det;

   /* resulting line normal */
   vcross(norml, normp1, normp2);

   /* scale normp1 and normp2 */
   vcopy(tmp1, normp1);
   vcopy(tmp2, normp2);
   vscale(tmp1, c1);
   vscale(tmp2, c2);

   /* resulting line origin */
   vadd(pointl, tmp1, tmp2);

   return TRUE;
   }

/* find the point that results from intersecting a plane            */
/* and a line returns TRUE is found, FALSE if colinear              */
/* http://astronomy.swin.edu.au/pbourke/geometry/planeline/         */
int intersect_plane_line_n(double *point,
                           double *norml, double *pointl, double *normp, double *pointp)
{
   double   u, num, denom;
   double   tmp[3];
   double   point2[3];

   /* first generate the 2nd point on the line from the line normal */
   vadd(point2, pointl, norml);

   /* solve for u */
   vsub(tmp, pointp, pointl);
   num = vdot(normp, tmp);

   vsub(tmp, point2, pointl);
   denom = vdot(normp, tmp);

   /* check that we are not co-linear or parallel */
   if(denom == 0){
      return FALSE;
      }

   u = num / denom;

   vsub(tmp, point2, pointl);
   vscale(tmp, u);
   vadd(point, pointl, tmp);

   return TRUE;
   }

/* find the point that results from intersecting a plane            */
/* and a line returns TRUE is found, FALSE if colinear              */
/* http://astronomy.swin.edu.au/pbourke/geometry/planeline/         */
int intersect_plane_line_p(double *point,
                           double *point1, double *point2, double *normp, double *pointp)
{
   double   u, denom;
   double   tmp[3];

   /* solve for u - check denominator first (co-linear or parallel) */
   vsub(tmp, point2, point1);
   denom = vdot(normp, tmp);
   if(denom == 0){
      return FALSE;
      }

   vsub(tmp, pointp, point1);
   u = vdot(normp, tmp) / denom;

   vsub(tmp, point2, point1);
   vscale(tmp, u);
   vadd(point, point1, tmp);

   return TRUE;
   }

/* -------------------------------------------------------------------------- */
/* Cohoe Sutherland Clipping of a line in a square                            */
/* http://www.daimi.au.dk/~mbl/cgcourse/wiki/cohen-sutherland_line-cli.html   */
/* define the coding of end points */
enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };
int      ComputeCode(double x, double y, double xmin, double xmax, double ymin,
                     double ymax);

/* compute the code of a point relative to a rectangle */
int ComputeCode(double x, double y, double xmin, double xmax, double ymin, double ymax)
{
   int      c = 0;

   if(y > ymax)
      c |= TOP;
   else if(y < ymin)
      c |= BOTTOM;
   if(x > xmax)
      c |= RIGHT;
   else if(x < xmin)
      c |= LEFT;
   return c;
   }

/* clip line P0(x0,y0)-P1(x1,y1) against a rectangle */
/* returns TRUE if inside, FALSE if outside          */
int cohen_sutherland_clip(double *x0, double *y0,
                          double *x1, double *y1,
                          double xmin, double xmax, double ymin, double ymax)
{
   int      C0, C1, C;
   double   x, y;

   C0 = ComputeCode(*x0, *y0, xmin, xmax, ymin, ymax);
   C1 = ComputeCode(*x1, *y1, xmin, xmax, ymin, ymax);

   for(;;){
      /* trivial accept: both ends in rectangle */
      if((C0 | C1) == 0){
         //MidPointLineReal(x0, y0, x1, y1);  ??????
         return TRUE;
         }

      /* trivial reject: both ends on the external side of the rectangle */
      if((C0 & C1) != 0){
         return FALSE;
         }

      /* normal case: clip end outside rectangle */
      C = C0 ? C0 : C1;
      if(C & TOP){
         x = *x0 + (*x1 - *x0) * (ymax - *y0) / (*y1 - *y0);
         y = ymax;
         }
      else if(C & BOTTOM){
         x = *x0 + (*x1 - *x0) * (ymin - *y0) / (*y1 - *y0);
         y = ymin;
         }
      else if(C & RIGHT){
         x = xmax;
         y = *y0 + (*y1 - *y0) * (xmax - *x0) / (*x1 - *x0);
         }
      else {
         x = xmin;
         y = *y0 + (*y1 - *y0) * (xmin - *x0) / (*x1 - *x0);
         }

      /* set new end point and iterate */
      if(C == C0){
         *x0 = x;
         *y0 = y;
         C0 = ComputeCode(*x0, *y0, xmin, xmax, ymin, ymax);
         }
      else {
         *x1 = x;
         *y1 = y;
         C1 = ComputeCode(*x1, *y1, xmin, xmax, ymin, ymax);
         }
      }

   /* notreached */
   }

/* transform functions */
/* convert a pane voxel co-ordinate to a world co-ordinate */
void update_voxel_from_world(Pane_info pane)
{
   general_inverse_transform_point(pane->transform,
                                   pane->w[0], pane->w[1], pane->w[2],
                                   &pane->v[0], &pane->v[1], &pane->v[2]);
   pane->v[3] = pane->w[3];
   }

/* convert a pane world co-ordinate to a voxel co-ordinate */
void update_world_from_voxel(Pane_info pane)
{
   general_transform_point(pane->transform,
                           pane->v[0], pane->v[1], pane->v[2],
                           &pane->w[0], &pane->w[1], &pane->w[2]);
   pane->w[3] = pane->v[3];
   }

/* convert a view world vector to a voxel vector */
void update_voxel_vector_from_world(Pane_info pane, View_info view)
{
   Real     point[3], origin[3];
   register int c;

   general_inverse_transform_point(pane->transform,
                                   0.0, 0.0, 0.0, &origin[0], &origin[1], &origin[2]);
   general_inverse_transform_point(pane->transform,
                                   view->tilt_w[0],
                                   view->tilt_w[1],
                                   view->tilt_w[2], &point[0], &point[1], &point[2]);
   for(c = 0; c < 3; c++){
      view->tilt_v[c] = point[c] - origin[c];
      }
   }

/* convert a view voxel vector to a world vector */
void update_world_vector_from_voxel(Pane_info pane, View_info view)
{
   Real     point[3], origin[3];
   register int c;

   general_transform_point(pane->transform,
                           0.0, 0.0, 0.0, &origin[0], &origin[1], &origin[2]);
   general_transform_point(pane->transform,
                           view->tilt_v[0],
                           view->tilt_v[1],
                           view->tilt_v[2], &point[0], &point[1], &point[2]);
   for(c = 0; c < 3; c++){
      view->tilt_w[c] = point[c] - origin[c];
      }
   }
