/* lookup_table.c */

#include <ctype.h>
#include "lookup_table.h"
#include "viewnup.h"

/* Structure for sorting the lookup table */
typedef struct {
   double   key;
   int      index;
   } Sort_Key;

/* Internal Function prototypes */
double  *get_null_value(int vector_length, char *null_value_string);
char    *get_next_line(char *line, int linelen, FILE * fp);
int      sorting_function(const void *value1, const void *value2);
double  *get_values_from_string(char *string, int array_size, double *array, int *nread);
void     lookup_in_table(double index, Lookup_Table * lookup_table,
                         double output_value[]);

/* returns a new lookup_table */
Lookup_Table *new_lookup_table(void)
{
   Lookup_Table *table = (Lookup_Table *) g_malloc(sizeof(Lookup_Table));

   return table;
   }

GList   *get_cmaps_list(Main_info * ptr)
{
   GList   *list = g_list_alloc();
   int      c;

   for(c = 0; c < ptr->n_tables; c++){
      list = g_list_append(list, ptr->lookup_tables[c]->name);
      }

   return list;
   }

Lookup_Table *get_cmap_ptr(const char *cmap_name, Main_info * ptr)
{
   int      c;

   for(c = 0; c < ptr->n_tables; c++){
      if(strcmp(ptr->lookup_tables[c]->name, cmap_name) == 0){
         return ptr->lookup_tables[c];
         }
      }

   return NULL;
   }

void init_lookup_tables(Main_info * ptr)
{

   Lookup_Table *lut;

   static double grey[] = {
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 1.0, 1.0, 1.0, 1.0
      };

   static double spectral[] = {
      0.00, 0.0000, 0.0000, 0.0000, 0.0,
      0.05, 0.4667, 0.0000, 0.5333, 1.0,
      0.10, 0.5333, 0.0000, 0.6000, 1.0,
      0.15, 0.0000, 0.0000, 0.6667, 1.0,
      0.20, 0.0000, 0.0000, 0.8667, 1.0,
      0.25, 0.0000, 0.4667, 0.8667, 1.0,
      0.30, 0.0000, 0.6000, 0.8667, 1.0,
      0.35, 0.0000, 0.6667, 0.6667, 1.0,
      0.40, 0.0000, 0.6667, 0.5333, 1.0,
      0.45, 0.0000, 0.6000, 0.0000, 1.0,
      0.50, 0.0000, 0.7333, 0.0000, 1.0,
      0.55, 0.0000, 0.8667, 0.0000, 1.0,
      0.60, 0.0000, 1.0000, 0.0000, 1.0,
      0.65, 0.7333, 1.0000, 0.0000, 1.0,
      0.70, 0.9333, 0.9333, 0.0000, 1.0,
      0.75, 1.0000, 0.8000, 0.0000, 1.0,
      0.80, 1.0000, 0.6000, 0.0000, 1.0,
      0.85, 1.0000, 0.0000, 0.0000, 1.0,
      0.90, 0.8667, 0.0000, 0.0000, 1.0,
      0.95, 0.8000, 0.0000, 0.0000, 1.0,
      1.00, 0.8000, 0.8000, 0.8000, 1.0
      };

   static double hotmetal[] = {
      0.00, 0.0, 0.0, 0.0, 0.0,
      0.25, 0.5, 0.0, 0.0, 0.3,
      0.50, 1.0, 0.5, 0.0, 0.7,
      0.75, 1.0, 1.0, 0.5, 0.9,
      1.00, 1.0, 1.0, 1.0, 1.0
      };

   static double bluered[] = {
      0.000, 0.8, 0.8, 1.0, 1.00,
      0.125, 0.4, 0.8, 1.0, 0.95,
      0.250, 0.0, 0.4, 1.0, 0.85,
      0.375, 0.0, 0.0, 0.5, 0.60,
      0.500, 0.0, 0.0, 0.0, 0.10,
      0.625, 0.5, 0.0, 0.0, 0.60,
      0.750, 1.0, 0.4, 0.0, 0.85,
      0.825, 1.0, 0.8, 0.4, 0.95,
      1.000, 1.0, 0.8, 0.8, 1.00
      };

   static double red[] = {
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 1.0, 0.0, 0.0, 1.0
      };

   static double green[] = {
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 1.0, 0.0, 1.0
      };

   static double blue[] = {
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 0.0, 1.0, 1.0
      };

   static double cyan[] = {
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 0.0, 1.0, 1.0, 1.0
      };

   static double magenta[] = {
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 1.0, 0.0, 1.0, 1.0
      };

   static double yellow[] = {
      0.0, 0.0, 0.0, 0.0, 0.0,
      1.0, 1.0, 1.0, 0.0, 1.0
      };

   static double hotred[] = {
      0.00, 0.0, 0.0, 0.0, 0.0,
      0.25, 0.5, 0.0, 0.0, 0.3,
      0.50, 1.0, 0.0, 0.5, 0.7,
      0.75, 1.0, 0.5, 1.0, 0.9,
      1.00, 1.0, 1.0, 1.0, 1.0
      };

   static double hotgreen[] = {
      0.00, 0.0, 0.0, 0.0, 0.0,
      0.25, 0.0, 0.5, 0.0, 0.3,
      0.50, 0.0, 1.0, 0.5, 0.7,
      0.75, 0.5, 1.0, 1.0, 0.9,
      1.00, 1.0, 1.0, 1.0, 1.0
      };

   static double hotblue[] = {
      0.00, 0.0, 0.0, 0.0, 0.0,
      0.25, 0.0, 0.0, 0.5, 0.3,
      0.50, 0.0, 0.5, 1.0, 0.7,
      0.75, 0.5, 1.0, 1.0, 0.9,
      1.00, 1.0, 1.0, 1.0, 1.0
      };

   /* grey lookup table */
   ptr->lookup_tables[0] = new_lookup_table();
   strcpy(ptr->lookup_tables[0]->name, "grey");
   ptr->lookup_tables[0]->nentries = sizeof(grey) / sizeof(grey[0]) / 5;
   ptr->lookup_tables[0]->vector_length = 4;
   ptr->lookup_tables[0]->table = grey;

   /* spectral lookup table */
   ptr->lookup_tables[1] = new_lookup_table();
   strcpy(ptr->lookup_tables[1]->name, "spectral");
   ptr->lookup_tables[1]->nentries = sizeof(spectral) / sizeof(spectral[0]) / 5;
   ptr->lookup_tables[1]->vector_length = 4;
   ptr->lookup_tables[1]->table = spectral;

   /* hotmetal lookup table */
   lut = ptr->lookup_tables[2] = new_lookup_table();
   strcpy(lut->name, "hotmetal");
   lut->nentries = sizeof(hotmetal) / sizeof(hotmetal[0]) / 5;
   lut->vector_length = 4;
   lut->table = hotmetal;

   /* bluered lookup table */
   lut = ptr->lookup_tables[3] = new_lookup_table();
   strcpy(lut->name, "bluered");
   lut->nentries = sizeof(bluered) / sizeof(bluered[0]) / 5;
   lut->vector_length = 4;
   lut->table = bluered;

   /* red lookup table */
   lut = ptr->lookup_tables[4] = new_lookup_table();
   strcpy(lut->name, "red");
   lut->nentries = sizeof(red) / sizeof(red[0]) / 5;
   lut->vector_length = 4;
   lut->table = red;

   /* green lookup table */
   lut = ptr->lookup_tables[5] = new_lookup_table();
   strcpy(lut->name, "green");
   lut->nentries = sizeof(green) / sizeof(green[0]) / 5;
   lut->vector_length = 4;
   lut->table = green;

   /* blue lookup table */
   lut = ptr->lookup_tables[6] = new_lookup_table();
   strcpy(lut->name, "blue");
   lut->nentries = sizeof(blue) / sizeof(blue[0]) / 5;
   lut->vector_length = 4;
   lut->table = blue;

   /* cyan lookup table */
   lut = ptr->lookup_tables[7] = new_lookup_table();
   strcpy(lut->name, "cyan");
   lut->nentries = sizeof(cyan) / sizeof(cyan[0]) / 5;
   lut->vector_length = 4;
   lut->table = cyan;

   /* magenta lookup table */
   lut = ptr->lookup_tables[8] = new_lookup_table();
   strcpy(lut->name, "magenta");
   lut->nentries = sizeof(magenta) / sizeof(magenta[0]) / 5;
   lut->vector_length = 4;
   lut->table = magenta;

   /* yellow lookup table */
   lut = ptr->lookup_tables[9] = new_lookup_table();
   strcpy(lut->name, "yellow");
   lut->nentries = sizeof(yellow) / sizeof(yellow[0]) / 5;
   lut->vector_length = 4;
   lut->table = yellow;

   /* hotred lookup table */
   lut = ptr->lookup_tables[10] = new_lookup_table();
   strcpy(lut->name, "hotred");
   lut->nentries = sizeof(hotred) / sizeof(hotred[0]) / 5;
   lut->vector_length = 4;
   lut->table = hotred;

   /* hotgreen lookup table */
   lut = ptr->lookup_tables[11] = new_lookup_table();
   strcpy(lut->name, "hotgreen");
   lut->nentries = sizeof(hotgreen) / sizeof(hotgreen[0]) / 5;
   lut->vector_length = 4;
   lut->table = hotgreen;

   /* hotblue lookup table */
   lut = ptr->lookup_tables[12] = new_lookup_table();
   strcpy(lut->name, "hotblue");
   lut->nentries = sizeof(hotblue) / sizeof(hotblue[0]) / 5;
   lut->vector_length = 4;
   lut->table = hotblue;

   ptr->n_tables = 13;
   }

/* loads a table into the pane cmap stuctures */
int load_table_to_pane(Pane_info pane)
{

   int      c;
   double   value, scale;
   double   out[4];

   double   pane_range;
   double   lowwer_perc, upper_perc;

   if(pane->cmap_ptr == NULL){
      g_print("ACK! no cmap there! - pane: %s\n", pane->file_name->str);
      return FALSE;
      }

   pane_range = pane->pane_max - pane->pane_min;

   lowwer_perc = (pane->pane_min_value - pane->pane_min) / pane_range;
   upper_perc = (pane->pane_max_value - pane->pane_min) / pane_range;
   scale = upper_perc - lowwer_perc;

   for(c = 0; c < 256; c++){

      value = (((double)c / 255.0) - lowwer_perc) / scale;
      if(value < 0.0){
         value = 0.0;
         }
      else if(value > 1.0){
         value = 1.0;
         }

      /* Look it up */
      lookup_in_table(value, pane->cmap_ptr, out);
      pane->cmap_r[c] = out[0];
      pane->cmap_g[c] = out[1];
      pane->cmap_b[c] = out[2];

      if(pane->cmap_ptr->vector_length > 3){
         pane->cmap_a[c] = out[3];
         }
      else {
         pane->cmap_a[c] = 1.0;
         }
      }

   return TRUE;
   }

/* stores a LUT and returns it's index             */
/* usually called directly after read_lookup_table */
int store_table(Lookup_Table * lut, Main_info * ptr)
{

   if(ptr->n_tables > MAX_TABLES - 2){
      g_print("Too many tables, I canna handle no more captain!\n");
      return FALSE;
      }
   else {
      ptr->lookup_tables[ptr->n_tables] = lut;
      ptr->n_tables++;
      return TRUE;
      }
   }

/* ----------------------------- MNI Header -----------------------------------
@NAME       : read_lookup_table
@INPUT      : lookup_filename - name of file from which to read lookup table
@OUTPUT     : (nothing)
@RETURNS    : Pointer to lookup table
@DESCRIPTION: Routine to read in a lookup table from a file.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : December 8, 1994 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
Lookup_Table *read_lookup_table(const char *lookup_filename)
{
   Lookup_Table *lookup_table;
   FILE    *fp;
   double  *table, *row, *new_table;
   int      nentries, table_nvalues, nvalues, ivalue, ientry;
   char     line[1000];
   Sort_Key *sort_table;
   int      need_sort;
   int      old_offset, new_offset;

   /* Open the file */
   fp = fopen(lookup_filename, "r");
   if(fp == NULL){
      g_print("Unable to open lookup file \"%s\".\n", lookup_filename);
      return NULL;
      }

   /* Read in the table */
   /* Read the first line and get the vector length from that */
   nentries = 0;
   if(get_next_line(line, sizeof(line), fp) == NULL){
      if(fp != NULL)
         g_print("Lookup file %s is empty.\n", lookup_filename);
      else
         g_print("Lookup string is empty.\n");
      return NULL;
      }
   row = get_values_from_string(line, 0, NULL, &table_nvalues);
   if(table_nvalues < 2){
      g_print("First line has fewer than 2 values.\n");
      if(row != NULL)
         g_free(row);
      return NULL;
      }
   table = (void *)g_malloc(sizeof(*table) * table_nvalues);
   for(ivalue = 0; ivalue < table_nvalues; ivalue++)
      table[ivalue] = row[ivalue];
   nentries++;
   need_sort = FALSE;
   while(get_next_line(line, sizeof(line), fp) != NULL){
      (void)get_values_from_string(line, table_nvalues, row, &nvalues);
      if(nvalues != table_nvalues){
         g_print("Wrong number of values on line %d.\n", nentries + 1);
         g_free(row);
         g_free(table);
         return NULL;
         }
      table = (void *)realloc(table, sizeof(*table) * table_nvalues * (nentries + 1));
      for(ivalue = 0; ivalue < table_nvalues; ivalue++){
         table[ivalue + nentries * table_nvalues] = row[ivalue];
         }
      nentries++;

      /* Check for need to sort */
      if(table[(nentries - 2) * table_nvalues] > table[(nentries - 1) * table_nvalues]){
         need_sort = TRUE;
         }
      }

   /* Close the input file */
   if(fp != NULL){
      (void)fclose(fp);
      }

   /* Do sorting if needed */
   if(need_sort){

      /* Set up sorting table */
      sort_table = (void *)g_malloc(sizeof(*sort_table) * nentries);
      for(ientry = 0; ientry < nentries; ientry++){
         sort_table[ientry].key = table[ientry * table_nvalues];
         sort_table[ientry].index = ientry;
         }

      /* Sort the sorting table */
      qsort((void *)sort_table, nentries, sizeof(*sort_table), sorting_function);

      /* Copy the table */
      new_table = (void *)g_malloc(sizeof(*table) * table_nvalues * nentries);
      for(ientry = 0; ientry < nentries; ientry++){
         new_offset = ientry * table_nvalues;
         old_offset = sort_table[ientry].index * table_nvalues;
         for(ivalue = 0; ivalue < table_nvalues; ivalue++){
            new_table[new_offset + ivalue] = table[old_offset + ivalue];
            }
         }
      g_free(table);
      table = new_table;
      g_free(sort_table);
      }

   /* Allocate space for the lookup table and set initial values */
   lookup_table = (void *)g_malloc(sizeof(*lookup_table));
   lookup_table->nentries = nentries;
   lookup_table->vector_length = table_nvalues - 1;
   lookup_table->table = table;

   /* Return the lookup table */
   return lookup_table;
   }

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_next_line
@INPUT      : linelen - length of line to read in
              fp - file pointer or NULL if data should be read from a string
              string - pointer string containing data
@OUTPUT     : line - line that has been read in
              string - pointer is advanced to character following ";"
@RETURNS    : pointer to line or NULL if end of data occurs
@DESCRIPTION: Routine to get the next line from a file or from a string
              that uses ";" as a line separator.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : July 10, 1996 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
#define LOOKUP_LINE_SEPARATOR ';'
char    *get_next_line(char *line, int linelen, FILE * fp)
{

   /* Read from the file if appropriate */
   if(fp != NULL){
      return fgets(line, linelen, fp);
      }
   return (NULL);
   }

/* ----------------------------- MNI Header -----------------------------------
@NAME       : sorting_function
@INPUT      : value1 - pointer to first value
              value2 - pointer to second value
@OUTPUT     : (nothing)
@RETURNS    : -1, 0 or 1 for value1 <, ==, > value2
@DESCRIPTION: Routine to compare two values for sorting. The value is a
              pointer to a structure that has a double precision primary
              key. If that is equal then an integer secondary key is used.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : December 8, 1994 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
int sorting_function(const void *value1, const void *value2)
{
   Sort_Key *key1, *key2;

   key1 = (Sort_Key *) value1;
   key2 = (Sort_Key *) value2;

   if(key1->key == key2->key){
      if(key1->index == key2->index)
         return 0;
      else if(key1->index < key2->index)
         return -1;
      else
         return 1;
      }
   else if(key1->key < key2->key)
      return -1;
   else
      return 1;

   }

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_null_value
@INPUT      : vector_length - desired vector length
              string - string from which we should get null values
@OUTPUT     : (nothing)
@RETURNS    : Pointer to array of values
@DESCRIPTION: Routine to convert a string to an array of values 
              to be used as a null value for the lookup table. Returns 
              NULL if string is NULL
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : December 8, 1994 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
double  *get_null_value(int vector_length, char *string)
{
   int      num_read;
   double  *values;

   /* Check for NULL string */
   if(string == NULL)
      return NULL;

   /* Read values */
   values = get_values_from_string(string, 0, NULL, &num_read);

   /* Check the number of values read */
   if(num_read != vector_length){
      if(values != NULL)
         g_free(values);
      g_print("Null value does not match lookup table (%d values).\n", num_read);
      exit(EXIT_FAILURE);
      }

   return values;
   }

/* ----------------------------- MNI Header -----------------------------------
@NAME       : lookup_in_table
@INPUT      : index - value to look up in table
              lookup_table - the lookup table (big surprise!)
@OUTPUT     : output_value - vector of output values.
@RETURNS    : (nothing)
@DESCRIPTION: Routine to look up a value in the table.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : December 8, 1994 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
void lookup_in_table(double index, Lookup_Table * lookup_table, double output_value[])
{

   int      vector_length, nentries;
   int      start, length, mid;
   int      offset, offset1, offset2, ivalue;
   double   value1, value2, frac, rfrac, denom;

   /* Check for bad lookup table */
   nentries = lookup_table->nentries;
   vector_length = lookup_table->vector_length;
   if((nentries < 1) || (vector_length < 1)){
      g_print("Bad table size %d x %d\n", nentries, vector_length);
      exit(EXIT_FAILURE);
      }

   /* Search the table for the value */
   start = 0;
   length = nentries;
   while(length > 1){
      mid = start + length / 2;
      offset = mid * (vector_length + 1);
      if(index < lookup_table->table[offset]){
         length = mid - start;
         }
      else {
         length = start + length - mid;
         start = mid;
         }
      }

   /* Add a special check for the end of the table */
   if(nentries > 1){
      offset1 = vector_length + 1;
      offset2 = (nentries - 2) * (vector_length + 1);
      if((start == 0) && (index == lookup_table->table[offset1]))
         start = 1;
      else if((start == nentries - 1) && (index == lookup_table->table[offset2]))
         start = nentries - 2;
      }

   /* Save the value */
   offset = start * (vector_length + 1);

   offset1 = offset;
   if(start < nentries - 1)
      offset2 = offset + vector_length + 1;
   else
      offset2 = offset;
   value1 = lookup_table->table[offset1];
   value2 = lookup_table->table[offset2];
   denom = value2 - value1;
   if(denom != 0.0)
      frac = (index - value1) / denom;
   else
      frac = 0.0;
   if(frac < 0.0)
      frac = 0.0;
   if(frac > 1.0)
      frac = 1.0;
   rfrac = 1.0 - frac;
   for(ivalue = 0; ivalue < vector_length; ivalue++){
      output_value[ivalue] =
         rfrac * lookup_table->table[offset1 + 1 + ivalue] +
         frac * lookup_table->table[offset2 + 1 + ivalue];
      }
   }

/* ----------------------------- MNI Header -----------------------------------
@NAME       : get_values_from_string
@INPUT      : string - string containing values
              array_size - maximum number of values (<=0 means allocate a new
                 array )
@OUTPUT     : array - array into which values are written (can be NULL if
                 array_size <= 0)
              nread - number of values read
@RETURNS    : Pointer to array of values
@DESCRIPTION: Routine to convert a string to an array of values 
              (floating point). If an existing array is passed in
              (array_size > 0), then up to array_size values are copied into 
              it. Otherwise, a new array is created. Normally, the number of
              values read is returned - if an error occurs (non-numeric string
              for example), then zero is returned.
@METHOD     : 
@GLOBALS    : 
@CALLS      : 
@CREATED    : December 8, 1994 (Peter Neelin)
@MODIFIED   : 
---------------------------------------------------------------------------- */
double  *get_values_from_string(char *string, int array_size, double *array, int *nread)
{
#define VECTOR_SEPARATOR ','

   char    *cur, *prev, *end;
   int      num_read, num_alloc;
   double   dvalue;

   /* Set up variables */
   num_read = 0;
   if(array_size <= 0){
      num_alloc = 0;
      array = NULL;
      }
   else {
      num_alloc = array_size;
      }

   /* Skip leading white space */
   end = string + strlen(string);
   cur = string;
   while(isspace((int)(*cur)))
      cur++;

   /* Loop through string looking for doubles */
   while(cur != end){

      /* Get double */
      prev = cur;
      dvalue = strtod(prev, &cur);
      if(cur == prev){
         *nread = 0;
         if(array_size <= 0 && array != NULL){
            g_free(array);
            }
         return NULL;
         }

      /* Add the value to the list */
      num_read++;
      if(num_read > num_alloc){
         if(array_size <= 0){
            num_alloc += 1;
            if(array == NULL){
               array = (void *)g_malloc(num_alloc * sizeof(*array));
               }
            else {
               array = (void *)realloc(array, num_alloc * sizeof(*array));
               }
            }
         else {
            *nread = num_read;
            return array;
            }
         }
      array[num_read - 1] = dvalue;

      /* Skip any spaces */
      while(isspace((int)(*cur)))
         cur++;

      /* Skip an optional comma */
      if(*cur == VECTOR_SEPARATOR)
         cur++;

      }

   *nread = num_read;
   return array;
   }
